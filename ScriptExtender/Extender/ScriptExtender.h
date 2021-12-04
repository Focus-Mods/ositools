#pragma once

#include <GameDefinitions/Osiris.h>
#include <Extender/Shared/StatLoadOrderHelper.h>
#include <Extender/Shared/SavegameSerializer.h>
#include <Extender/Shared/ExtenderConfig.h>
#include <Extender/Shared/ModuleHasher.h>
#include <Osiris/OsirisExtender.h>

#include <Extender/Client/ExtensionStateClient.h>
#include <Extender/Server/ExtensionStateServer.h>
#include <Extender/Client/ScriptExtenderClient.h>
#include <Extender/Server/ScriptExtenderServer.h>
#if !defined(OSI_NO_DEBUGGER)
#include <Osiris/Debugger/DebugInterface.h>
#include <Osiris/Debugger/DebugMessages.h>
#include <Osiris/Debugger/Debugger.h>
#include <Lua/Debugger/LuaDebugger.h>
#include <Lua/Debugger/LuaDebugMessages.h>
#endif
#include <Lua/Shared/LuaBundle.h>
#include <GameHooks/OsirisWrappers.h>
#include <Osiris/Shared/CustomFunctions.h>
#include <GameHooks/DataLibraries.h>
#include <GameHooks/EngineHooks.h>
#include <Osiris/Functions/FunctionLibrary.h>
#include "NetProtocol.h"
#include <GameDefinitions/Symbols.h>
#include <GameDefinitions/GlobalFixedStrings.h>
#include <Hit.h>
#include <StatusHelpers.h>

#include <thread>
#include <mutex>
#include <shared_mutex>

BEGIN_SE()

class ScriptExtender
{
public:
	ScriptExtender();

	void Initialize();
	void PostStartup();
	void Shutdown();

	inline ExtenderConfig& GetConfig()
	{
		return config_;
	}

	void LogLuaError(std::string_view msg);
	void LogOsirisError(std::string_view msg);
	void LogOsirisWarning(std::string_view msg);
	void LogOsirisMsg(std::string_view msg);

	inline esv::ScriptExtender& GetServer()
	{
		return server_;
	}

	inline ecl::ScriptExtender& GetClient()
	{
		return client_;
	}

	inline LibraryManager const & GetLibraryManager() const
	{
		return Libraries;
	}

	inline LibraryManager & GetLibraryManager()
	{
		return Libraries;
	}

#if !defined(OSI_NO_DEBUGGER)
	inline lua::dbg::Debugger* GetLuaDebugger()
	{
		return luaDebugger_.get();
	}

	inline lua::dbg::DebugMessageHandler* GetLuaDebugMessageHandler()
	{
		return luaDebugMsgHandler_.get();
	}
#endif

	ExtensionStateBase* GetCurrentExtensionState();

	bool HasFeatureFlag(char const *) const;

	inline StatLoadOrderHelper& GetStatLoadOrderHelper()
	{
		return statLoadOrderHelper_;
	}

	inline esv::StatusHelpers& GetStatusHelpers()
	{
		return statusHelpers_;
	}

	inline ModuleHasher& Hasher()
	{
		return hasher_;
	}

	inline EngineHooks& GetEngineHooks()
	{
		return hooks_;
	}

	inline lua::LuaBundle& GetLuaBuiltinBundle()
	{
		return luaBuiltinBundle_;
	}

	void ClearPathOverrides();
	void AddPathOverride(STDString const & path, STDString const & overriddenPath);
	std::optional<STDString> GetPathOverride(STDString const& path);

	std::wstring MakeLogFilePath(std::wstring const& Type, std::wstring const& Extension);
	void InitRuntimeLogging();

private:
	esv::ScriptExtender server_;
	ecl::ScriptExtender client_;
	LibraryManager Libraries;
	EngineHooks hooks_;
	std::recursive_mutex globalStateLock_;
	std::shared_mutex pathOverrideMutex_;
	std::unordered_map<STDString, STDString> pathOverrides_;
	SavegameSerializer savegameSerializer_;
	StatLoadOrderHelper statLoadOrderHelper_;
	esv::HitProxy hitProxy_;
	esv::StatusHelpers statusHelpers_;
	ModuleHasher hasher_;
	lua::LuaBundle luaBuiltinBundle_;

	ExtenderConfig config_;
	bool extensionsEnabled_{ false };
	bool postStartupDone_{ false };

#if !defined(OSI_NO_DEBUGGER)
	std::thread* luaDebuggerThread_{ nullptr };
	std::unique_ptr<LuaDebugInterface> luaDebugInterface_;
	std::unique_ptr<lua::dbg::DebugMessageHandler> luaDebugMsgHandler_;
	std::unique_ptr<lua::dbg::Debugger> luaDebugger_;
#endif

	void OnBaseModuleLoaded(void * self);
	void OnModuleLoadStarted(TranslatedStringRepository * self);
	void OnStatsLoadStarted(stats::RPGStats* mgr);
	void OnStatsLoadFinished(stats::RPGStats* mgr);
	void OnSkillPrototypeManagerInit(void * self);
	FileReader * OnFileReaderCreate(FileReader::CtorProc* next, FileReader * self, Path const& path, unsigned int type);
	void OnSavegameVisit(void* osirisHelpers, ObjectVisitor* visitor);

	void OnInitNetworkFixedStrings(eoc::NetworkFixedStrings* self, void * arg1);
};

extern std::unique_ptr<ScriptExtender> gExtender;

END_SE()
