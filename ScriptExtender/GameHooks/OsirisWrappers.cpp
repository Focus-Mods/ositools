#include <stdafx.h>
#include <Extender/ScriptExtender.h>
#include <GameHooks/OsirisWrappers.h>
#include <detours.h>
#include <thread>
#include <psapi.h>

namespace dse
{

#define STATIC_HOOK(name) decltype(OsirisWrappers::name) * decltype(OsirisWrappers::name)::gHook;

STATIC_HOOK(RegisterDivFunctions)
STATIC_HOOK(InitGame)
STATIC_HOOK(DeleteAllData)
STATIC_HOOK(GetFunctionMappings)
STATIC_HOOK(OpenLogFile)
STATIC_HOOK(CloseLogFile)
STATIC_HOOK(Compile)
STATIC_HOOK(Load)
STATIC_HOOK(Merge)
STATIC_HOOK(Event)
STATIC_HOOK(RuleActionCall)
STATIC_HOOK(Call)
STATIC_HOOK(Query)
STATIC_HOOK(Error)
STATIC_HOOK(Assert)
STATIC_HOOK(CreateFileW)
STATIC_HOOK(CloseHandle)
STATIC_HOOK(InitNetworkFixedStrings)
STATIC_HOOK(SkillPrototypeManagerInit)
STATIC_HOOK(eocnet__ClientConnectMessage__Serialize)
STATIC_HOOK(eocnet__ClientAcceptMessage__Serialize)
STATIC_HOOK(esv__OsirisVariableHelper__SavegameVisit)
STATIC_HOOK(TranslatedStringRepository__UnloadOverrides)


OsirisWrappers::OsirisWrappers()
{}

void OsirisWrappers::RegisterDIVFunctionsPreHook(void * Osiris, DivFunctions * Functions)
{
	CallOriginal = Functions->Call;
	QueryOriginal = Functions->Query;
	ErrorOriginal = Functions->ErrorMessage;
	AssertOriginal = Functions->Assert;

	Functions->Call = &CallWrapper;
	Functions->Query = &QueryWrapper;
	Functions->ErrorMessage = &ErrorWrapper;
	Functions->Assert = &AssertWrapper;
}

void OsirisWrappers::Initialize()
{
	Kernel32Module = LoadLibrary(L"kernel32.dll");
	if (Kernel32Module == NULL) {
		Fail("Could not load kernel32.dll");
	}

	OsirisModule = LoadLibrary(L"osiris_x64.dll");
	if (OsirisModule == NULL) {
		Fail("Could not load osiris_x64.dll");
	}

	MODULEINFO moduleInfo;
	if (!GetModuleInformation(GetCurrentProcess(), OsirisModule, &moduleInfo, sizeof(moduleInfo))) {
		Fail("Could not get module info of osiris_x64.dll");
	}

	OsirisDllStart = moduleInfo.lpBaseOfDll;
	OsirisDllSize = moduleInfo.SizeOfImage;

	OriginalRuleActionCallProc = (RuleActionCallProc)FindRuleActionCallProc();
	if (!OriginalRuleActionCallProc) {
		Fail("Could not locate RuleAction::Call in osiris_x64.dll");
	}

	// FIXME - move to BinaryMappings
	FARPROC OsirisCtorProc = GetProcAddress(OsirisModule, "??0COsiris@@QEAA@XZ");
	if (OsirisCtorProc == NULL) {
		Fail("Could not locate COsiris::COsiris() in osiris_x64.dll");
	}

	FindOsirisGlobals(OsirisCtorProc);

	FARPROC SetOptionProc = GetProcAddress(OsirisModule, "?SetOption@COsiris@@QEAAXI@Z");
	if (SetOptionProc == NULL) {
		Fail("Could not locate COsiris::SetOption in osiris_x64.dll");
	}

	FindDebugFlags(SetOptionProc);

#if 0
	DEBUG("OsirisWrappers::Initialize: Detouring functions");
#endif

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	RegisterDivFunctions.Wrap(OsirisModule, "?RegisterDIVFunctions@COsiris@@QEAAXPEAUTOsirisInitFunction@@@Z");
	InitGame.Wrap(OsirisModule, "?InitGame@COsiris@@QEAA_NXZ");
	DeleteAllData.Wrap(OsirisModule, "?DeleteAllData@COsiris@@QEAAX_N@Z");
	GetFunctionMappings.Wrap(OsirisModule, "?GetFunctionMappings@COsiris@@QEAAXPEAPEAUMappingInfo@@PEAI@Z");
	OpenLogFile.Wrap(OsirisModule, "?OpenLogFile@COsiris@@QEAA_NPEB_W0@Z");
	CloseLogFile.Wrap(OsirisModule, "?CloseLogFile@COsiris@@QEAAXXZ");
	Load.Wrap(OsirisModule, "?Load@COsiris@@QEAA_NAEAVCOsiSmartBuf@@@Z");
	Compile.Wrap(OsirisModule, "?Compile@COsiris@@QEAA_NPEB_W0@Z");
	Merge.Wrap(OsirisModule, "?Merge@COsiris@@QEAA_NPEB_W@Z");
	Event.Wrap(OsirisModule, "?Event@COsiris@@QEAA?AW4ReturnCode@osi@@IPEAVCOsiArgumentDesc@@@Z");
	RuleActionCall.Wrap(OriginalRuleActionCallProc);

	Call.Wrap(&CallWrapper);
	Query.Wrap(&QueryWrapper);
	Error.Wrap(&ErrorWrapper);
	// FIXME - PERF Assert.Wrap(&AssertWrapper);

	CreateFileW.Wrap(Kernel32Module, "CreateFileW");
	CloseHandle.Wrap(Kernel32Module, "CloseHandle");

	DetourTransactionCommit();
}

void OsirisWrappers::InitializeDeferredExtensions()
{
	if (DeferredExtensionsInitialized) {
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	auto & lib = GetStaticSymbols();
	if (lib.esv__OsirisVariableHelper__SavegameVisit != nullptr) {
		esv__OsirisVariableHelper__SavegameVisit.Wrap(lib.esv__OsirisVariableHelper__SavegameVisit);
	}

	DetourTransactionCommit();

	DeferredExtensionsInitialized = true;
}

void OsirisWrappers::InitializeExtensions()
{
	if (ExtensionsInitialized) {
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	auto & lib = GetStaticSymbols();
	if (lib.InitNetworkFixedStrings != nullptr) {
		InitNetworkFixedStrings.Wrap(lib.InitNetworkFixedStrings);
	}

	if (lib.SkillPrototypeManager__Init != nullptr) {
		SkillPrototypeManagerInit.Wrap(lib.SkillPrototypeManager__Init);
	}

	if (lib.TranslatedStringRepository__UnloadOverrides != nullptr) {
		TranslatedStringRepository__UnloadOverrides.Wrap(lib.TranslatedStringRepository__UnloadOverrides);
	}

	DetourTransactionCommit();

	ExtensionsInitialized = true;
}

void OsirisWrappers::InitializeNetworking(net::MessageFactory* factory)
{
	if (NetworkingInitialized) {
		return;
	}

	if (factory->MessagePools.size() <= (unsigned)NetMessage::NETMSG_CLIENT_ACCEPT) {
		ERR("MessageFactory not initialized yet?");
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	auto clientConnect = factory->MessagePools[(unsigned)NetMessage::NETMSG_CLIENT_CONNECT]->Template;
	auto clientAccept = factory->MessagePools[(unsigned)NetMessage::NETMSG_CLIENT_ACCEPT]->Template;
	eocnet__ClientConnectMessage__Serialize.Wrap((*(net::MessageVMT**)clientConnect)->Serialize);
	eocnet__ClientAcceptMessage__Serialize.Wrap((*(net::MessageVMT**)clientAccept)->Serialize);

	DetourTransactionCommit();

	NetworkingInitialized = true;
}

void OsirisWrappers::Shutdown()
{
#if 0
	DEBUG("OsirisWrappers::Shutdown: Unregistering hooks");
#endif
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	esv__OsirisVariableHelper__SavegameVisit.Unwrap();
	eocnet__ClientConnectMessage__Serialize.Unwrap();
	eocnet__ClientAcceptMessage__Serialize.Unwrap();

	InitNetworkFixedStrings.Unwrap();
	SkillPrototypeManagerInit.Unwrap();
	TranslatedStringRepository__UnloadOverrides.Unwrap();
	ExtensionsInitialized = false;

	RegisterDivFunctions.Unwrap();
	InitGame.Unwrap();
	DeleteAllData.Unwrap();
	GetFunctionMappings.Unwrap();
	OpenLogFile.Unwrap();
	CloseLogFile.Unwrap();
	Load.Unwrap();
	Compile.Unwrap();
	Merge.Unwrap();
	Event.Unwrap();
	RuleActionCall.Unwrap();

	Call.Unwrap();
	Query.Unwrap();
	Error.Unwrap();
	Assert.Unwrap();

	CreateFileW.Unwrap();
	CloseHandle.Unwrap();

	DetourTransactionCommit();

	FreeModule(Kernel32Module);
	FreeModule(OsirisModule);
}

bool OsirisWrappers::CallWrapper(uint32_t FunctionId, OsiArgumentDesc * Params)
{
	return gExtender->GetServer().Osiris().GetWrappers().CallOriginal(FunctionId, Params);
}

bool OsirisWrappers::QueryWrapper(uint32_t FunctionId, OsiArgumentDesc * Params)
{
	return gExtender->GetServer().Osiris().GetWrappers().QueryOriginal(FunctionId, Params);
}

void OsirisWrappers::ErrorWrapper(char const * Message)
{
	gExtender->GetServer().Osiris().GetWrappers().ErrorOriginal(Message);
}

void OsirisWrappers::AssertWrapper(bool Successful, char const * Message, bool Unknown2)
{
	gExtender->GetServer().Osiris().GetWrappers().AssertOriginal(Successful, Message, Unknown2);
}

void * OsirisWrappers::FindRuleActionCallProc()
{
#if 0
	DEBUG("OsirisWrappers::FindRuleActionCallProc");
#endif
	uint8_t * Addr = static_cast<uint8_t *>(OsirisDllStart);

	// Function prologue of RuleAction::Call
	static const uint8_t instructions[18] = {
		0x40, 0x55, // push rbp
		0x53, // push rbx
		0x56, // push rsi
		0x41, 0x56, // push r14
		0x48, 0x8D, 0x6C, 0x24, 0xC1, // lea rbp, [rsp-3Fh]
		0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00 // sub rsp, 88h
	};

	// Look for prologue in the entire osiris DLL
	for (uint8_t * ptr = Addr; ptr < Addr + OsirisDllSize; ptr++)
	{
		if (*reinterpret_cast<uint64_t *>(ptr) == *reinterpret_cast<uint64_t const *>(&instructions[0])
			&& memcmp(instructions, ptr, sizeof(instructions)) == 0)
		{
			return ptr;
		}
	}

	return nullptr;
}

uint8_t * ResolveRealFunctionAddress(uint8_t * Address)
{
	// Resolve function pointer through relocations
	for (uint8_t * ptr = Address; ptr < Address + 64; ptr++)
	{
		// Look for the instruction "cmp qword ptr [rip+xxxxxx], 0"
		if (ptr[0] == 0x48 && ptr[1] == 0x83 && ptr[2] == 0x3d && ptr[6] == 0x00 &&
			// Look for the instruction "jmp xxxx"
			ptr[13] == 0xe9)
		{
			int32_t relOffset = *reinterpret_cast<int32_t *>(ptr + 14);
			return ptr + relOffset + 18;
		}
	}

	// Could not find any relocations
	return Address;
}

void OsirisWrappers::FindOsirisGlobals(FARPROC CtorProc)
{
#if 0
	DEBUG("ScriptExtender::FindOsirisGlobals:");
#endif
	uint8_t * Addr = ResolveRealFunctionAddress((uint8_t *)CtorProc);

	// Try to find pointers of Osiris globals
	const unsigned NumGlobals = 8;
	uint8_t * globals[NumGlobals];
	unsigned foundGlobals = 0;
	for (uint8_t * ptr = Addr; ptr < Addr + 0x300; ptr++)
	{
		// Look for the instruction "jmp short $+7"
		if (ptr[0] == 0xEB && ptr[1] == 0x07 &&
			// Look for the instruction "mov cs:[rip + xxx], <64-bit register>"
			ptr[2] == 0x48 && ptr[3] == 0x89 && (ptr[4] & 0xC7) == 0x05)
		{
			int32_t relOffset = *reinterpret_cast<int32_t *>(ptr + 5);
			uint64_t osiPtr = (uint64_t)ptr + relOffset + 9;
			globals[foundGlobals++] = (uint8_t *)osiPtr;
			if (foundGlobals == NumGlobals) break;
		}
	}

	if (foundGlobals < NumGlobals)
	{
		Fail("Could not locate global Osiris variables");
	}

	Globals.Variables = (VariableDb **)globals[0];
	Globals.Types = (OsiTypeDb **)globals[1];
	Globals.Functions = (FunctionDb **)globals[2];
	Globals.Objects = (ObjectDb **)globals[3];
	Globals.Goals = (GoalDb **)globals[4];
	Globals.Adapters = (AdapterDb **)globals[5];
	Globals.Databases = (DatabaseDb **)globals[6];
	Globals.Nodes = (NodeDb **)globals[7];

#if 0
	DEBUG("\tVariables = %p", Globals.Variables);
	DEBUG("\tTypes = %p", Globals.Types);
	DEBUG("\tFunctions = %p", Globals.Functions);
	DEBUG("\tObjects = %p", Globals.Objects);
	DEBUG("\tGoals = %p", Globals.Goals);
	DEBUG("\tAdapters = %p", Globals.Adapters);
	DEBUG("\tDatabases = %p", Globals.Databases);
	DEBUG("\tNodes = %p", Globals.Nodes);
#endif
}

void OsirisWrappers::FindDebugFlags(FARPROC SetOptionProc)
{
	uint8_t * Addr = ResolveRealFunctionAddress((uint8_t *)SetOptionProc);

	// Try to find pointer of global var DebugFlags
	for (uint8_t * ptr = Addr; ptr < Addr + 0x80; ptr++)
	{
		// Look for the instruction "mov ecx, cs:xxx"
		if (ptr[0] == 0x8B && ptr[1] == 0x0D &&
			// Look for the instruction "shr e*x, 14h"
			ptr[8] == 0xC1 && ptr[10] == 0x14)
		{
			int32_t relOffset = *reinterpret_cast<int32_t *>(ptr + 2);
			uint64_t dbgPtr = (uint64_t)ptr + relOffset + 6;
			Globals.DebugFlags = (DebugFlag *)dbgPtr;
			break;
		}
	}

	if (Globals.DebugFlags == nullptr) {
		Fail("Could not locate global variable DebugFlags");
	}

#if 0
	DEBUG("ScriptExtender::FindDebugFlags: DebugFlags = %p", Globals.DebugFlags);
#endif
}

void OsirisWrappers::SaveNodeVMT(NodeType type, NodeVMT * vmt)
{
	assert(type >= NodeType::Database && type <= NodeType::Max);
	VMTs[(unsigned)type] = vmt;
}

bool OsirisWrappers::ResolveNodeVMTs()
{
	if (!resolvedVMTs_ || !*resolvedVMTs_) {
		resolvedVMTs_ = ResolveNodeVMTsInternal();
	}

	return *resolvedVMTs_;
}

bool OsirisWrappers::ResolveNodeVMTsInternal()
{
	auto Db = *Globals.Nodes;

#if 0
	DEBUG("ScriptExtender::ResolveNodeVMTs");
#endif
	std::set<NodeVMT *> VMTs;

	for (unsigned i = 0; i < Db->Db.Size; i++) {
		auto node = Db->Db.Start[i];
		auto vmt = *(NodeVMT **)node;
		VMTs.insert(vmt);
	}

	if (VMTs.size() != (unsigned)NodeType::Max) {
		OsiErrorS("Could not locate all Osiris node VMT-s");
		return false;
	}

	// RuleNode has a different SetLineNumber implementation
	void * srv{ nullptr };
	std::vector<NodeVMT *> srvA, srvB;
	for (auto vmt : VMTs) {
		if (srv == nullptr) {
			srv = vmt->SetLineNumber;
		}

		if (srv == vmt->SetLineNumber) {
			srvA.push_back(vmt);
		} else {
			srvB.push_back(vmt);
		}
	}

	NodeVMT * ruleNodeVMT;
	if (srvA.size() == 1) {
		ruleNodeVMT = *srvA.begin();
	} else if (srvB.size() == 1) {
		ruleNodeVMT = *srvB.begin();
	} else {
		OsiErrorS("Could not locate RuleNode::__vfptr");
		return false;
	}

#if 0
	DEBUG("RuleNode::__vfptr is %p", ruleNodeVMT);
#endif
	SaveNodeVMT(NodeType::Rule, ruleNodeVMT);

	// RelOpNode is the only node that has the same GetAdapter implementation
	NodeVMT * relOpNodeVMT{ nullptr };
	for (auto vmt : VMTs) {
		if (vmt->GetAdapter == ruleNodeVMT->GetAdapter
			&& vmt != ruleNodeVMT) {
			if (relOpNodeVMT == nullptr) {
				relOpNodeVMT = vmt;
			} else {
				OsiErrorS("RelOpNode::__vfptr pattern matches multiple VMT-s");
				return false;
			}
		}
	}

	if (relOpNodeVMT == nullptr) {
		OsiErrorS("Could not locate RelOpNode::__vfptr");
		return false;
	}

#if 0
	DEBUG("RuleNode::__vfptr is %p", relOpNodeVMT);
#endif
	SaveNodeVMT(NodeType::RelOp, relOpNodeVMT);

	// Find And, NotAnd
	NodeVMT * and1VMT{ nullptr }, *and2VMT{ nullptr };
	for (auto vmt : VMTs) {
		if (vmt->SetNextNode == ruleNodeVMT->SetNextNode
			&& vmt->GetAdapter != ruleNodeVMT->GetAdapter) {
			if (and1VMT == nullptr) {
				and1VMT = vmt;
			} else if (and2VMT == nullptr) {
				and2VMT = vmt;
			} else {
				OsiErrorS("AndNode::__vfptr pattern matches multiple VMT-s");
				return false;
			}
		}
	}

	if (and1VMT == nullptr || and2VMT == nullptr) {
		OsiErrorS("Could not locate AndNode::__vfptr");
		return false;
	}

#if 0
	DEBUG("AndNode::__vfptr is %p and %p", and1VMT, and2VMT);
#endif
	// No reliable way to detect these; assume that AndNode VMT < NotAndNode VMT
	if (and1VMT < and2VMT) {
		SaveNodeVMT(NodeType::And, and1VMT);
		SaveNodeVMT(NodeType::NotAnd, and2VMT);
	} else {
		SaveNodeVMT(NodeType::NotAnd, and1VMT);
		SaveNodeVMT(NodeType::And, and2VMT);
	}

	// Find Query nodes
	void * snn{ nullptr };
	std::map<void *, std::vector<NodeVMT *>> snnMaps;
	std::vector<NodeVMT *> * queryVMTs{ nullptr };
	for (auto vmt : VMTs) {
		if (snnMaps.find(vmt->SetNextNode) == snnMaps.end()) {
			std::vector<NodeVMT *> nvmts{ vmt };
			snnMaps.insert(std::make_pair(vmt->SetNextNode, nvmts));
		} else {
			snnMaps[vmt->SetNextNode].push_back(vmt);
		}
	}

	for (auto & map : snnMaps) {
		if (map.second.size() == 3) {
			queryVMTs = &map.second;
			break;
		}
	}

	if (queryVMTs == nullptr) {
		OsiErrorS("Could not locate all Query node VMT-s");
		return false;
	}

	for (auto vmt : *queryVMTs) {
		auto getName = (NodeVMT::GetQueryNameProc)vmt->GetQueryName;
		std::string name{ getName(nullptr) };
		if (name == "internal query") {
#if 0
			DEBUG("InternalQuery::__vfptr is %p", vmt);
#endif
			SaveNodeVMT(NodeType::InternalQuery, vmt);
		} else if (name == "DIV query") {
#if 0
			DEBUG("DivQuery::__vfptr is %p", vmt);
#endif
			SaveNodeVMT(NodeType::DivQuery, vmt);
		} else if (name == "Osi user query") {
#if 0
			DEBUG("UserQuery::__vfptr is %p", vmt);
#endif
			SaveNodeVMT(NodeType::UserQuery, vmt);
		} else {
			OsiErrorS("Unrecognized Query node VMT");
			return false;
		}
	}


	// Proc node has different IsProc() code
	NodeVMT * procNodeVMT{ nullptr }, * databaseNodeVMT{ nullptr };
	for (auto vmt : VMTs) {
		if (vmt->IsProc != ruleNodeVMT->IsProc) {
			if (procNodeVMT == nullptr) {
				procNodeVMT = vmt;
			} else {
				OsiErrorS("ProcNode::__vfptr pattern matches multiple VMT-s");
				return false;
			}
		}

		if (vmt->IsDataNode != ruleNodeVMT->IsDataNode
			&& vmt->IsProc == ruleNodeVMT->IsProc) {
			if (databaseNodeVMT == nullptr) {
				databaseNodeVMT = vmt;
			} else {
				OsiErrorS("DatabaseNode::__vfptr pattern matches multiple VMT-s");
				return false;
			}
		}
	}

	if (procNodeVMT == nullptr) {
		OsiErrorS("Could not locate ProcNode::__vfptr");
		return false;
	}

	if (databaseNodeVMT == nullptr) {
		OsiErrorS("Could not locate DatabaseNode::__vfptr");
		return false;
	}

#if 0
	DEBUG("ProcNode::__vfptr is %p", procNodeVMT);
	DEBUG("DatabaseNode::__vfptr is %p", databaseNodeVMT);
#endif
	SaveNodeVMT(NodeType::Proc, procNodeVMT);
	SaveNodeVMT(NodeType::Database, databaseNodeVMT);
	return true;
}

}
