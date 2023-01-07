#include <stdafx.h>
#include <Extender/Client/ExtensionStateClient.h>
#include <GameDefinitions/Symbols.h>
#include <Extender/ScriptExtender.h>

#include <Extender/Client/CharacterTaskBinder.inl>

BEGIN_NS(ecl)

ExtensionState & ExtensionState::Get()
{
	return gExtender->GetClient().GetExtensionState();
}

ExtensionState::ExtensionState()
	: ExtensionStateBase(false)
{}

ExtensionState::~ExtensionState()
{
	if (Lua) Lua->Shutdown();
}

lua::State * ExtensionState::GetLua()
{
	if (Lua) {
		return Lua.get();
	} else {
		return nullptr;
	}
}

ModManager * ExtensionState::GetModManager()
{
	return GetModManagerClient();
}

void ExtensionState::DoLuaReset()
{
	if (Lua) Lua->Shutdown();
	Lua.reset();

	context_ = nextContext_;
	assert(context_ != ExtensionStateContext::Uninitialized);
	Lua = std::make_unique<lua::ClientState>(nextGenerationId_++);
	Lua->Initialize();
}

void ExtensionState::LuaStartup()
{
	ExtensionStateBase::LuaStartup();

	LuaClientPin lua(*this);
	auto gameState = GetStaticSymbols().GetClientState();
	if (gameState
		&& (*gameState == GameState::LoadLevel
			|| (*gameState == GameState::LoadModule && WasStatLoadTriggered())
			|| *gameState == GameState::LoadSession
			|| *gameState == GameState::LoadGMCampaign
			|| *gameState == GameState::Paused
			|| *gameState == GameState::PrepareRunning
			|| *gameState == GameState::Running
			|| *gameState == GameState::GameMasterPause)) {
		lua->OnModuleResume();
	}
}

END_NS()
