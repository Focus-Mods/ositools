#pragma once

#include <GameDefinitions/Components/Character.h>
#include <GameDefinitions/CustomStats.h>
#include <GameDefinitions/GameObjects/GameAction.h>
#include <GameDefinitions/Components/Item.h>
#include <GameDefinitions/Misc.h>
#include <GameDefinitions/Osiris.h>
#include <GameDefinitions/GameObjects/Status.h>
#include <GameDefinitions/GameObjects/ActionMachine.h>
#include <GameDefinitions/TurnManager.h>
#include <GameDefinitions/Symbols.h>
#include <GameHooks/Wrappers.h>
#include <optional>

namespace dse {


	class WriteAnchor
	{
	public:
		inline WriteAnchor(uint8_t const * ptr, std::size_t size)
			: ptr_(const_cast<uint8_t *>(ptr)),
			size_(size)
		{
			BOOL succeeded = VirtualProtect((LPVOID)ptr_, size_, PAGE_READWRITE, &oldProtect_);
			if (!succeeded) Fail("VirtualProtect() failed");
		}

		inline ~WriteAnchor()
		{
			BOOL succeeded = VirtualProtect((LPVOID)ptr_, size_, oldProtect_, &oldProtect_);
			if (!succeeded) Fail("VirtualProtect() failed");
		}

		inline uint8_t * ptr()
		{
			return ptr_;
		}

	private:
		uint8_t * ptr_;
		std::size_t size_;
		DWORD oldProtect_;
	};

	struct Pattern
	{
		void FromString(std::string_view s);
		void FromRaw(const char * s);
		void Scan(uint8_t const * start, size_t length, std::function<std::optional<bool> (uint8_t const *)> callback, bool multiple = true);

	private:
		struct PatternByte
		{
			uint8_t pattern;
			uint8_t mask;
		};

		std::vector<PatternByte> pattern_;

		bool MatchPattern(uint8_t const * start);
		void ScanPrefix1(uint8_t const * start, uint8_t const * end, std::function<std::optional<bool> (uint8_t const *)> callback, bool multiple);
		void ScanPrefix2(uint8_t const * start, uint8_t const * end, std::function<std::optional<bool> (uint8_t const *)> callback, bool multiple);
		void ScanPrefix4(uint8_t const * start, uint8_t const * end, std::function<std::optional<bool> (uint8_t const *)> callback, bool multiple);
	};

	uint8_t const * AsmResolveInstructionRef(uint8_t const * code);

	struct SymbolMappingCondition
	{
		enum MatchType
		{
			kNone,
			kString, // Match string
			kFixedString, // Match a FixedString reference
		};

		MatchType Type{ kNone };
		int32_t Offset{ 0 };
		char const * String{ nullptr };
	};

	enum class SymbolMappingResult
	{
		Success,
		Fail,
		TryNext
	};

	struct StaticSymbolRef
	{
		std::size_t Offset;
		void ** TargetPtr;

		inline StaticSymbolRef()
			: Offset(-1), TargetPtr(nullptr)
		{}

		inline StaticSymbolRef(void ** ptr)
			: Offset(-1), TargetPtr(ptr)
		{}

		explicit inline StaticSymbolRef(std::size_t offset)
			: Offset(offset), TargetPtr(nullptr)
		{}

		inline void ** Get() const
		{
			if (Offset != -1) {
				return (void **)((uint8_t *)&GetStaticSymbols() + Offset);
			} else {
				return TargetPtr;
			}
		}
	};

	#define STATIC_SYM(name) StaticSymbolRef(offsetof(StaticSymbols, name))
	#define CHAR_GETTER_SYM(name) StaticSymbolRef(offsetof(StaticSymbols, CharStatsGetters) + offsetof(CharacterStatsGetters, name))

	struct SymbolMappingTarget
	{
		typedef SymbolMappingResult (* HandlerProc)(uint8_t const *);

		enum ActionType
		{
			kNone,
			kAbsolute, // Save absolute value (p + Offset)
			kIndirect // Save AsmResolveIndirectRef(p + Offset)
		};

		char const * Name{ nullptr };
		ActionType Type{ kNone };
		int32_t Offset{ 0 };
		StaticSymbolRef Target;
		HandlerProc Handler{ nullptr };
		struct SymbolMappingData const * NextSymbol{ nullptr };
		int32_t NextSymbolSeekSize{ 0 };
	};

	enum class SymbolVersion
	{
		None,
		Below,
		AboveOrEqual
	};

	struct SymbolMappingData
	{
		struct VersionRequirement
		{
			SymbolVersion Type{ SymbolVersion::None };
			uint32_t Revision{ 0 };
		};

		enum MatchScope
		{
			kBinary, // Full binary
			kText, // .text segment of binary
			kCustom // Custom scope (specified as scan parameter)
		};

		enum Flags : uint32_t
		{
			kCritical = 1 << 0, // Mapping failure causes a critical error
			kDeferred = 1 << 1, // Perform mapping after fixed string pool was loaded
			kAllowFail = 1 << 2, // Allow mapping to fail without throwing an error
		};

		char const * Name{ nullptr };
		MatchScope Scope{ SymbolMappingData::kText };
		uint32_t Flag{ 0 };
		char const * Matcher;
		SymbolMappingCondition Conditions;
		SymbolMappingTarget Target1;
		SymbolMappingTarget Target2;
		SymbolMappingTarget Target3;
		VersionRequirement Version;
	};

	struct GameVersionInfo
	{
		uint16_t Major{ 0 }, Minor{ 0 }, Revision{ 0 }, Build{ 0 };

		inline bool IsSupported() const
		{
			// We need v3.6.54 or later for game, v3.6.51+ for editor
			return (Major == 3 && Minor > 6)
#if defined(OSI_EOCAPP)
				|| (Major == 3 && Minor == 6 && Revision >= 54);
#else
				|| (Major == 3 && Minor == 6 && Revision >= 51);
#endif
		}
	};

	class LibraryManager
	{
	public:
		bool FindLibraries(uint32_t gameRevision);
		bool PostStartupFindLibraries();
		void EnableCustomStats();
		void DisableItemFolding();
		void Cleanup();
		bool GetGameVersion(GameVersionInfo & version);

		void ShowStartupError(STDWString const & msg, bool wait, bool exitGame);
		void ShowStartupError(STDWString const & msg, bool exitGame);
		void ShowStartupMessage(STDWString const & msg, bool exitGame);

		bool EvaluateSymbolCondition(SymbolMappingCondition const & cond, uint8_t const * match);
		SymbolMappingResult ExecSymbolMappingAction(SymbolMappingTarget const & target, uint8_t const * match);
		bool MapSymbol(SymbolMappingData const & mapping, uint8_t const * customStart, std::size_t customSize);

		inline uint8_t const * GetModuleStart() const
		{
			return moduleStart_;
		}

		inline size_t GetModuleSize() const
		{
			return moduleSize_;
		}

		inline bool CriticalInitializationFailed() const
		{
			return CriticalInitFailed;
		}

		inline bool InitializationFailed() const
		{
			return InitFailed;
		}

#define HOOK_DEFN(name, sym, defn, hookType) enum class name##Tag {}; \
	hookType<name##Tag, defn> name;
#include <GameDefinitions/EngineHooks.inl>
#undef HOOK_DEFN

	private:

		void MapAllSymbols(bool deferred);
		void FindTextSegment();

#if defined(OSI_EOCAPP)
		bool FindEoCApp(uint8_t const * & start, size_t & size);
		void FindServerGlobalsEoCApp();
		void FindEoCGlobalsEoCApp();
		void FindGlobalStringTableEoCApp();
#else
		bool FindEoCPlugin(uint8_t const * & start, size_t & size);
		void FindExportsEoCPlugin();
		void FindServerGlobalsEoCPlugin();
		void FindEoCGlobalsEoCPlugin();
		void FindGlobalStringTableCoreLib();
#endif

		void FindExportsIggy();

		bool IsConstStringRef(uint8_t const * ref, char const * str) const;
		bool IsFixedStringRef(uint8_t const * ref, char const * str) const;
		bool CanShowError();
		bool CanShowMessages();

		uint8_t const * moduleStart_{ nullptr };
		size_t moduleSize_{ 0 };
		uint8_t const * moduleTextStart_{ nullptr };
		size_t moduleTextSize_{ 0 };
		uint32_t gameRevision_;

#if !defined(OSI_EOCAPP)
		HMODULE coreLib_{ NULL };
		uint8_t const * coreLibStart_{ nullptr };
		size_t coreLibSize_{ 0 };
		HMODULE gameEngine_{ NULL };
#endif

		bool InitFailed{ false };
		bool CriticalInitFailed{ false };
		bool PostLoaded{ false };
		bool EnabledCustomStats{ false };
		bool EnabledCustomStatsPane{ false };
	};
}
