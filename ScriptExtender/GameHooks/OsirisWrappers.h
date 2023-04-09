#pragma once

#include <GameDefinitions/Base/Base.h>
#include <GameDefinitions/Osiris.h>
#include <GameDefinitions/Enumerations.h>
#include <GameHooks/Wrappers.h>
#include <detours.h>
#include <thread>

namespace dse {

uint8_t * ResolveRealFunctionAddress(uint8_t * Address);

#define WRAPPABLE(ty, name) enum class name##Tag {}; \
	WrappableFunction<name##Tag, ty> name

class OsirisWrappers
{
public:
	OsirisWrappers();

	void Initialize();
	void InitializeExtensions();
	void InitializeDeferredExtensions();
	void InitializeNetworking(net::MessageFactory * factory);
	void Shutdown();
	bool ResolveNodeVMTs();
	void RegisterDIVFunctionsPreHook(void* Osiris, DivFunctions* Functions);

	OsirisStaticGlobals Globals;

	void * OsirisDllStart{ nullptr };
	uint32_t OsirisDllSize{ 0 };

	WRAPPABLE(int(void *, DivFunctions *), RegisterDivFunctions);
	WRAPPABLE(int(void *), InitGame);
	WRAPPABLE(int(void *, bool), DeleteAllData);
	WRAPPABLE(void(void *, MappingInfo **, uint32_t *), GetFunctionMappings);

	WRAPPABLE(void(void * Osiris, wchar_t const * Path, wchar_t const * Mode), OpenLogFile);
	WRAPPABLE(void(void * Osiris), CloseLogFile);

	WRAPPABLE(bool(void *, wchar_t const *, wchar_t const *), Compile);
	WRAPPABLE(int(void *, void *), Load);
	WRAPPABLE(bool(void *, wchar_t *), Merge);
	WRAPPABLE(ReturnCode(void *, uint32_t, OsiArgumentDesc *), Event);

	RuleActionCallProc OriginalRuleActionCallProc;
	WRAPPABLE(void(RuleActionNode *, void *, void *, void *, void *), RuleActionCall);

	WRAPPABLE(bool(uint32_t FunctionHandle, OsiArgumentDesc * Params), Call);
	WRAPPABLE(bool(uint32_t FunctionHandle, OsiArgumentDesc * Params), Query);
	WRAPPABLE(void(char const * Message), Error);
	WRAPPABLE(void(bool Successful, char const * Message, bool Unknown2), Assert);

	WRAPPABLE(HANDLE(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE), CreateFileW);
	WRAPPABLE(BOOL(HANDLE), CloseHandle);

	// FIXME - move all to EngineHooks!
	WRAPPABLE(void(eoc::NetworkFixedStrings*, void *), InitNetworkFixedStrings);
	WRAPPABLE(void(void *), SkillPrototypeManagerInit);
	WRAPPABLE(void(net::Message*, net::BitstreamSerializer*), eocnet__ClientConnectMessage__Serialize);
	WRAPPABLE(void(net::Message*, net::BitstreamSerializer*), eocnet__ClientAcceptMessage__Serialize);
	WRAPPABLE(bool(void*, ObjectVisitor*), esv__OsirisVariableHelper__SavegameVisit);
	WRAPPABLE(void(TranslatedStringRepository*), TranslatedStringRepository__UnloadOverrides);

	DivFunctions::CallProc CallOriginal;
	DivFunctions::CallProc QueryOriginal;
	DivFunctions::ErrorMessageProc ErrorOriginal;
	DivFunctions::AssertProc AssertOriginal;

	NodeVMT* VMTs[(unsigned)NodeType::Max + 1];

private:
	HMODULE Kernel32Module{ NULL };
	HMODULE OsirisModule{ NULL };
	bool ExtensionsInitialized{ false };
	bool DeferredExtensionsInitialized{ false };
	bool NetworkingInitialized{ false };

	std::optional<bool> resolvedVMTs_;

	void * FindRuleActionCallProc();
	void FindOsirisGlobals(FARPROC CtorProc);
	void FindDebugFlags(FARPROC SetOptionProc);
	bool ResolveNodeVMTsInternal();

	void SaveNodeVMT(NodeType type, NodeVMT* vmt);

	static bool CallWrapper(uint32_t FunctionHandle, OsiArgumentDesc * Params);
	static bool QueryWrapper(uint32_t FunctionHandle, OsiArgumentDesc * Params);
	static void ErrorWrapper(char const * Message);
	static void AssertWrapper(bool Successful, char const * Message, bool Unknown2);
};

}
