#include <stdafx.h>
#include <GameHooks/DataLibraries.h>
#include <Extender/ScriptExtender.h>
#include <GameDefinitions/Symbols.h>
#include <functional>
#include <psapi.h>

#if defined(OSI_EOCAPP)
namespace dse
{
	SymbolMappingData const sSymbolLevelManager = {
		"esv::LevelManager2",
		SymbolMappingData::kCustom, 0,
		"48 8B 05 XX XX XX XX " // mov     rax, cs:ls__gServerLevelAllocator
		"4C 8D 44 24 60 " // lea     r8, [rsp+58h+arg_0]
		"48 8D 54 24 70 " // lea     rdx, [rsp+58h+arg_10]
		"C7 44 24 60 FF FF FF FF " // mov     dword ptr [rsp+58h+arg_0], 0FFFFFFFFh
		"48 8B 88 80 00 00 00 " //  mov     rcx, [rax+80h]
		"48 8B 05 XX XX XX XX " // mov     rax, cs:ls__ObjectHandle__Unassigned
		"48 89 44 24 70 " // mov     [rsp+58h+arg_10], rax
		"48 8B B9 D0 00 00 00 " // mov     rdi, [rcx+0D0h]
		"48 8B CF " // mov     rcx, rdi
		"E8 XX XX XX XX " // call    xxx
		"4C 8D 46 70 " // lea     r8, [rsi+70h]
		"48 8B C8 " // mov     rcx, rax
		"48 8D 56 68 " // lea     rdx, [rsi+68h]
		"48 8B D8 " // mov     rbx, rax
		"E8 XX XX XX XX " // call    esv__GameObjectMoveAction__Setup
		"48 8D 8F 30 01 00 00 " // lea     rcx, [rdi+130h]
		"48 89 5C 24 60 " // mov     [rsp+58h+arg_0], rbx
		"48 8D 54 24 60 " // lea     rdx, [rsp+58h+arg_0]
		"E8 XX XX XX XX ", // call    esv__GameActionManager__AddAction
		{},
		{"esv::LevelManager", SymbolMappingTarget::kIndirect, 0, STATIC_SYM(EsvLevelManager)},
		{"esv::GameObjectMoveAction::Setup", SymbolMappingTarget::kIndirect, 73, STATIC_SYM(GameObjectMoveActionSetup)},
		{"esv::GameActionManager::AddAction", SymbolMappingTarget::kIndirect, 95, STATIC_SYM(AddGameAction)}
	};

	SymbolMappingData const sSymbolWallAction = {
		"WallAction2",
		SymbolMappingData::kCustom, 0,
		"48 8B C4 " // mov     rax, rsp
		"48 89 58 10 " // mov     [rax+10h], rbx
		"48 89 70 18 ", // mov     [rax+18h], rsi
		{},
		{"esv::WallAction::CreateWall", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(WallActionCreateWall)}
	};

	SymbolMappingData const sSymbolTornadoAction = {
		"TornadoAction2",
		SymbolMappingData::kCustom, 0,
		"48 8B C4 " // mov     rax, rsp
		"53 ", // push    rbx
		{},
		{"esv::TornadoAction::Setup", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(TornadoActionSetup)}
	};

	SymbolMappingData const sSymbolSummonHelpersSummon = {
		"esv::SummonHelpers::Summon",
		SymbolMappingData::kCustom, 0,
		"48 8D 54 24 70 " // lea     rdx, [rsp+220h+args]
		"40 88 75 A1 " // mov     byte ptr [rbp+120h+var_180+1], sil
		"48 8D 4D 30 " // lea     rcx, [rbp+120h+results]
		"E8 XX XX XX XX " // call    esv__SummonHelpers__Summon
		"48 8B 55 30 ", // mov     rdx, [rbp+120h+results.SummonCharacterHandle]
		{},
		{"esv::SummonHelpers::Summon", SymbolMappingTarget::kIndirect, 13, STATIC_SYM(SummonHelpersSummon)}
	};

	SymbolMappingData const sSymbolApplyStatus = {
		"ApplyStatus",
		SymbolMappingData::kCustom, 0,
		"C7 43 2C 00 00 00 00 " // mov     dword ptr [rbx+2Ch], 0
		"48 8B CF " // mov     rcx, rdi
		"E8 XX XX XX XX " // call    esv__StatusMachine__ApplyStatus
		"48 8B 7C 24 40 ", // mov     rdi, [rsp+28h+arg_10]
		{},
		{"esv::StatusMachine::ApplyStatus", SymbolMappingTarget::kIndirect, 10, STATIC_SYM(esv__StatusMachine__ApplyStatus)}
	};

	SymbolMappingResult FindStatusHitEoCApp2(uint8_t const * match)
	{
		auto & library = gExtender->GetLibraryManager();
		auto moduleStart = library.GetModuleStart();
		auto moduleSize = library.GetModuleSize();

		// Look for this function ptr
		auto ptr = (uint64_t)match;
		for (auto p = moduleStart; p < moduleStart + moduleSize; p += 8) {
			if (*reinterpret_cast<uint64_t const *>(p) == ptr) {
				GetStaticSymbols().StatusHitVMT = reinterpret_cast<esv::StatusVMT const *>(p - 12 * 8);
				return SymbolMappingResult::Success;
			}
		}

		return SymbolMappingResult::Fail;
	}

	SymbolMappingData const sSymbolStatusHit = {
		"esv::StatusHit::__vftable",
		SymbolMappingData::kCustom, 0,
		"40 55 " // push    rbp
		"41 54 " // push    r12
		"41 55 " // push    r13
		"41 57 ", // push    r15
		{},
		{"esv::StatusHit::__vftable", SymbolMappingTarget::kAbsolute, 0, nullptr, &FindStatusHitEoCApp2}
	};

	SymbolMappingData const sSymbolStatusHit64 = {
		"esv::StatusHit::__vftable",
		SymbolMappingData::kCustom, 0,
		"40 55 " // push    rbp
		"41 54 " // push    r12
		"41 57 ", // push    r15
		{},
		{"esv::StatusHit::__vftable", SymbolMappingTarget::kAbsolute, 0, nullptr, &FindStatusHitEoCApp2}
	};

	SymbolMappingResult FindStatusHealEoCApp2(uint8_t const * match)
	{
		auto & library = gExtender->GetLibraryManager();
		auto moduleStart = library.GetModuleStart();
		auto moduleSize = library.GetModuleSize();

		// Look for this function ptr
		auto ptr = (uint64_t)match;
		for (auto p = moduleStart; p < moduleStart + moduleSize; p += 8) {
			if (*reinterpret_cast<uint64_t const *>(p) == ptr) {
				GetStaticSymbols().StatusHealVMT = reinterpret_cast<esv::StatusVMT const *>(p - 25 * 8);
				return SymbolMappingResult::Success;
			}
		}

		return SymbolMappingResult::Fail;
	}

	SymbolMappingData const sSymbolStatusHeal = {
		"esv::StatusHeal::__vftable",
		SymbolMappingData::kCustom, 0,
		"48 89 5C 24 10 " // mov     [rsp-8+arg_8], rbx
		"48 89 74 24 18 ", // mov     [rsp-8+arg_10], rsi
		{},
		{"esv::StatusHeal::__vftable", SymbolMappingTarget::kAbsolute, 0, nullptr, &FindStatusHealEoCApp2}
	};

	SymbolMappingResult FindActivateEntitySystemEoCApp(uint8_t const * match)
	{
		if (GetStaticSymbols().ActivateClientSystemsHook == nullptr) {
			GetStaticSymbols().ActivateClientSystemsHook = match;
			return SymbolMappingResult::TryNext;
		} else {
			GetStaticSymbols().ActivateServerSystemsHook = match;
			return SymbolMappingResult::Success;
		}
	}

	SymbolMappingData const sSymbolGetAbility = {
		"GetAbility",
		SymbolMappingData::kCustom, 0,
		"45 33 C9 " // xor     r9d, r9d 
		"45 33 C0 " // xor     r8d, r8d
		"8B D0 " // mov     edx, eax 
		"48 8B CB " // mov     rcx, rbx 
		"E8 XX XX XX XX ", // call    CDivinityStats_Character__GetAbility
		{},
		{"GetAbility", SymbolMappingTarget::kIndirect, 11, CHAR_GETTER_SYM(GetAbility)}
	};

	SymbolMappingData const sSymbolGetTalent = {
		"GetTalent",
		SymbolMappingData::kCustom, 0,
		"45 0F B6 C7 " // movzx   r8d, r15b
		"BA 03 00 00 00 " // mov     edx, 3
		"48 8B CB " // mov     rcx, rbx
		"03 F7 " // add     esi, edi
		"E8 XX XX XX XX ", // call    CDivinityStats_Character__HasTalent
		{},
		{"GetTalent", SymbolMappingTarget::kIndirect, 14, CHAR_GETTER_SYM(GetTalent)}
	};

	SymbolMappingResult FindCharacterStatGettersEoCApp(uint8_t const * match)
	{
		unsigned ptrIndex = 0;
		auto & getters = GetStaticSymbols().CharStatsGetters;
		for (auto p = match; p < match + 0x240; p++) {
			auto insn = *reinterpret_cast<uint32_t const *>(p);
			if (insn == 0xE8CE8B48 /* 48 8B CE E8 -- mov  rcx, rsi; call */
				|| insn == 0xE8084389 /* 89 43 08 E8 -- mov [rbx+8], eax; call */) {
				if (ptrIndex < std::size(getters.Ptrs)) {
					auto ptr = AsmResolveInstructionRef(p + 3);
					getters.Ptrs[ptrIndex++] = (void *)ptr;
				}
			} else if (insn == 0x00908389) { /* mov [rbx+90h], eax */
				if (ptrIndex < std::size(getters.Ptrs)) {
					auto ptr = AsmResolveInstructionRef(p + 6);
					getters.Ptrs[ptrIndex++] = (void *)ptr;
				}
			}
		}

		if (getters.GetBlockChance != nullptr) {
			auto & library = gExtender->GetLibraryManager();
			library.MapSymbol(sSymbolGetAbility, (uint8_t *)getters.GetDodge, 0x480);
			library.MapSymbol(sSymbolGetTalent, (uint8_t *)getters.GetDodge, 0x480);
		}

		return (getters.GetBlockChance != nullptr) ? SymbolMappingResult::Success : SymbolMappingResult::Fail;
	}

	SymbolMappingData const sSymbolChanceToHitBoost = {
		"GetChanceToHitBoost",
			SymbolMappingData::kCustom, 0,
			"48 0F 4D C2 " // cmovge  rax, rdx
			"8B 18 " // mov     ebx, [rax]
			"E8 XX XX XX XX ", // call    CDivinityStats_Character__GetChanceToHitBoost
		{},
		{"GetChanceToHitBoost", SymbolMappingTarget::kIndirect, 6, CHAR_GETTER_SYM(GetChanceToHitBoost)}
	};

	SymbolMappingData const sSymbolCharacterHitInternal = {
		"CDivinityStats_Character::_HitInternal",
		SymbolMappingData::kCustom, 0,
		"88 44 24 28 " // mov     [rsp+98h+noHitRoll], al
		"89 7C 24 20 " // mov     [rsp+98h+hitType], edi
		"E8 XX XX XX XX ", // call    CDivinityStats_Character___HitInternal
		{},
		{"CDivinityStats_Character::_HitInternal", SymbolMappingTarget::kIndirect, 8, STATIC_SYM(CDivinityStats_Character__HitInternal)}
	};

	SymbolMappingData const sSymbolCharacterHit = {
		"esv::Character::CharacterHit",
		SymbolMappingData::kCustom, 0,
		"C7 44 24 20 05 00 00 00 " // mov     dword ptr [rsp+140h+var_120], 5
		"44 89 65 50 " // mov     [rbp+40h+arg_0], r12d
		"E8 XX XX XX XX " // call    esv__Character__Hit
		"4C 8B AE A0 01 00 00 " // mov     r13, [rsi+1A0h]
		"EB 5F ", // jmp short xxx
		{},
		{"esv::Character::CharacterHit", SymbolMappingTarget::kIndirect, 12, STATIC_SYM(esv__Character__Hit), nullptr, &sSymbolCharacterHitInternal, 0x280}
	};

	SymbolMappingResult FindLibrariesEoCApp(uint8_t const * match)
	{
		auto & lib = GetStaticSymbols().Libraries;

		auto initFunc = AsmResolveInstructionRef(match + 18);
		auto freeFunc = AsmResolveInstructionRef(match + 25);
		if (initFunc != nullptr && freeFunc != nullptr) {
			auto it = lib.find(initFunc);
			if (it != lib.end()) {
				it->second.refs++;
			} else {
				lib.insert(std::pair<uint8_t const *, StaticSymbols::EoCLibraryInfo>(initFunc, { initFunc, freeFunc, 1 }));
			}

			return SymbolMappingResult::TryNext;
		} else {
			return SymbolMappingResult::Fail;
		}
	}

	SymbolMappingResult FindActionMachineSetState(uint8_t const * match)
	{
		if (GetStaticSymbols().ecl__ActionMachine__SetState == nullptr) {
			GetStaticSymbols().ecl__ActionMachine__SetState = (esv::ActionMachine::SetStateProc*)match;
			return SymbolMappingResult::TryNext;
		} else {
			GetStaticSymbols().esv__ActionMachine__SetState = (esv::ActionMachine::SetStateProc*)match;
			return SymbolMappingResult::Success;
		}
	}

	SymbolMappingResult FindActionMachineResetState(uint8_t const* match)
	{
		if (GetStaticSymbols().ecl__ActionMachine__ResetState == nullptr) {
			GetStaticSymbols().ecl__ActionMachine__ResetState = (esv::ActionMachine::ResetStateProc*)match;
			return SymbolMappingResult::TryNext;
		}
		else {
			GetStaticSymbols().esv__ActionMachine__ResetState = (esv::ActionMachine::ResetStateProc*)match;
			return SymbolMappingResult::Success;
		}
	}

	SymbolMappingData const sSymbolSkillPrototypeFormatDescriptionParam = {
		"eoc::SkillPrototype::FormatDescriptionParam2",
		SymbolMappingData::kCustom, 0,
		"4C 8B DC " // mov     r11, rsp
		"55 " // push    rbp
		"53 ", // push    rbx
		{},
		{"eoc::SkillPrototype::FormatDescriptionParam2", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(SkillPrototype__FormatDescriptionParam)}
	};

	SymbolMappingData const sSymbolSkillPrototypeGetSkillDamage = {
		"eoc::SkillPrototype::GetSkillDamage2",
		SymbolMappingData::kCustom, 0,
		"44 88 4C 24 20 " // mov     byte ptr [rsp+arg_18], r9b
		"48 89 54 24 10 ", // mov     [rsp+damageList], rdx
		{},
		{"eoc::SkillPrototype::GetSkillDamage2", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(SkillPrototype__GetSkillDamage)}
	};

	SymbolMappingData const sSymbolStatusPrototypeFormatDescriptionParam = {
		"eoc::StatusPrototype::FormatDescriptionParam2",
		SymbolMappingData::kCustom, 0,
		"4C 8B DC " // mov     r11, rsp
		"55 " // push    rbp
		"56 ", // push    rsi
		{},
		{"eoc::StatusPrototype::FormatDescriptionParam2", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(StatusPrototype__FormatDescriptionParam)}
	};

	SymbolMappingData const sSymbolEoCUIvftable = {
		"ecl::EoCUI::vftable",
		SymbolMappingData::kCustom, 0,
		"C7 83 48 01 00 00 00 00 00 00 " // mov     dword ptr [rbx+148h], 0
		"48 8D 05 XX XX XX XX ", // lea     rax, ecl__EoCUI__vftable
		{},
		{"ecl::EoCUI::vftable", SymbolMappingTarget::kIndirect, 10, STATIC_SYM(EoCUI__vftable)}
	};

	SymbolMappingData const sSymbolMappings[] = {
		{
			"EoCMemoryMgr", 
			SymbolMappingData::kText, SymbolMappingData::kCritical,
			"89 41 10 " // mov     [rcx+10h], eax
			"E8 XX XX XX XX " // call    ls__GlobalAllocator__Free
			"48 8B 47 08 " // mov     rax, [rdi+8]
			"BA 28 01 00 00 " // mov     edx, 128h
			"48 89 46 08 " // mov     [rsi+8], rax
			"48 C7 47 08 00 00 00 00 " // mov     qword ptr [rdi+8], 0
			"C7 07 00 00 00 00 " // mov     dword ptr [rdi], 0
			"E8 XX XX XX XX " // call    ls__GlobalAllocator__Malloc
			"33 D2 ", // xor     edx, edx 
			{}, // Unconditional
			{"EoCAlloc", SymbolMappingTarget::kIndirect, 35, STATIC_SYM(EoCAlloc)},
			{"EoCFree", SymbolMappingTarget::kIndirect, 3, STATIC_SYM(EoCFree)}
		},

		{
			"ls::FixedString::Create",
			SymbolMappingData::kText, SymbolMappingData::kCritical,
			"48 8B F0 " // mov     rsi, rax
			"48 85 C0 " // test    rax, rax
			"0F XX XX XX 00 00 " // jz      loc_xxx
			"83 CA FF " // or      edx, 0FFFFFFFFh
			"4C 89 74 XX 60 " // mov     [rsp+0DB0h+var_D50], r14
			"48 8D 0D XX XX XX XX " // lea     rcx, str_Damage ; "Damage"
			"E8 XX XX XX XX " // call    ls__FixedString__Create
			"83 CA FF ", // or      edx, 0FFFFFFFFh
			{SymbolMappingCondition::kString, 20, "Damage"},
			{"ls::FixedString::Create", SymbolMappingTarget::kIndirect, 27, STATIC_SYM(CreateFixedString)}
		},

		{
			"ls::FileReader::FileReader",
			SymbolMappingData::kText, SymbolMappingData::kCritical,
			"F3 0F 7F 45 D7 " // movdqu  [rbp+4Fh+var_78], xmm0
			"E8 XX XX XX XX " // call    xxx
			"45 33 C0 " // xor     r8d, r8d
			"89 75 17 " // mov     [rbp+4Fh+var_38], esi
			"48 8B D7 " // mov     rdx, rdi
			"48 89 75 1F " // mov     [rbp+4Fh+var_30], rsi
			"48 8D 4D CF " // lea     rcx, [rbp+4Fh+var_80]
			"E8 XX XX XX XX " // call    ls__FileReader__FileReader
			"48 8D 55 CF " // lea     rdx, [rbp+4Fh+var_80]
			"48 8D 4C 24 30 " // lea     rcx, [rsp+110h+var_E0]
			"E8 XX XX XX XX " // call    sub_1411CF840
			"48 8D 4D CF " // lea     rcx, [rbp+4Fh+var_80]
			"E8 XX XX XX XX ", // call    ls__FileReader__dtor
			{}, // Unconditional
			{"ls::FileReader::FileReader", SymbolMappingTarget::kIndirect, 27, STATIC_SYM(FileReaderCtor)},
			{"ls::FileReader::~FileReader", SymbolMappingTarget::kIndirect, 50, STATIC_SYM(FileReaderDtor)}
		},

		// TODO - find FileReaderDtor

		{
			"PathRoots",
			SymbolMappingData::kText, SymbolMappingData::kCritical,
			"48 83 EC 20 " // sub     rsp, 20h
			"0F B7 C2 " // movzx   eax, dx
			"48 8B D9 " // mov     rbx, rcx
			"48 8D 0D XX XX XX XX " // lea     rcx, ls__Path__s_Roots
			"49 8B F8 " // mov     rdi, r8
			"48 8D 34 C1 ", // lea     rsi, [rcx+rax*8]
			{}, // Unconditional
			{"PathRoots", SymbolMappingTarget::kIndirect, 10, STATIC_SYM(PathRoots)}
		},

		{
			"esv::GameActionManager::CreateAction",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"48 8D 15 XX XX XX XX " // lea     rdx, fsx_GameAction
			"XX 8B XX " // mov     xx, xx
			"FF 90 80 00 00 00 " // call    qword ptr [rax+80h]
			"84 C0 " // test    al, al
			"0F 84 XX XX 00 00 " // jz      xxx
			"45 84 FF " // test    r15b, r15b
			"74 XX " // jz      short loc_xxx
			"4C 8B 06 " // mov     r8, [rsi]
			"49 8B XX " // mov     rcx, xx
			"8B 56 08 " // mov     edx, [rsi+8]
			"E8 XX XX XX XX ", // call    esv__GameActionManager__CreateAction
			{SymbolMappingCondition::kFixedString, 0, "GameAction"},
			{"esv::GameActionManager::CreateAction", SymbolMappingTarget::kIndirect, 38, STATIC_SYM(CreateGameAction)}
		},

		{
			"esv::ProjectileHelpers::ShootProjectile",
			SymbolMappingData::kText, 0,
			"C6 45 AE 01 " // mov     [rbp+50h+var_A2], 1
			"F6 80 08 02 00 00 0C " // test    byte ptr [rax+208h], 0Ch
			"0F 95 45 AD " // setnz   [rbp+50h+var_A3]
			"E8 XX XX XX XX ", // call    esv__ProjectileHelpers__ShootProjectile
			{},
			{"esv::ProjectileHelpers::ShootProjectile", SymbolMappingTarget::kIndirect, 15, STATIC_SYM(esv__ProjectileHelpers__ShootProjectile)}
		},

		{
			"esv::Projectile::Explode",
			SymbolMappingData::kText, 0,
			"48 8D AC 24 F0 FD FF FF " // lea     rbp, [rsp-210h]
			"48 81 EC 10 03 00 00 " // sub     rsp, 310h
			"44 0F 29 B4 24 80 02 00 00 " // movaps  [rsp+330h+var_B0], xmm14
			"44 0F 29 BC 24 70 02 00 00 ", // movaps  [rsp+330h+var_C0], xmm15
			{},
			{"esv::Projectile::Explode", SymbolMappingTarget::kAbsolute, -0x13, STATIC_SYM(esv__Projectile__Explode)}
		},

		{
			"esv::LevelManager",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"0F 84 XX 00 00 00 " // jz      xxx
			"4C 8B 00 " // mov     r8, [rax]
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_ForceMove
			"48 8B C8 " // mov     rcx, rax
			"41 FF 50 28 " // call    qword ptr [r8+28h]
			"4D 8B 06 " // mov     r8, [r14]
			"49 8B CE ", // mov     rcx, r14
			{SymbolMappingCondition::kFixedString, 9, "ForceMove"},
			{"esv::LevelManager", SymbolMappingTarget::kAbsolute, 0, nullptr, nullptr, &sSymbolLevelManager, 0x100}
		},

		{
			"esv::WallAction::CreateWall",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"F3 41 0F 5E C6 " // divss   xmm0, xmm14
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_Template1
			"49 8B CC " // mov     rcx, r12
			"41 8B FE ", // mov     edi, r14d
			{SymbolMappingCondition::kFixedString, 5, "Template1"},
			{"esv::WallAction::CreateWall", SymbolMappingTarget::kAbsolute, -0x300, nullptr, nullptr, &sSymbolWallAction, 0x300}
		},

		{
			"TornadoAction",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"EB XX " // jmp     short xxx
			"48 8B 00 " // mov     rax, [rax]
			"48 8D 15 XX XX XX XX " // lea     rdx, fsx_RandomPoints
			"48 8B XX " // mov     rcx, rbx
			"FF 50 28 " // call    qword ptr [rax+28h]
			"4C 8B XX ", // mov     r8, [rbx]
			{SymbolMappingCondition::kFixedString, 5, "RandomPoints"},
			{"TornadoAction", SymbolMappingTarget::kAbsolute, -0x200, nullptr, nullptr, &sSymbolTornadoAction, 0x200}
		},

		{
			"esv::SummonHelpers::Summon",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"44 0F 28 94 24 90 01 00 00 " // movaps  xmm10, [rsp+220h+var_90]
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_SpawnObject
			"49 8B CC ", // mov     rcx, r12
			{SymbolMappingCondition::kFixedString, 9, "SpawnObject"},
			{"esv::SummonHelpers::Summon", SymbolMappingTarget::kAbsolute, -0x400, nullptr, nullptr, &sSymbolSummonHelpersSummon, 0x400}
		},

		{
			"esv::StatusMachine::CreateStatus",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"83 7A 1C 00 " // cmp     dword ptr [rdx+1Ch], 0
			"48 8B F2 " // mov     rsi, rdx
			"4C 8B F1 " // mov     r14, rcx
			"0F 8E 8C 00 00 00 " // jle     short xxx
			"4C 8B 05 XX XX XX XX " // mov     r8, cs:?Unassigned@ObjectHandle@ls@@2V12@B
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_LIFESTEAL
			"48 89 5C 24 30 " //  mov     [rsp+28h+arg_0], rbx
			"48 89 7C 24 40 " //  mov     [rsp+28h+arg_10], rdi
			"48 8B B9 A0 01 00 00 " //  mov     rdi, [rcx+1A0h]
			"48 8B CF " //  mov     rcx, rdi 
			"E8 XX XX XX XX ", //  call    esv__StatusMachine__CreateStatus
			{SymbolMappingCondition::kFixedString, 23, "LIFESTEAL"},
			{"esv::StatusMachine::CreateStatus", SymbolMappingTarget::kIndirect, 50, STATIC_SYM(esv__StatusMachine__CreateStatus)},
			{"esv::StatusMachine::ApplyStatus", SymbolMappingTarget::kAbsolute, 55, nullptr, nullptr, &sSymbolApplyStatus, 0x100},
		},

		{
			"esv::StatusHit::__vftable <64",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"4C 8D 0D XX XX XX XX " // lea     r9, fsx_Dummy_BodyFX
			"48 8D 15 XX XX XX XX " // lea     rdx, fsx_RS3_FX_GP_Status_Retaliation_Beam_01
			"E8 XX XX XX XX " // call    esv__EffectFactory__CreateEffectWrapper
			"48 8B D8 ", // mov     rbx, rax
			{SymbolMappingCondition::kFixedString, 7, "RS3_FX_GP_Status_Retaliation_Beam_01"},
			{"esv::StatusHit::__vftable", SymbolMappingTarget::kAbsolute, -0xa00, nullptr, nullptr, &sSymbolStatusHit, 0xa00}, {}, {},
			{SymbolVersion::Below, 64} // Before GB5
		},

		{
			"esv::StatusHit::__vftable >=64",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"4C 8D 0D XX XX XX XX " // lea     r9, fsx_Dummy_BodyFX
			"48 8D 15 XX XX XX XX " // lea     rdx, fsx_RS3_FX_GP_Status_Retaliation_Beam_01
			"E8 XX XX XX XX " // call    esv__EffectFactory__CreateEffectWrapper
			"48 8B D8 ", // mov     rbx, rax
			{SymbolMappingCondition::kFixedString, 7, "RS3_FX_GP_Status_Retaliation_Beam_01"},
			{"esv::StatusHit::__vftable", SymbolMappingTarget::kAbsolute, -0x800, nullptr, nullptr, &sSymbolStatusHit64, 0x800}, {}, {},
			{SymbolVersion::AboveOrEqual, 64} // After GB5
		},

		{
			"esv::StatusHit::Setup",
			SymbolMappingData::kText, 0,
			"40 55 " // push    rbp
			"41 54 " // push    r12
			"48 83 EC 38 " // sub     rsp, 38h
			"4C 89 74 24 28 " // mov     [rsp+48h+var_20], r14
			"48 8B E9 " // mov     rbp, rcx
			"4C 8D B1 A0 00 00 00 " // lea     r14, [rcx+0A0h]
			"4C 89 7C 24 20 " // mov     [rsp+48h+var_28], r15
			"49 8B CE " // mov     rcx, r14
			"E8 ", // call    HitDamageInfo__Copy
			{},
			{"esv::StatusHit::Setup", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(esv__StatusHit__Setup)}
		},

		{
			"esv::StatusHeal::__vftable",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"45 33 C9 " // xor     r9d, r9d
			"48 8D 15 XX XX XX XX " // lea     rdx, fsx_TargetDependentHeal
			"48 8B CB " // mov     rcx, rbx
			"FF 90 B0 01 00 00 ", // call    qword ptr [rax+1B0h]
			{SymbolMappingCondition::kFixedString, 3, "TargetDependentHeal"},
			{"esv::StatusHeal::__vftable", SymbolMappingTarget::kAbsolute, -0x200, nullptr, nullptr, &sSymbolStatusHeal, 0xa00}
		},

		{
			"esv::ParseItem",
			SymbolMappingData::kText, 0,
			"45 0F B6 C1 " // movzx   r8d, r9b
			"48 8B CF " // mov     rcx, rdi
			"E8 XX XX XX XX " // call    esv__ParseItem
			"33 D2 " // xor     edx, edx
			"48 8D 4C 24 38 " // lea     rcx, [rsp+78h+var_40]
			"E8 XX XX XX XX ", // call    esv__CreateItemFromParsed
			{},
			{"esv::ParseItem", SymbolMappingTarget::kIndirect, 7, STATIC_SYM(ParseItem)},
			{"esv::CreateItemFromParsed", SymbolMappingTarget::kIndirect, 19, STATIC_SYM(CreateItemFromParsed)},
		},

		{
			"UICharacterSheetHook",
			SymbolMappingData::kText, 0,
			"48 89 5D 60 " // mov     [rbp+0D0h+var_70], rbx
			"0F B6 88 B0 00 00 00 " // movzx   ecx, byte ptr [rax+0B0h]
			"41 3A CF " // cmp     cl, r15b
			"F2 0F 11 75 08 " // movsd   [rbp+0D0h+var_C8], xmm6
			"48 8B 4E 18 " // mov     rcx, [rsi+18h]
			"C7 45 B0 03 00 00 00 " // mov     [rbp+0D0h+var_120], 3
			// Replacement: c6 45 f8 01             mov    BYTE PTR [rbp-0x8],0x1
			"0F 94 45 F8 ", // setz    [rbp+0D0h+var_D8]
			{},
			{"UICharacterSheetHook", SymbolMappingTarget::kAbsolute, 30, STATIC_SYM(UICharacterSheetHook)},
		},

		{
			"ActivateEntitySystemHook",
			SymbolMappingData::kText, 0,
			"48 03 93 D0 00 00 00 " // add     rdx, [rbx+0D0h]
			"E8 XX XX XX XX " // call    ecs__EntityWorld__ActivateSystemInternal
			"48 8B 05 XX XX XX XX " // mov     rax, cs:ls__GlobalSwitches
			"80 B8 F3 0B 00 00 01 " // cmp     byte ptr [rax+0BF3h], 1
			// Replacement: 90 90
			"75 XX " // jnz     short loc_1415A124C
			"48 6B 15 XX XX XX XX 38 " // imul    rdx, cs:qword_1428F14F0, 38h
			"48 8B CB ", // mov     rcx, rbx
			{},
			{"ActivateEntitySystemHook", SymbolMappingTarget::kAbsolute, 26, nullptr, &FindActivateEntitySystemEoCApp},
		},

		{
			"CustomStatUIRollHook",
			SymbolMappingData::kText, 0,
			// Replacement: C3 (retn)
			"4C 8B DC " // mov     r11, rsp
			"48 81 EC 88 00 00 00 " // sub     rsp, 88h
			"48 8B 05 XX XX XX XX " // mov     rax, cs:__security_cookie
			"48 33 C4 " // xor     rax, rsp
			"48 89 44 24 70 " // mov     [rsp+88h+var_18], rax
			"48 8B 05 XX XX XX XX " // mov     rax, cs:?s_Ptr@PlayerManager@ls@@1PEAV12@EA
			"49 8D 53 A8 " // lea     rdx, [r11-58h]
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:qword_142940598
			"49 89 43 98 " // mov     [r11-68h], rax
			"48 8D 05 ", // lea     rax, ecl__CustomStatsProtocolSomething__vftable
			{},
			{"CustomStatUIRollHook", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(CustomStatUIRollHook)},
		},

		{
			"esv::CustomStatsProtocol::ProcessMsg",
			SymbolMappingData::kText, 0,
			// Replacement: C3 (retn)
			"4C 89 4C 24 20 " // mov     [rsp+arg_18], r9
			"53 " // push    rbx
			"57 " // push    rdi
			"41 56 " // push    r14
			"48 83 EC 30 " // sub     rsp, 30h
			"41 8B 51 08 " // mov     edx, [r9+8]
			"49 8B F9 " // mov     rdi, r9
			"4D 8B F0 " // mov     r14, r8
			"81 FA 3B 01 00 00 ", // cmp     edx, 13Bh
			{},
			{"esv::CustomStatsProtocol::ProcessMsg", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(EsvCustomStatsProtocolProcessMsg)},
		},

		{
			"NetworkFixedStrings",
			SymbolMappingData::kText, 0,
			"4C 8D 05 XX XX XX XX " // lea     r8, aNetworkfixedst ; "NetworkFixedStrings"
			"BA 01 00 00 00 " // mov     edx, 1
			"48 8B CF " // mov     rcx, rdi
			"FF 90 80 00 00 00 " // call    qword ptr [rax+80h]
			"48 8B 15 XX XX XX XX " // mov     rdx, cs:qword_14297F520
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:eoc__gNetworkFixedStrings
			"48 83 C2 28 " // add     rdx, 28h
			"E8 XX XX XX ", // call    eoc__NetworkFixedStrings__RegisterAll
			{SymbolMappingCondition::kString, 0, "NetworkFixedStrings"},
			{"NetworkFixedStrings", SymbolMappingTarget::kIndirect, 28, STATIC_SYM(NetworkFixedStrings)},
			{"InitNetworkFixedStrings", SymbolMappingTarget::kIndirect, 39, STATIC_SYM(InitNetworkFixedStrings)}
		},

		{
			"ErrorFuncs",
			SymbolMappingData::kText, 0,
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:ecl__gEocClient
			"4C 8D 8D 08 04 00 00 " // lea     r9, [rbp+8F8h+var_4F0]
			"41 B0 01 " // mov     r8b, 1
			"48 8D 95 E8 03 00 00 " // lea     rdx, [rbp+8F8h+var_510]
			"E8 XX XX XX XX " // call    ecl__EocClient__HandleError
			"48 8D 8D E8 03 00 00 ", // lea     rdx, [rbp+8F8h+var_510]
			{},
			{"ecl::EoCClient", SymbolMappingTarget::kIndirect, 0, STATIC_SYM(EoCClient)},
			{"ecl::EoCClient::HandleError", SymbolMappingTarget::kIndirect, 24, STATIC_SYM(EoCClientHandleError)}
		},

		{
			"eoc::SkillPrototypeManager::Init",
			SymbolMappingData::kText, 0,
			"4C 8D 05 XX XX XX XX " // lea     r8, str_Skills     ; "Skills"
			"BA 01 00 00 00 " // mov     edx, 1
			"48 8B CF " // mov     rcx, rdi
			"FF 90 80 00 00 00 " // call    qword ptr [rax+80h]
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:eoc__gSkillPrototypeManager
			"E8 XX XX XX XX ", // call    eoc__SkillPrototypeManager__Init
			{SymbolMappingCondition::kString, 0, "Skills"},
			{"eoc::SkillPrototypeManager::Init", SymbolMappingTarget::kIndirect, 28, STATIC_SYM(SkillPrototypeManagerInit)},
			{"eoc::SkillPrototypeManager", SymbolMappingTarget::kIndirect, 21, STATIC_SYM(eoc__SkillPrototypeManager)}
		},

		{
			"RPGStats::Load",
			SymbolMappingData::kText, 0,
			"48 8D 55 80 " // lea     rdx, [rbp+380h+modStatPaths]
			"48 8B CB " // mov     rcx, rbx
			"E8 XX XX XX XX " // call    CRPGStats__Load
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:qword_1429E2700
			"48 8B 01 " // mov     rax, [rcx]
			"FF 50 08 ", // call    qword ptr [rax+8]
			{},
			{"RPGStats::Load", SymbolMappingTarget::kIndirect, 7, STATIC_SYM(RPGStats__Load)}
		},

		{
			"eoc::StatusPrototypeManager::Init",
			SymbolMappingData::kText, 0,
			"4C 8D 05 XX XX XX XX " // lea     r8, aStatuses   ; "Statuses"
			"BA 01 00 00 00 " // mov     edx, 1
			"48 8B CF " // mov     rcx, rdi
			"FF 90 80 00 00 00 " // call    qword ptr [rax+80h]
			"48 8B 0D XX XX XX XX ", // mov     rcx, cs:eoc__gStatusPrototypeManager
			{SymbolMappingCondition::kString, 0, "Statuses"},
			{"eoc::StatusPrototypeManager", SymbolMappingTarget::kIndirect, 21, STATIC_SYM(eoc__StatusPrototypeManager)}
		},

		{
			"ecl::GameStateEventManager::ExecuteGameStateChangedEvent",
			SymbolMappingData::kText, 0,
			"4C 8D 0D XX XX XX XX " // lea r9, aClientStateSwa ; "CLIENT STATE SWAP - from: %s, to: %s\n"
			"4C 89 44 24 28 " // mov     [rsp+68h+var_40], r8
			"33 D2 " // xor     edx, edx
			"4C 8D 44 24 30 ", // lea     r8, [rsp+68h+var_38]
			{SymbolMappingCondition::kString, 0, "CLIENT STATE SWAP - from: %s, to: %s\n"},
			{"ecl::GameStateEventManager::ExecuteGameStateChangedEvent", SymbolMappingTarget::kIndirect, 0x2A, STATIC_SYM(ecl__GameStateEventManager__ExecuteGameStateChangedEvent)}
		},

		{
			"esv::GameStateEventManager::ExecuteGameStateChangedEvent",
			SymbolMappingData::kText, 0,
			"4C 8D 0D XX XX XX XX " // lea     r9, aServerStateSwa ; "SERVER STATE SWAP - from: %s, to: %s\n"
			"4C 89 44 24 28 " // mov     [rsp+68h+var_40], r8
			"33 D2 " // xor     edx, edx
			"4C 8D 44 24 30 ", // lea     r8, [rsp+68h+var_38]
			{SymbolMappingCondition::kString, 0, "SERVER STATE SWAP - from: %s, to: %s\n"},
			{"esv::GameStateEventManager::ExecuteGameStateChangedEvent", SymbolMappingTarget::kIndirect, 0x2A, STATIC_SYM(esv__GameStateEventManager__ExecuteGameStateChangedEvent)}
		},

		{
			"esv::GameStateThreaded::GameStateWorker::DoWork",
			SymbolMappingData::kText, 0,
			"48 8D 2D XX XX XX XX ", // lea     rbp, aEsvGamestateth ; "esv::GameStateThreaded::GameStateWorker::DoWork"
			{SymbolMappingCondition::kString, 0, "esv::GameStateThreaded::GameStateWorker::DoWork"},
			{"esv::GameStateThreaded::GameStateWorker::DoWork", SymbolMappingTarget::kAbsolute, -0x35, STATIC_SYM(esv__GameStateThreaded__GameStateWorker__DoWork)}
		},

		{
			"ecl::GameStateThreaded::GameStateWorker::DoWork",
			SymbolMappingData::kText, 0,
			"48 8D 2D XX XX XX XX ", // lea     rbp, aEclGamestateth ; "ecl::GameStateThreaded::GameStateWorker::DoWork"
			{SymbolMappingCondition::kString, 0, "ecl::GameStateThreaded::GameStateWorker::DoWork"},
			{"ecl::GameStateThreaded::GameStateWorker::DoWork", SymbolMappingTarget::kAbsolute, -0x26, STATIC_SYM(ecl__GameStateThreaded__GameStateWorker__DoWork)}
		},

		{
			"net::Host::AddProtocol",
			SymbolMappingData::kText, 0,
			"4C 8B C0 " // mov     r8, rax
			"BA 25 00 00 00 " // mov     edx, 25h ; '%'
			"48 8B CF " // mov     rcx, rdi
			"E8 XX XX XX XX " // call    net__Host__AddProtocol
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:xxx
			"4C 8D 4D 10 ", // lea     r9, [rbp+arg_0]
			{},
			{"net::Host::AddProtocol", SymbolMappingTarget::kIndirect, 11, STATIC_SYM(net__Host__AddProtocol)}
		},

		{
			"net::MessageFactory::RegisterMessage",
			SymbolMappingData::kText, 0,
			"48 8D 0D XX XX XX XX " // lea     rcx, aEocnetLevelloa ; "eocnet::LevelLoadMessage"
			"41 B9 04 00 00 00 " // mov     r9d, 4
			"48 89 4C 24 20 " // mov     [rsp+30h+var_10], rcx
			"4C 8B C0 " // mov     r8, rax
			"48 8B CF " // mov     rcx, rdi
			"BA FE 00 00 00 " // mov     edx, 0FEh
			"E8 XX XX XX XX ", // call    net__MessageFactory__RegisterMessage
			{SymbolMappingCondition::kString, 0, "eocnet::LevelLoadMessage"},
			{"net::MessageFactory::RegisterMessage", SymbolMappingTarget::kIndirect, 29, STATIC_SYM(net__MessageFactory__RegisterMessage)}
		},

		{
			"net::MessageFactory::GetFreeMessage",
			SymbolMappingData::kText, 0,
			"84 C0 " // test    al, al
			"0F 84 XX XX 00 00 " // jz      xxx
			"48 8B 05 XX XX XX XX " // mov     rax, cs:esv__gEoCServer
			"BA 2D 00 00 00 " // mov     edx, 2Dh
			"48 8B B0 80 00 00 00 " // mov     rsi, [rax+80h]
			"48 8B 8E E8 01 00 00 " // mov     rcx, [rsi+1E8h]
			"E8 XX XX XX XX ", // call    net__MessageFactory__GetFreeMessage
			{},
			{"net::MessageFactory::GetFreeMessage", SymbolMappingTarget::kIndirect, 34, STATIC_SYM(net__MessageFactory__GetFreeMessage)}
		},

		{
			"CharacterStatsGetters",
			SymbolMappingData::kText, 0,
			"40 84 ED " // test    bpl, bpl
			"74 08 " // jz      short loc_141C7A428
			"8B 86 44 03 00 00 " // mov     eax, [rsi+344h]
			"EB 06 " // jmp     short loc_141C7A42E
			"8B 86 40 03 00 00 " // mov     eax, [rsi+340h]
			"40 0F B6 D5 " // movzx   edx, bpl
			"89 43 44 ", // mov     [rbx+44h], eax
			{},
			{"CharacterStatsGetters", SymbolMappingTarget::kAbsolute, 0, nullptr, &FindCharacterStatGettersEoCApp}
		},

		{
			"GetHitChance",
			SymbolMappingData::kText, 0,
			"40 56 " // push    rsi
			"57 " // push    rdi
			"48 83 EC 28 " // sub     rsp, 28h
			"45 33 C0 " // xor     r8d, r8d
			"48 8B F2 " // mov     rsi, rdx
			"48 8B F9 " // mov     rdi, rcx
			"41 8D 50 78 " // lea     edx, [r8+78h]
			"E8 XX XX XX XX ", // call    CDivinityStats_Character__HasTalent
			{},
			{"GetHitChance", SymbolMappingTarget::kAbsolute, 0, CHAR_GETTER_SYM(GetHitChance), nullptr, &sSymbolChanceToHitBoost, 0x200}
		},

		{
			"esv::Character::Hit",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"74 74 " // jz      short loc_141A3EED4
			"48 8B 00 " // mov     rax, [rax]
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_DamageItems
			"48 8B CE " // mov     rcx, rsi
			"FF 50 28 ", // call    qword ptr [rax+28h]
			{SymbolMappingCondition::kFixedString, 5, "DamageItems"},
			{"esv::Character::Hit", SymbolMappingTarget::kAbsolute, 0, nullptr, nullptr, &sSymbolCharacterHit, 0x400}
		},

		{
			"FindLibrariesEoCApp",
			SymbolMappingData::kText, SymbolMappingData::kAllowFail,
			"8B C8 " // mov ecx, eax
			"FF C0 " // inc eax
			"89 05 XX XX XX XX " // mov cs:xxx, eax
			"85 C9 " // test ecx, ecx
			"75 17 " // jnz short loc_xxx
			"44 8D 41 XX " // lea r8d, [rdx+XXh]
			"48 8D 0D XX XX XX XX " // lea rcx, xxx
			"48 8D 15 XX XX XX XX ", // lea rdx, xxx
			{},
			{"FindLibrariesEoCApp", SymbolMappingTarget::kAbsolute, 0, nullptr, &FindLibrariesEoCApp}
		},

		{
			"esv::ActionMachine::UpdateSyncState",
			SymbolMappingData::kText, 0,
			"C6 44 24 28 01 " // mov     [rsp+58h+var_30], 1
			"45 33 C9 " // xor     r9d, r9d
			"45 33 C0 " // xor     r8d, r8d
			"C6 44 24 20 00 " // mov     [rsp+58h+var_38], 0
			"41 0F B6 D2 " // movzx   edx, r10b
			"41 C6 44 3B 53 00 " // mov     byte ptr [r11+rdi+53h], 0
			"48 8B CF " // mov     rcx, rdi
			"E8 XX XX XX XX ", // call    xxx
			{},
			{"esv::ActionMachine::UpdateSyncState", SymbolMappingTarget::kIndirect, 29, nullptr, &FindActionMachineSetState}
		},

		{
			"esv::ActionMachine::ResetState",
			SymbolMappingData::kText, 0,
			"48 83 EC 30 " // sub     rsp, 30h
			"40 32 F6 " // xor     sil, sil
			"4C 89 74 24 50 " // mov     [rsp+48h+arg_0], r14
			"45 33 E4 " // xor     r12d, r12d
			"48 8D 79 53 " // lea     rdi, [rcx+53h]
			"0F B6 EA " // movzx   ebp, dl
			"48 8D 59 18 " // lea     rbx, [rcx+18h]
			"4C 8B F9 ", // mov     r15, rcx
			{},
			{"esv::ActionMachine::ResetState", SymbolMappingTarget::kAbsolute, -20, nullptr, &FindActionMachineResetState}
		},

		{
			"eoc::SkillPrototype::FormatDescriptionParam",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"48 83 CF FF " // or      rdi, 0FFFFFFFFFFFFFFFFh
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_Damage_Multiplier
			"44 8B C7 " // mov     r8d, edi
			"48 8B CB " // mov     rcx, rbx
			"E8 XX XX XX XX " // call    sub_141C979B0
			"8B C8 ", // mov     ecx, eax
			{SymbolMappingCondition::kFixedString, 4, "Damage Multiplier"},
			{"eoc::SkillPrototype::FormatDescriptionParam", SymbolMappingTarget::kAbsolute, -0x200, nullptr, nullptr, &sSymbolSkillPrototypeFormatDescriptionParam, 0x200}
		},

		{
			"eoc::SkillPrototype::GetSkillDamage",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"0F 84 4D 06 00 00 " // jz      loc_xxx
			"4C 8B 00 " // mov     r8, [rax]
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_Damage_Multiplier
			"48 8B C8 ", // mov     rcx, rax
			{SymbolMappingCondition::kFixedString, 9, "Damage Multiplier"},
			{"eoc::SkillPrototype::GetSkillDamage", SymbolMappingTarget::kAbsolute, -0x200, nullptr, nullptr, &sSymbolSkillPrototypeGetSkillDamage, 0x200}
		},

		{
			"eoc::StatusPrototype::FormatDescriptionParam",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"49 89 7B C8 " // mov     [r11-38h], rdi
			"0F 85 B5 00 00 00 " // jnz     loc_xxx
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_DamageStats
			"49 8B CE ", // mov     rcx, r14
			{SymbolMappingCondition::kFixedString, 10, "DamageStats"},
			{"eoc::StatusPrototype::FormatDescriptionParam", SymbolMappingTarget::kAbsolute, -0x100, nullptr, nullptr, &sSymbolStatusPrototypeFormatDescriptionParam, 0x100}
		},

		{
			"esv::EoCServer",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_CurrentCellCount
			"48 8B CB " // mov     rcx, rbx
			"FF 90 90 01 00 00 " // call    qword ptr [rax+190h]
			"48 8B 03 " // mov     rax, [rbx]
			"48 8B CB " // mov     rcx, rbx
			"48 8B 3D XX XX XX XX ", //  mov     rdi, cs:esv__gEoCServer
			{SymbolMappingCondition::kFixedString, 0, "CurrentCellCount"},
			{"esv::EoCServer", SymbolMappingTarget::kIndirect, 22, STATIC_SYM(EoCServer)}
		},

		{
			"esv::TurnManager::UpdateTurnOrder",
			SymbolMappingData::kText, 0,
			"84 D2 " // test    dl, dl
			"0F 84 0E 05 00 00 " // jz      xxx
			"88 54 24 10 " // mov     [rsp+arg_8], dl
			"41 57 " // push    r15
			"48 83 EC 60 " // sub     rsp, 60h
			"44 8B 81 C4 00 00 00 ", // mov     r8d, [rcx+0C4h]
			{},
			{"esv::TurnManager::UpdateTurnOrder", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(esv__TurnManager__UpdateTurnOrder)}
		},

		{
			"GlobalSwitches",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"48 83 EC 20 " // sub     rsp, 20h
			"48 8B 05 XX XX XX XX " // mov     rax, cs:gGlobalSwitches
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_UseLevelCache
			"48 8B F9 ", // mov     rdi, rcx
			{SymbolMappingCondition::kFixedString, 11, "UseLevelCache"},
			{"GlobalSwitches", SymbolMappingTarget::kIndirect, 4, STATIC_SYM(pGlobalSwitches)}
		},

		{
			"CDivinityStats_Item::FoldDynamicAttributes",
			SymbolMappingData::kText, 0,
			"4C 8B DC " // mov     r11, rsp
			"49 89 4B 08 " // mov     [r11+8], rcx
			"55 " // push    rbp
			"41 55 " // push    r13
			"49 8D 6B A1 " // lea     rbp, [r11-5Fh]
			"48 81 EC B8 00 00 00 " // sub     rsp, 0B8h
			"48 8B 05 XX XX XX XX " // mov     rax, cs:gGlobalSwitches
			"4C 8B E9 ", // mov     r13, rcx
			{},
			{"CDivinityStats_Item::FoldDynamicAttributes", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(ItemFoldDynamicAttributes)}
		},

		{
			"ls::ModuleSettings::HasCustomMods",
			SymbolMappingData::kText, SymbolMappingData::kAllowFail,
			"48 89 6C 24 20 " // mov     [rsp+arg_18], rbp
			"56 " // push    rsi
			"8B 69 1C " // mov     ebp, [rcx+1Ch]
			"33 F6 " // xor     esi, esi
			"48 85 ED " // test    rbp, rbp
			"0F 84 XX 00 00 00 " // jz      xxx
			"48 8B 05 XX XX XX XX ", // mov     rax, cs:fs_xxx
			{},
			{"ls::ModuleSettings::HasCustomMods", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(ModuleSettingsHasCustomMods)}
		},

		{
			"ls::ModuleSettings::HasCustomModsGB5",
			SymbolMappingData::kText, SymbolMappingData::kAllowFail,
			"48 8B 05 XX XX XX XX " // mov     rax, cs:xxx
			"33 ED " // xor     ebp, ebp
			"48 8B 88 A8 00 00 00 " // mov     rcx, [rax+0A8h]
			"44 8B B9 A4 03 00 00 " // mov     r15d, [rcx+3A4h]
			"4D 85 FF ", // test    r15, r15
			{},
			{"ls::ModuleSettings::HasCustomModsGB5", SymbolMappingTarget::kAbsolute, -0x18, STATIC_SYM(ModuleSettingsHasCustomModsGB5)}, {}, {},
			{SymbolVersion::AboveOrEqual, 64} // After GB5
		},

		{
			"ls::ModuleSettings::Validate",
			SymbolMappingData::kText, 0,
			"41 54 " // push    r12
			"41 55 " // push    r13
			"41 57 " // push    r15
			"48 83 EC 40 " // sub     rsp, 40h
			"44 8B 79 1C " // mov     r15d, [rcx+1Ch]
			"4D 8B E8 " // mov     r13, r8
			"8B 42 1C " // mov     eax, [rdx+1Ch]
			"4C 8B E2 ", // mov     r12, rdx
			{},
			{"ls::ModuleSettings::Validate", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(ModuleSettings__Validate)}
		},

		{
			"ls::Module::Hash",
			SymbolMappingData::kText, 0,
			"48 89 5C 24 20 " // mov     [rsp+arg_18], rbx
			"57 " // push    rdi
			"48 81 EC C0 00 00 00 " // sub     rsp, 0C0h
			"48 8B 05 XX XX XX XX " // mov     rax, cs:__security_cookie
			"48 33 C4 " // xor     rax, rsp
			"48 89 84 24 B0 00 00 00 " // mov     [rsp+0C8h+var_18], rax
			"48 8B 05 XX XX XX XX " // mov     rax, cs:gEngine
			"33 FF ", // xor     edi, edi
			{},
			{"ls::Module::Hash", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(Module__Hash)}
		},

		{
			"esv::LoadProtocol::HandleModuleLoaded",
			SymbolMappingData::kText, 0,
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:xxx
			"4C 8D 3D XX XX XX XX " // lea     r15, "esv::LoadProtocol::HandleModuleLoaded"
			"49 8B D8 ", // mov     rbx, r8
			{SymbolMappingCondition::kString, 7, "esv::LoadProtocol::HandleModuleLoaded"},
			{"esv::LoadProtocol::HandleModuleLoaded", SymbolMappingTarget::kAbsolute, -0x24, STATIC_SYM(esv__LoadProtocol__HandleModuleLoaded)}
		},

		{
			"esv::Inventory::Equip",
			SymbolMappingData::kText, 0,
			"48 89 54 24 10 " // mov     [rsp-8+arg_8], rdx
			"55 56 57 41 54 41 56 " // push    rbp, rsi, rdi, r12, r14
			"48 8D 6C 24 E9 " // lea     rbp, [rsp-17h]
			"48 81 EC D0 00 00 00 ", // sub     rsp, 0D0h
			{},
			{"esv::Inventory::Equip", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(InventoryEquip)}
		},

		{
			"ls::UIObjectManager",
			SymbolMappingData::kText, 0,
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:qword_1429824B0
			"4D 8B C6 " // mov     r8, r14
			"48 81 C1 D8 00 00 00 " // add     rcx, 0D8h
			"BA 92 00 00 00 " // mov     edx, 92h
			"E8 XX XX XX XX ", // call    ls__UIObjectManager__RegisterUIObjectCreator
			{},
			{"ls::UIObjectManager", SymbolMappingTarget::kIndirect, 0, STATIC_SYM(UIObjectManager__Instance)},
			{"ls::UIObjectManager::RegisterUIObjectCreator", SymbolMappingTarget::kIndirect, 22, STATIC_SYM(UIObjectManager__RegisterUIObjectCreator)}
		},

		{
			"ls::UIObjectManager::CreateUIObject",
			SymbolMappingData::kText, 0,
			"89 44 24 20 " // mov     [rsp+40h+var_20], eax
			"45 8D 41 94 " // lea     r8d, [r9-6Ch]
			"E8 XX XX XX XX " // call    ls__UIObjectManager__CreateUIObject
			"48 8B 08 " // mov     rcx, [rax]
			"48 89 8B 10 02 00 00 ", // mov     [rbx+210h], rcx
			{},
			{"ls::UIObjectManager::CreateUIObject", SymbolMappingTarget::kIndirect, 8, STATIC_SYM(UIObjectManager__CreateUIObject)}
		},

		{
			"ls::UIObjectManager::DestroyUIObject",
			SymbolMappingData::kText, 0,
			"48 8D 93 A8 02 00 00 " // lea     rdx, [rbx+2A8h]
			"48 8B CF " // mov     rcx, rdi
			"E8 XX XX XX XX " // call    ls__UIObjectManager__DestroyUIObject
			"48 8B 05 XX XX XX XX " // mov     rax, cs:ls__ObjectHandle__Unassigned
			"48 89 83 A8 02 00 00 " // mov     [rbx+2A8h], rax
			"45 84 E4 ", // test    r12b, r12b
			{},
			{"ls::UIObjectManager::DestroyUIObject", SymbolMappingTarget::kIndirect, 10, STATIC_SYM(UIObjectManager__DestroyUIObject)}
		},

		{
			"eoc::EoCUI::ctor",
			SymbolMappingData::kText, 0,
			"48 8B D9 " // mov     rbx, rcx
			"E8 XX XX XX XX " // call    ecl__EocUI__ctor
			"C7 83 98 01 00 00 00 00 A0 40 " // mov     dword ptr [rbx+198h], 40A00000h
			"48 8D 8B B0 01 00 00 " // lea     rcx, [rbx+1B0h]
			"C7 83 9C 01 00 00 FF FF FF FF ", // mov     dword ptr [rbx+19Ch], 0FFFFFFFFh
			{},
			{"eoc::EoCUI::ctor", SymbolMappingTarget::kIndirect, 3, STATIC_SYM(EoCUI__ctor), nullptr, &sSymbolEoCUIvftable, 0x40}
		},

		{
			"eoc::gSpeakerManager",
			SymbolMappingData::kText, 0,
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:eoc__gSpeakerManager
			"E8 XX XX XX XX " // call    eoc__SpeakerManager__LoadVoiceMetaData
			"48 8B 07 " // mov     rax, [rdi]
			"4C 8D 05 XX XX XX XX ", // lea     r8, aRootTemplates ; "Root templates"
			{SymbolMappingCondition::kString, 15, "Root templates"},
			{"eoc::gSpeakerManager", SymbolMappingTarget::kIndirect, 0, STATIC_SYM(eoc__SpeakerManager)}
		},

		{
			"ls::TranslatedStringRepository::UnloadOverrides",
			SymbolMappingData::kText, 0,
			"4C 8D 05 XX XX XX XX " // lea     r8, aLoadingModLoca ; "Loading Mod Localization"
			"BA 01 00 00 00 " // mov     edx, 1
			"48 8B CF " // mov     rcx, rdi
			"FF 90 80 00 00 00 " // call    qword ptr [rax+80h]
			"4C 8B 25 XX XX XX XX " // mov     r12, cs:ls__gTranslatedStringRepository
			"49 8B CC " // mov     rcx, r12
			"E8 XX XX XX XX ", // call    ls__TranslatedStringRepository__UnloadOverrides
			{SymbolMappingCondition::kString, 0, "Loading Mod Localization"},
			{"ls::TranslatedStringRepository::UnloadOverrides", SymbolMappingTarget::kIndirect, 31, STATIC_SYM(TranslatedStringRepository__UnloadOverrides)},
		},

		{
			"ls::TranslatedStringRepository::Get",
			SymbolMappingData::kText, 0,
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:ls__gTranslatedStringRepository
			"4C 8D 44 XX XX " // lea     r8, [rsp+190h+a3]
			"C6 44 24 XX XX " // mov     [rsp+190h+a6], 1
			"48 8D 54 XX XX " // lea     rdx, [rsp+190h+a2]
			"45 33 C9 " // xor     r9d, r9d
			"88 5C XX XX " // mov     [rsp+190h+a5], bl
			"E8 XX XX XX XX " // call    ls__TranslatedStringRepository__Get
			"4C 8D 4C XX XX " // lea     r9, [rsp+190h+var_140]
			"4C 8B C0 " // mov     r8, rax
			"48 8D 15 XX XX XX XX ", // lea     rdx, aListcivilabili ; "listCivilAbilities"
			{SymbolMappingCondition::kString, 42, "listCivilAbilities"},
			{"ls::TranslatedStringRepository::Instance", SymbolMappingTarget::kIndirect, 0, STATIC_SYM(TranslatedStringRepository__Instance)},
			{"ls::TranslatedStringRepository::Get", SymbolMappingTarget::kIndirect, 29, STATIC_SYM(TranslatedStringRepository__Get)},
		},

		{
			"ls::TranslatedStringKeyManager::GetTranlatedStringFromKey",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"48 8D 15 XX XX XX XX " // lea     rdx, fs_DisplayName
			"48 8B CB " // mov     rcx, rbx
			"E8 XX XX XX XX " // call    sub_141C6E690
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:ls__gTranslatedStringKeyManager
			"48 8D 55 30 " // lea     rdx, [rbp+1C0h+var_190]
			"41 B1 01 " // mov     r9b, 1
			"4C 8B C0 " // mov     r8, rax
			"4C 8B F0 " // mov     r14, rax
			"E8 XX XX XX XX ", // call    ls__TranslatedStringKeyManager__GetTranlatedStringFromKey
			{SymbolMappingCondition::kFixedString, 0, "DisplayName"},
			{"ls::TranslatedStringKeyManager::Instance", SymbolMappingTarget::kIndirect, 15, STATIC_SYM(TranslatedStringKeyManager__Instance)},
			{"ls::TranslatedStringKeyManager::GetTranlatedStringFromKey", SymbolMappingTarget::kIndirect, 35, STATIC_SYM(TranslatedStringKeyManager__GetTranlatedStringFromKey)},
		},

		{
			"ls::ModuleManager::CollectAvailableMods",
			SymbolMappingData::kText, 0,
			"C7 81 7C 03 00 00 00 00 00 00 " // mov     dword ptr [rcx+37Ch], 0
			"48 81 C1 68 03 00 00 " // add     rcx, 368h
			"E8 XX XX XX XX " // call    ls__ModManager__CollectAvailableMods
			"48 8B 93 70 03 00 00 ", // mov     rdx, [rbx+370h]
			{},
			{"ls::ModuleManager::CollectAvailableMods", SymbolMappingTarget::kIndirect, 17, STATIC_SYM(ModManager__CollectAvailableMods)}
		},

		{
			"ls::ScriptCheckBlock::Build",
			SymbolMappingData::kText, 0,
			"4C 8B 4B 10 " // mov     r9, [rbx+10h]
			"48 8D 55 18 " // lea     rdx, [rbp+18h]
			"41 FF C9 " // dec     r9d
			"48 8D 4C 24 28 " // lea     rcx, [rsp+78h+Dst]
			"45 33 C0 " // xor     r8d, r8d
			"E8 XX XX XX XX ", // call    ls__ScriptCheckBlock__Build
			{},
			{"ls::ScriptCheckBlock::Build", SymbolMappingTarget::kIndirect, 19, STATIC_SYM(ScriptCheckBlock__Build)}
		},

		{
			"esv::OsirisVariableHelper::SavegameVisit",
			SymbolMappingData::kText, SymbolMappingData::kDeferred | SymbolMappingData::kCritical,
			"48 89 5C 24 18 " // mov     [rsp+arg_10], rbx
			"48 89 6C 24 20 " // mov     [rsp+arg_18], rbp
			"57 " // push    rdi
			"48 83 EC 20 " // sub     rsp, 20h
			"48 8B 02 " // mov     rax, [rdx]
			"48 8B DA " // mov     rbx, rdx
			"48 8B E9 " // mov     rbp, rcx
			"48 8D 15 XX XX XX XX ", // lea     rdx, fs_OsirisVariableHelper
			{SymbolMappingCondition::kFixedString, 24, "OsirisVariableHelper"},
			{"esv::OsirisVariableHelper::SavegameVisit", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(esv__OsirisVariableHelper__SavegameVisit)}
		},

		{
			"esv::Character::ApplyDamage < 64",
			SymbolMappingData::kText, 0,
			"C7 45 CB 06 00 00 00 " // mov     [rbp+57h+hitDamageInfo.AttackDirection_Enum], 6
			"44 89 75 07 " // mov     [rbp+57h+var_50], r14d
			"C7 45 0B 00 00 80 BF " // mov     [rbp+57h+var_4C], 0BF800000h
			"44 89 75 0F " // mov     [rbp+57h+var_48], r14d
			"E8 XX XX XX XX ", // call    esv__Character__ApplyDamage
			{},
			{"esv::Character::ApplyDamage", SymbolMappingTarget::kIndirect, 22, STATIC_SYM(esv__Character__ApplyDamage)}, {}, {},
			{SymbolVersion::Below, 64} // Before GB5
		},

		{
			"esv::Character::ApplyDamage >= 64",
			SymbolMappingData::kText, 0,
			"48 89 44 24 20 " // mov     [rsp+0D0h+var_B0], rax
			"C7 45 CB 06 00 00 00 " // mov     [rbp+57h+hitDamageInfo.AttackDirection_Enum], 6
			"89 5D 07 " // mov     [rbp+57h+var_50], r14d
			"C7 45 0B 00 00 80 BF " // mov     [rbp+57h+var_4C], 0BF800000h
			"89 5D 0F " // mov     [rbp+57h+var_48], r14d
			"E8 XX XX XX XX ", // call    esv__Character__ApplyDamage
			{},
			{"esv::Character::ApplyDamage", SymbolMappingTarget::kIndirect, 25, STATIC_SYM(esv__Character__ApplyDamage)}, {}, {},
			{SymbolVersion::AboveOrEqual, 64} // After GB5
		},

		{
			"ecl::InventoryFactory",
			SymbolMappingData::kText, 0,
			"48 8B 3D XX XX XX XX " // mov     rdi, cs:ecl__gInventoryFactory
			"48 8D 8F XX XX XX XX " // lea     rcx, [rdi+88h]
			"E8 XX XX XX XX " // call    xxx
			"48 8D 54 24 28 " // lea     rdx, [rsp+98h+var_70]
			"48 89 7C 24 28 " // mov     [rsp+98h+var_70], rdi
			"48 8D 8F A0 00 00 00 " // lea     rcx, [rdi+0A0h]
			"E8 XX XX XX XX " // call    sub_140379950
			"48 8D 8F A0 00 00 00 " // lea     rcx, [rdi+0A0h]
			"E8 XX XX XX XX " // call    sub_1403796E0
			"48 8B CF " // mov     rcx, rdi
			"E8 XX XX XX XX " // call    sub_140378F50
			"48 8B 06 " // mov     rax, [rsi]
			"4C 8D 05 XX XX XX XX ", // lea     r8, str_Triggers
			{SymbolMappingCondition::kString, 64, "Triggers"},
			{"ecl::InventoryFactory", SymbolMappingTarget::kIndirect, 0, STATIC_SYM(EclInventoryFactory)}
		},

		{
			"ecl::LevelManager",
			SymbolMappingData::kText, 0,
			"4C 8D 05 XX XX XX XX " // lea     r8, "NetworkFixedStrings"
			"BA 01 00 00 00 " // mov     edx, 1
			"48 8B CF " // mov     rcx, rdi
			"FF 90 80 00 00 00 " // call    qword ptr [rax+80h]
			"48 8B 15 XX XX XX XX " // mov     rdx, cs:ecl__gLevelManager
			"48 8B 0D XX XX XX XX ", // mov     rcx, cs:eoc__gNetworkFixedStrings
			{SymbolMappingCondition::kString, 0, "NetworkFixedStrings"},
			{"esv::LevelManager", SymbolMappingTarget::kIndirect, 21, STATIC_SYM(EclLevelManager)}
		},

		{
			"esv::ItemHelpers::GenerateTreasureItem",
			SymbolMappingData::kText, 0,
			"48 83 EC 30 " // sub     rsp, 30h
			"48 8B 1D XX XX XX XX " // mov     rbx, cs:gRPGStats
			"8B F2 " // mov     esi, edx
			"4C 8B F1 " // mov     r14, rcx
			"48 8D 51 08 " // lea     rdx, [rcx+8]
			"33 FF " // xor     edi, edi
			"33 ED " // xor     ebp, ebp
			"48 8B 83 90 00 00 00 ", // mov     rax, [rbx+90h]
			{},
			{"esv::ItemHelpers::GenerateTreasureItem", SymbolMappingTarget::kAbsolute, -0x14, STATIC_SYM(esv__ItemHelpers__GenerateTreasureItem)}
		},

		{
			"esv::CombineManager::ExecuteCombination",
			SymbolMappingData::kText, 0,
			"48 8D AC 24 18 FF FF FF " // lea     rbp, [rsp-0E8h]
			"48 81 EC E8 01 00 00 " // sub     rsp, 1E8h	
			"48 8B 05 XX XX XX XX " // mov     rax, cs:__security_cookie
			"48 33 C4 " // xor     rax, rsp
			"48 89 85 B0 00 00 00 " // mov     [rbp+100h+var_50], rax
			"4C 8B AD 40 01 00 00 ", // mov     r13, [rbp+100h+combinationId]
			{},
			{"esv::CombineManager::ExecuteCombination", SymbolMappingTarget::kAbsolute, -0x08, STATIC_SYM(esv__CombineManager__ExecuteCombination)}
		},

		{
			"eoc::SurfaceTransformActionsFromType",
			SymbolMappingData::kText, 0,
			"83 F8 4F " // cmp     eax, 4Fh
			"77 13 " // ja      short xxx
			"4C 69 E8 D0 02 00 00 " // imul    r13, rax, 2D0h
			"48 8D 05 XX XX XX XX ", // lea     rax, eoc__SurfaceTransformActionsFromType
			{},
			{"eoc::SurfaceTransformActionsFromType", SymbolMappingTarget::kIndirect, 12, STATIC_SYM(eoc__SurfaceTransformActionsFromType)}
		},

		{
			"eoc::SurfaceTemplates",
			SymbolMappingData::kText, 0,
			"48 63 81 90 01 00 00 " // movsxd  rax, dword ptr [rcx+190h]
			"85 C0 " // test    eax, eax
			"78 XX " // js      short XX
			"48 8D 0D XX XX XX XX " // lea     rcx, qword_14295B6E0
			"48 39 1C C1 ", // cmp     [rcx+rax*8], rbx
			{},
			{"eoc::SurfaceTemplates", SymbolMappingTarget::kIndirect, 11, STATIC_SYM(eoc__SurfaceTemplates)}
		},

		{
			"esv::SurfaceActionFactory",
			SymbolMappingData::kText, 0,
			"BA 01 00 00 00 " // mov     edx, 1
			"48 8B 0D XX XX XX XX " // mov     rcx, cs:esv__gSurfaceActionFactory
			"E8 XX XX XX XX " // call    esv__SurfaceActionFactory__CreateAction
			"48 8B F8 " // mov     rdi, rax
			"49 8B CF " // mov     rcx, r15
			"8B 45 18 ", // mov     eax, [rbp+18h]
			{},
			{"esv::SurfaceActionFactory", SymbolMappingTarget::kIndirect, 5, STATIC_SYM(esv__SurfaceActionFactory)},
			{"esv::SurfaceActionFactory::CreateAction", SymbolMappingTarget::kIndirect, 12, STATIC_SYM(esv__SurfaceActionFactory__CreateAction)},
		},

		{
			"esv::TransformSurfaceAction::Init",
			SymbolMappingData::kText, 0,
			"4C 8D 0D XX XX XX XX " // lea     r9, xxx
			"48 8D 0D XX XX XX XX " // lea     rcx, "TransformSurfaceAction Created! action: %u (%d:%s)"
			"4C 8B 10 ", // mov     r10, [rax]
			{SymbolMappingCondition::kString, 7, "TransformSurfaceAction Created! action: %u (%d:%s)\n"},
			{"esv::TransformSurfaceAction::Init", SymbolMappingTarget::kAbsolute, -0x2A, STATIC_SYM(esv__TransformSurfaceAction__Init)},
		},

		{
			"eoc::SkillPrototype::GetAttackAPCost",
			SymbolMappingData::kText, 0,
			"41 57 " // push    r15
			"48 83 EC 48 " // sub     rsp, 48h
			"8B 79 28 " // mov     edi, [rcx+28h]
			"4D 8B F9 ", // mov     r15, r9
			{},
			{"eoc::SkillPrototype::GetAttackAPCost", SymbolMappingTarget::kAbsolute, -0x0C, STATIC_SYM(SkillPrototype__GetAttackAPCost)}
		},

		{
			"eoc::AiGrid::SearchForCell",
			SymbolMappingData::kText, 0,
			"4C 8B DC " // mov     r11, rsp
			"41 56 " // push    r14
			"48 81 EC 90 00 00 00 " // sub     rsp, 90h
			"33 C0 " // xor     eax, eax
			"F3 0F 11 54 24 4C ", // movss   dword ptr [rsp+98h+var_50+4], xmm2
			{},
			{"eoc::AiGrid::SearchForCell", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(eoc__AiGrid__SearchForCell)}
		},

		{
			"esv::ExecutePropertyDataOnGroundHit",
			SymbolMappingData::kText, 0,
			"49 8D 6B C8 " // lea     rbp, [r11-38h]
			"48 81 EC 18 01 00 00 " // sub     rsp, 118h
			"48 8B 05 XX XX XX XX " // mov     rax, cs:esv__gLevelManager
			"4D 8B E8 " // mov     r13, r8
			"4C 8B F1 " // mov     r14, rcx
			"4C 8B A0 80 00 00 00 ", // mov     r12, [rax+80h]
			{},
			{"esv::ExecutePropertyDataOnGroundHit", SymbolMappingTarget::kAbsolute, -0x12, STATIC_SYM(esv__ExecutePropertyDataOnGroundHit)}
		},

		{
			"App::Instance",
			SymbolMappingData::kText, 0,
			"48 89 45 2F " // mov     [rbp+57h+var_28], rax
			"48 8B F1 " // mov     rsi, rcx
			"BA 02 00 00 00 " // mov     edx, 2
			"48 8B 0D XX XX XX XX " //  mov     rcx, cs:qword_142959A50
			"48 8B 01 " // mov     rax, [rcx]
			"FF 90 40 02 00 00 ", // call    qword ptr [rax+240h]
			{},
			{"App::Instance", SymbolMappingTarget::kIndirect, 12, STATIC_SYM(AppInstance)}
		},

		{
			"ls::ResourceManager::Instance",
			SymbolMappingData::kText, 0,
			"48 3B 9F C0 00 00 00 " // cmp     rbx, [rdi+0C0h]
			"7C XX " // jl      short xxx
			"48 8B 05 XX XX XX XX " // mov     rax, cs:xxx
			"45 33 C9 ", //  xor     r9d, r9d
			{},
			{"ls::ResourceManager::Instance", SymbolMappingTarget::kIndirect, 9, STATIC_SYM(ResourceManager__Instance)}
		},

		{
			"RPGStats::ParseProperties",
			SymbolMappingData::kText, 0,
			"41 57 " // push    r15
			"48 8D AC 24 20 FD FF FF " // lea     rbp, [rsp-2E0h]
			"48 81 EC E0 03 00 00 " // sub     rsp, 3E0h
			"48 8B 05 XX XX XX XX " // mov     rax, cs:__security_cookie
			"48 33 C4 " // xor     rax, rsp
			"48 89 85 D8 02 00 00 ", // mov     [rbp+310h+var_38], rax
			{},
			{"RPGStats::ParseProperties", SymbolMappingTarget::kAbsolute, -0x0E, STATIC_SYM(RPGStats__ParseProperties)}
		},

		{
			"esv::ExecutePropertyDataOnPositionOnly",
			SymbolMappingData::kText, 0,
			"48 8B C4 " // mov     rax, rsp
			"F3 0F 11 58 20 " // movss   dword ptr [rax+20h], xmm3
			"4C 89 40 18 " //  mov     [rax+18h], r8
			"48 89 50 10 ", // mov     [rax+10h], rdx
			{},
			{"esv::ExecutePropertyDataOnPositionOnly", SymbolMappingTarget::kAbsolute, 0, STATIC_SYM(esv__ExecutePropertyDataOnPositionOnly)}
		},

		{
			"esv::ExecuteCharacterExtraProperties",
			SymbolMappingData::kText, 0,
			"41 57 " // push    r15
			"48 8D AC 24 C8 FC FF FF " // lea     rbp, [rsp-338h]
			"48 81 EC 38 04 00 00 " // sub     rsp, 438h
			"48 8B 85 98 03 00 00 ", // mov     rax, [rbp+350h+damageInfo]
			{},
			{"esv::ExecuteCharacterExtraProperties", SymbolMappingTarget::kAbsolute, -0x18, STATIC_SYM(esv__ExecuteCharacterSetExtraProperties)}
		},

		{
			"ls::UIHelper::CustomDrawObject",
			SymbolMappingData::kText, 0,
			"5F " // pop     rdi
			"E9 XX XX XX XX " // jmp     ls__UIHelper__CustomDrawObject
			"41 B8 02 00 00 00 " // mov     r8d, 2
			"48 8D 15 XX XX XX XX " // lea     rdx, aTt_0
			"FF 15 XX XX XX XX ", // call    cs:wcsncmp
			{SymbolMappingCondition::kString, 12, "t"},
			{"ls::UIHelper::CustomDrawObject", SymbolMappingTarget::kIndirect, 1, STATIC_SYM(ls__UIHelper__CustomDrawObject)}
		},

		{
			"ls::UIHelper::UIClearIcon",
			SymbolMappingData::kText, 0,
			"48 8B CF " // mov     rcx, rdi
			"E8 XX XX XX XX " // call    ls__UIHelper__UIClearIcon
			"48 8D 4B 30 " // lea     rcx, [rbx+30h]
			"E8 XX XX XX XX " // call    ls__UIHelper__UIClearIcon
			"48 8D 35 XX XX XX XX " // lea     rsi, ls__CustomDrawStruct__VMT
			"48 8D 4B 30 " // lea     rcx, [rbx+30h]
			"48 89 73 30 ", // mov     [rbx+30h], rsi
			{},
			{"ls::UIHelper::UIClearIcon", SymbolMappingTarget::kIndirect, 3, STATIC_SYM(ls__UIHelper__UIClearIcon)},
			{"ls::CustomDrawStruct::VMT", SymbolMappingTarget::kIndirect, 17, STATIC_SYM(ls__CustomDrawStruct__VMT)}
		},

		{
			"ls::UIHelper::UICreateIconMesh",
			SymbolMappingData::kText, SymbolMappingData::kDeferred,
			"48 8D 0D XX XX XX XX " // lea     rcx, GEFS__strUIBaseMaterialUUID
			"48 89 4C 24 20 " // mov     [rsp+38h+var_18], rcx
			"49 8B CA " // mov     rcx, r10
			"44 8B C8 " // mov     r9d, eax
			"44 8B C0 " // mov     r8d, eax
			"E8 XX XX XX XX ", // call    ls__UIHelper__UICreateIconMesh
			{SymbolMappingCondition::kFixedString, 0, "9169b076-6e8d-44a4-bb52-95eedf9eab63"},
			{"ls::UIHelper::UICreateIconMesh", SymbolMappingTarget::kIndirect, 21, STATIC_SYM(ls__UIHelper__UICreateIconMesh)}
		},
	};

	bool LibraryManager::FindEoCApp(uint8_t const * & start, size_t & size)
	{
		HMODULE hEoCPlugin = GetModuleHandleW(L"EoCPlugin.dll");
		if (hEoCPlugin != NULL) {
			MessageBoxW(NULL, L"This version of the Script Extender can only be used ingame.", L"Osiris Extender Error", MB_OK | MB_ICONERROR);
			TerminateProcess(GetCurrentProcess(), 1);
		}

		HMODULE hEoCApp = GetModuleHandleW(L"EoCApp.exe");
		if (hEoCApp == NULL) {
			return false;
		}

		MODULEINFO moduleInfo;
		if (!GetModuleInformation(GetCurrentProcess(), hEoCApp, &moduleInfo, sizeof(moduleInfo))) {
			Fail(L"Could not get module info of EoCApp.exe");
		}

		start = (uint8_t const *)moduleInfo.lpBaseOfDll;
		size = moduleInfo.SizeOfImage;
		return true;
	}

	void LibraryManager::MapAllSymbols(bool deferred)
	{
		for (auto i = 0; i < std::size(sSymbolMappings); i++) {
			if ((deferred && (sSymbolMappings[i].Flag & SymbolMappingData::kDeferred))
				|| (!deferred && !(sSymbolMappings[i].Flag & SymbolMappingData::kDeferred))) {
				MapSymbol(sSymbolMappings[i], nullptr, 0);
			}
		}
	}

	void LibraryManager::FindServerGlobalsEoCApp()
	{
		StaticSymbols::EoCLibraryInfo const * serverLib{ nullptr };
		auto & sym = GetStaticSymbols();
		for (auto const & v : sym.Libraries) {
			if (v.second.refs > 100) {
				serverLib = &v.second;
				break;
			}
		}

		if (serverLib == nullptr) {
			Fail("LibraryManager::FindServerGlobalsEoCApp(): Could not find esv library!");
			return;
		}

		for (auto p = serverLib->initFunc; p < serverLib->initFunc + 0x1000; p++) {
			if (p[0] == 0x48 && p[1] == 0x89 && p[2] == 0x05
				&& p[7] == 0xEB && p[8] == 0x07
				&& p[9] == 0x48 && p[10] == 0x89
				&& p[16] == 0xE8
				&& p[21] == 0xE8
				&& p[26] == 0xE8
				&& p[31] == 0xE8
				&& p[36] == 0xE8) {

				uint8_t const * funcPtr = p + 16;
				for (auto i = 0; i < 36; i++) {
					int32_t rel = *(int32_t *)(funcPtr + 1);
					uint8_t const * registrantFunc = funcPtr + rel + 5;
					sym.ServerRegisterFuncs[i] = registrantFunc;
					funcPtr += 5;
				}

				funcPtr = p + 16 + (36 * 5) + 0x2C;
				for (auto i = 0; i < 14; i++) {
					int32_t rel = *(int32_t *)(funcPtr + 1);
					uint8_t const * registrantFunc = funcPtr + rel + 5;
					sym.ServerRegisterFuncs[i + 36] = registrantFunc;
					funcPtr += 5;
				}

				for (auto i = 0; i < std::size(GetStaticSymbols().ServerRegisterFuncs); i++) {
					uint8_t const * r = GetStaticSymbols().ServerRegisterFuncs[i];
					if (r[0] == 0x48 && r[1] == 0x83 && r[2] == 0xEC /* sub rsp, XXh */
						&& r[4] == 0x48 && r[5] == 0x8B && r[6] == 0x05 /* mov rax, cs:xxx */) {

						int32_t rel = *(int32_t *)(r + 7);
						uint8_t const * registrantObj = r + rel + 4 + 7;
						sym.ServerGlobals[i] = (uint8_t const **)registrantObj;

					}
					else {
						sym.ServerGlobals[i] = nullptr;
					}
				}
			}
		}

		sym.EsvInventoryFactory = (esv::InventoryFactory **)sym.ServerGlobals[(unsigned)EsvGlobalEoCApp::EsvInventoryFactory];
		sym.EsvSurfaceActionFactory = (esv::SurfaceActionFactory**)sym.ServerGlobals[(unsigned)EsvGlobalEoCApp::SurfaceActionFactory];

		if (sym.EsvInventoryFactory == nullptr) {
			CriticalInitFailed = true;
		}
	}

	void LibraryManager::FindEoCGlobalsEoCApp()
	{
		static uint8_t const libInitSig[] = {
			0x48, 0x89, 0x18,       // mov     [rax], rbx
			0x48, 0x89, 0x58, 0x08, // mov     [rax+8], rbx
			0x48, 0x89, 0x58, 0x10, // mov     [rax+10h], rbx
			0x48, 0x8D, 0x40, 0x40, // lea     rax, [rax+40h]
			0x48, 0x89, 0x58, 0xD8, // mov     [rax-28h], rbx
			0x48, 0x89, 0x58, 0xE0  // mov     [rax-20h], rbx
		};

		uint8_t const * globalsInitCode{ nullptr };
		for (auto const & lib : GetStaticSymbols().Libraries) {
			for (auto p = lib.second.initFunc; p < lib.second.initFunc + 0x300; p++) {
				if (p[0] == 0x48 && memcmp(p, libInitSig, sizeof(libInitSig)) == 0) {

					for (auto p2 = p + 0x20; p2 < p + 0x120; p2++) {
						if (p2[0] == 0xE8
							&& p2[5] == 0xE8
							&& p2[10] == 0xE8
							&& p2[15] == 0xE8
							&& p2[20] == 0xE8) {
							globalsInitCode = p2;
							break;
						}
					}
				}
			}
		}

		if (globalsInitCode == nullptr) {
			Fail("LibraryManager::FindEoCGlobalsEoCApp(): Could not find eoc library init function!");
			return;
		}

		for (auto i = 1; i < std::size(GetStaticSymbols().EocRegisterFuncs); i++) {
			int32_t rel = *(int32_t *)(globalsInitCode + 1);
			uint8_t const * registrantFunc = globalsInitCode + rel + 5;
			GetStaticSymbols().EocRegisterFuncs[i] = registrantFunc;
			globalsInitCode += 5;
		}

		for (auto i = 1; i < std::size(GetStaticSymbols().EocRegisterFuncs); i++) {
			uint8_t const * r = GetStaticSymbols().EocRegisterFuncs[i];
			if (r[0] == 0x48 && r[1] == 0x83 && r[2] == 0xEC /* sub rsp, XXh */
				&& r[4] == 0x48 && r[5] == 0x8B && r[6] == 0x05 /* mov rax, cs:xxx */) {

				int32_t rel = *(int32_t *)(r + 7);
				uint8_t const * registrantObj = r + rel + 4 + 7;
				GetStaticSymbols().EocGlobals[i] = (uint8_t const **)registrantObj;

			} else {
				GetStaticSymbols().EocGlobals[i] = nullptr;
			}
		}
	}

	void LibraryManager::FindGlobalStringTableEoCApp()
	{
		static uint8_t const sig1[] = {
			0xB9, 0x88, 0xFA, 0x5F, 0x00, // mov ecx, 5FFA88h
			0xE8, // call xxx
		};

		static uint8_t const sig2[] = {
			0x48, 0x85, 0xC0, //  test rax, rax
			0x74, 0x14, // jz xxx
			0x48, 0x8B, 0xC8, // mov rcx, rax
			0xE8, // call xxx
		};

		static uint8_t const sig3[] = {
			0x48, 0x89, 0x05 // mov xxx, rax
		};

		uint8_t const * p = (uint8_t const *)moduleStart_;
		uint8_t const * moduleEnd = p + moduleSize_;

		for (; p < moduleEnd - 100; p++) {
			if (*p == 0xB9
				&& memcmp(p, sig1, sizeof(sig1)) == 0
				&& memcmp(p + 10, sig2, sizeof(sig2)) == 0
				&& memcmp(p + 23, sig3, sizeof(sig3)) == 0) {
				int32_t rel = *(int32_t *)(p + 26);

				GetStaticSymbols().GlobalStrings = (GlobalStringTable const **)(p + rel + 23 + 7);
				break;
			}
		}

		if (GetStaticSymbols().GlobalStrings == nullptr) {
			ERR("LibraryManager::FindGlobalStringTableEoCApp(): Could not find global string table");
			CriticalInitFailed = true;
		}
	}
}
#endif
