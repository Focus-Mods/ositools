#pragma once

#include <GameDefinitions/Misc.h>
#include <GameDefinitions/Symbols.h>
#include <GameHooks/SymbolMapper.h>

BEGIN_SE()

struct GameVersionInfo
{
	uint16_t Major{ 0 }, Minor{ 0 }, Revision{ 0 }, Build{ 0 };

	inline bool IsSupported() const
	{
		// We need v3.6.69 or later for game, v3.6.51+ for editor
		return (Major == 3 && Minor > 6)
#if defined(OSI_EOCAPP)
			|| (Major == 3 && Minor == 6 && Revision >= 69);
#else
			|| (Major == 3 && Minor == 6 && Revision >= 51);
#endif
	}
};

class LibraryManager
{
public:
	LibraryManager();

	bool FindLibraries(uint32_t gameRevision);
	bool PostStartupFindLibraries();
	void EnableCustomStats();
	void DisableItemFolding();
	bool GetGameVersion(GameVersionInfo & version);

	void ShowStartupError(STDWString const & msg, bool wait, bool exitGame);
	void ShowStartupError(STDWString const & msg, bool exitGame);
	void ShowStartupMessage(STDWString const & msg, bool exitGame);

	inline bool CriticalInitializationFailed() const
	{
		return CriticalInitFailed;
	}

	inline bool InitializationFailed() const
	{
		return InitFailed;
	}

	inline SymbolMapper& Mapper()
	{
		return symbolMapper_;
	}

private:

	void PreRegisterLibraries(SymbolMappingLoader& loader);
	void RegisterLibraries(SymbolMapper& mapper);
	void RegisterSymbols();
	bool BindApp();
#if defined(OSI_EOCAPP)
	void FindServerGlobalsEoCApp();
	void FindEoCGlobalsEoCApp();
	void FindGlobalStringTableEoCApp();
#else
	void FindServerGlobalsEoCPlugin();
	void FindEoCGlobalsEoCPlugin();
	void FindGlobalStringTableCoreLib();
#endif

	bool CanShowError();
	bool CanShowMessages();

	SymbolMappings mappings_;
	SymbolMapper symbolMapper_;
	SymbolMapper::ModuleInfo appModule_;
	SymbolMapper::ModuleInfo gameEngineModule_;
	SymbolMapper::ModuleInfo coreLibModule_;

	bool InitFailed{ false };
	bool CriticalInitFailed{ false };
	bool PostLoaded{ false };
	bool EnabledCustomStats{ false };
	bool EnabledCustomStatsPane{ false };
	bool enabledCustomAlignments_{ false };
};

END_SE()
