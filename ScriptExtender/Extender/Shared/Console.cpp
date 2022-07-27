#include <stdafx.h>
#include <Version.h>
#include <Extender/Shared/Console.h>
#include <Extender/ScriptExtender.h>

BEGIN_SE()

DebugConsole gConsole;

void DebugConsole::SetColor(DebugMessageType type)
{
	WORD wAttributes = 0;
	switch (type) {
	case DebugMessageType::Error:
		wAttributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
		break;

	case DebugMessageType::Warning:
		wAttributes = FOREGROUND_RED | FOREGROUND_GREEN;
		break;

	case DebugMessageType::Osiris:
		wAttributes = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE;
		break;

	case DebugMessageType::Info:
		wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		break;

	case DebugMessageType::Debug:
	default:
		wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		break;
	}

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, wAttributes);
}

void DebugConsole::Debug(DebugMessageType type, char const* msg)
{
	if (consoleRunning_ && !silence_) {
		SetColor(type);
		OutputDebugStringA(msg);
		OutputDebugStringA("\r\n");
		std::cout << msg << std::endl;
		std::cout.flush();
		SetColor(DebugMessageType::Debug);
	}

	if (logToFile_) {
		logFile_.write(msg, strlen(msg));
		logFile_.write("\r\n", 2);
		logFile_.flush();
	}

#if !defined(OSI_NO_DEBUGGER)
	if (gExtender) {
		auto debugger = gExtender->GetLuaDebugger();
		if (debugger && debugger->IsDebuggerReady()) {
			debugger->OnLogMessage(type, msg);
		}
	}
#endif
}

void DebugConsole::Debug(DebugMessageType type, wchar_t const* msg)
{
	if (consoleRunning_ && !silence_) {
		SetColor(type);
		OutputDebugStringW(msg);
		OutputDebugStringW(L"\r\n");
		std::wcout << msg << std::endl;
		std::wcout.flush();
		SetColor(DebugMessageType::Debug);
	}

	if (logToFile_) {
		auto utf = ToUTF8(msg);
		logFile_.write(utf.c_str(), utf.size());
		logFile_.write("\r\n", 2);
		logFile_.flush();
	}

#if !defined(OSI_NO_DEBUGGER)
	if (gExtender) {
		auto debugger = gExtender->GetLuaDebugger();
		if (debugger && debugger->IsDebuggerReady()) {
			debugger->OnLogMessage(type, ToUTF8(msg));
		}
	}
#endif
}

void DebugConsole::SubmitTaskAndWait(bool server, std::function<void()> task)
{
	if (server) {
		auto state = GetStaticSymbols().GetServerState();
		if (!state) {
			ERR("Cannot queue server commands when the server state machine is not initialized");
		} else if (*state == esv::GameState::Paused || *state == esv::GameState::Running) {
			gExtender->GetServer().SubmitTaskAndWait(task);
		} else {
			ERR("Cannot queue server commands in game state %s", EnumInfo<esv::GameState>::Find(*state).GetString());
		}
	} else {
		auto state = GetStaticSymbols().GetClientState();
		if (!state) {
			ERR("Cannot queue client commands when the client state machine is not initialized");
		} else if (*state == ecl::GameState::Menu 
			|| *state == ecl::GameState::Lobby 
			|| *state == ecl::GameState::Paused 
			|| *state == ecl::GameState::Running) {
			gExtender->GetClient().SubmitTaskAndWait(task);
		} else {
			ERR("Cannot queue client commands in game state %s", EnumInfo<ecl::GameState>::Find(*state).GetString());
		}
	}
}

void DebugConsole::ConsoleThread()
{
	std::string line;
	bool silence = true;
	while (consoleRunning_) {
		wchar_t tempBuf;
		DWORD tempRead;
		if (!ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), &tempBuf, 1, &tempRead, NULL)
			|| !tempRead
			|| tempBuf != '\r') {
			continue;
		}

		DEBUG("Entering server Lua console.");

		bool serverContext = true;

		while (consoleRunning_) {
			inputEnabled_ = true;
			silence_ = silence;

			if (serverContext) {
				std::cout << "S >> ";
			} else {
				std::cout << "C >> ";
			}

			std::cout.flush();
			std::getline(std::cin, line);
			inputEnabled_ = false;
			silence_ = false;
			if (line == "exit") break;

			if (line.empty()) {
			} else if (line == "server") {
				DEBUG("Switching to server context.");
				serverContext = true;
			} else if (line == "client") {
				DEBUG("Switching to client context.");
				serverContext = false;
			} else if (line == "reset") {
				DEBUG("Resetting Lua states.");
				SubmitTaskAndWait(true, []() {
					gExtender->GetServer().ResetLuaState();
				});

				SubmitTaskAndWait(false, []() {
					if (!gExtender->GetServer().RequestResetClientLuaState()) {
						gExtender->GetClient().ResetLuaState();
					}
				});
			} else if (line == "reset server") {
				DEBUG("Resetting server Lua state.");
				SubmitTaskAndWait(true, []() {
					gExtender->GetServer().ResetLuaState();
				});
			} else if (line == "reset client") {
				DEBUG("Resetting client Lua state.");
				SubmitTaskAndWait(false, []() {
					if (!gExtender->GetServer().RequestResetClientLuaState()) {
						gExtender->GetClient().ResetLuaState();
					}
				});
			} else if (line == "silence on") {
				DEBUG("Silent mode ON");
				silence = true;
			} else if (line == "silence off") {
				DEBUG("Silent mode OFF");
				silence = false;
			} else if (line == "help") {
				DEBUG("Anything typed in will be executed as Lua code except the following special commands:");
				DEBUG("  server - Switch to server context");
				DEBUG("  client - Switch to client context");
				DEBUG("  reset client - Reset client Lua state");
				DEBUG("  reset server - Reset server Lua state");
				DEBUG("  reset - Reset client and server Lua states");
				DEBUG("  silence <on|off> - Enable/disable silent mode (log output when in input mode)");
				DEBUG("  exit - Leave console mode");
				DEBUG("  !<cmd> <arg1> ... <argN> - Trigger Lua \"ConsoleCommand\" event with arguments cmd, arg1, ..., argN");
			} else {
				auto task = [line]() {
					auto state = gExtender->GetCurrentExtensionState();

					if (!state) {
						ERR("Extensions not initialized!");
						return;
					}

					LuaVirtualPin pin(*state);
					if (!pin) {
						ERR("Lua state not initialized!");
						return;
					}

					if (line[0] == '!') {
						lua::ConsoleEventParams params;
						params.Command = line.substr(1);
						ThrowEvent(*pin, "DoConsoleCommand", params, false, 0);
					} else {
						lua::StaticLifetimeStackPin _(pin->GetStack(), pin->GetGlobalLifetime());
						auto L = pin->GetState();
						if (luaL_loadstring(L, line.c_str()) || lua::CallWithTraceback(L, 0, 0)) { // stack: errmsg
							ERR("%s", lua_tostring(L, -1));
							lua_pop(L, 1);
						}
					}
				};

				SubmitTaskAndWait(serverContext, task);
			}
		}

		DEBUG("Exiting console mode.");
	}
}

void DebugConsole::Create()
{
	AllocConsole();
	SetConsoleTitleW(L"D:OS2 Script Extender Debug Console");

	if (IsValidCodePage(CP_UTF8)) {
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);
	}

	auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	// Disable Ctrl+C handling
	SetConsoleCtrlHandler(NULL, TRUE);

	FILE * outputStream;
	freopen_s(&outputStream, "CONOUT$", "w", stdout);
	FILE* inputStream;
	freopen_s(&inputStream, "CONIN$", "r", stdin);
	consoleRunning_ = true;

	DEBUG("******************************************************************************");
	DEBUG("*                                                                            *");
	DEBUG("*                   D:OS2 Script Extender Debug Console                      *");
	DEBUG("*                                                                            *");
	DEBUG("******************************************************************************");
	DEBUG("");
	DEBUG("DOS2Ext v%d built on " __DATE__ " " __TIME__, CurrentVersion);

	consoleThread_ = new std::thread(&DebugConsole::ConsoleThread, this);
	created_ = true;
}

void DebugConsole::OpenLogFile(std::wstring const& path)
{
	if (logToFile_) {
		CloseLogFile();
	}

	logFile_.rdbuf()->pubsetbuf(0, 0);
	logFile_.open(path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
	if (!logFile_.good()) {
		ERR(L"Failed to open log file '%s'", path.c_str());
	} else {
		logToFile_ = true;
	}
}

void DebugConsole::CloseLogFile()
{
	if (!logToFile_) return;

	logFile_.close();
	logToFile_ = false;
}


END_SE()
