#pragma once

#include <Extender/Shared/ScriptExtenderBase.h>
#include <Extender/Shared/ExtenderConfig.h>
#include <Extender/Client/ExtensionStateClient.h>
#include <Extender/Client/NetworkManagerClient.h>
#include <GameDefinitions/Symbols.h>

BEGIN_NS(ecl)

class ScriptExtender : public ThreadedExtenderState
{
public:
	ScriptExtender(ExtenderConfig& config);

	void Initialize();
	void PostStartup();
	void Shutdown();

	inline bool HasExtensionState() const
	{
		return (bool)extensionState_;
	}

	inline ExtensionState& GetExtensionState() const
	{
		return *extensionState_;
	}

	inline NetworkManager& GetNetworkManager()
	{
		return network_;
	}

	inline NetworkFixedStringReceiver& GetNetworkFixedStrings()
	{
		return networkFixedStrings_;
	}

	/*inline ClientEntitySystemHelpers& GetEntityHelpers()
	{
		return entityHelpers_;
	}*/

	bool IsInClientThread() const;
	void ResetLuaState();
	void ResetExtensionState();
	void LoadExtensionState();

	// HACK - we need to expose this so it can be added to the CrashReporter whitelist
	enum class GameStateWorkerStartTag {};
	enum class GameStateMachineUpdateTag {};
	HookableFunction<GameStateWorkerStartTag, void(void*)> gameStateWorkerStart_;
	HookableFunction<GameStateMachineUpdateTag, void(void*, GameTime*)> gameStateMachineUpdate_;

private:
	ExtenderConfig& config_;
	std::unique_ptr<ExtensionState> extensionState_;
	bool extensionLoaded_{ false };
	NetworkManager network_;
	NetworkFixedStringReceiver networkFixedStrings_;
	//ClientEntitySystemHelpers entityHelpers_;

	enum class GameStateChangedEventTag {};
	PostHookableFunction<GameStateChangedEventTag, void(void*, GameState, GameState)> gameStateChangedEvent_;

	void OnBaseModuleLoaded(void* self);
	void OnGameStateChanged(void* self, GameState fromState, GameState toState);
	void OnGameStateWorkerStart(void* self);
	void OnGameStateWorkerExit(void* self);
	void OnUpdate(void* self, GameTime* time);

	void RegisterFlashTraceCallbacks();

	static void FlashTraceCallback(void* ctx, void* player, char const* message);
	static void FlashWarningCallback(void* ctx, void* player, int code, char const* message);
};

END_NS()