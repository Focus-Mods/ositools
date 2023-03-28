#include <Lua/Shared/LuaMethodHelpers.h>
#include <GameDefinitions/Symbols.h>
#include <Extender/ScriptExtender.h>

/// <lua_module>Vars</lua_module>
BEGIN_NS(lua::vars)

UserVariableFlags ParseUserVariableFlags(lua_State* L, int index)
{
	UserVariableFlags flags{ 0 };

	if (try_gettable<bool>(L, "Server", index, true)) {
		flags |= UserVariableFlags::IsOnServer;
	}

	if (try_gettable<bool>(L, "Client", index, false)) {
		flags |= UserVariableFlags::IsOnClient;
	}

	if ((flags & UserVariableFlags::IsOnServer) == UserVariableFlags::IsOnServer) {
		if (try_gettable<bool>(L, "WriteableOnServer", index, true)) {
			flags |= UserVariableFlags::WriteableOnServer;
		}

		if (try_gettable<bool>(L, "Persistent", index, true)) {
			flags |= UserVariableFlags::Persistent;
		}
	}

	if ((flags & UserVariableFlags::IsOnClient) == UserVariableFlags::IsOnClient) {
		if (try_gettable<bool>(L, "WriteableOnClient", index, false)) {
			flags |= UserVariableFlags::WriteableOnClient;
		}
	}

	if ((flags & (UserVariableFlags::IsOnClient|UserVariableFlags::IsOnServer)) == (UserVariableFlags::IsOnClient | UserVariableFlags::IsOnServer)) {
		if (try_gettable<bool>(L, "SyncToClient", index, false)) {
			flags |= UserVariableFlags::SyncServerToClient;
		}

		if (try_gettable<bool>(L, "SyncToServer", index, false)) {
			flags |= UserVariableFlags::SyncClientToServer;
		}
	}

	if (try_gettable<bool>(L, "SyncOnWrite", index, false)) {
		flags |= UserVariableFlags::SyncOnWrite;
	}

	if (try_gettable<bool>(L, "DontCache", index, false)) {
		flags |= UserVariableFlags::DontCache;
	}

	if (try_gettable<bool>(L, "SyncOnTick", index, true)) {
		flags |= UserVariableFlags::SyncOnTick;
	}

	return flags;
}

void RegisterUserVariable(lua_State* L, FixedString name)
{
	auto& vars = gExtender->GetCurrentExtensionState()->GetUserVariables();
	UserVariablePrototype proto;

	luaL_checktype(L, 2, LUA_TTABLE);
	proto.Flags = ParseUserVariableFlags(L, 2);

	vars.RegisterPrototype(name, proto);
}

void SyncUserVariables()
{
	auto& vars = gExtender->GetCurrentExtensionState()->GetUserVariables();
	vars.Flush(true);
}

void DirtyUserVariables(std::optional<FixedString> gameObject, std::optional<FixedString> key)
{
	auto& vars = gExtender->GetCurrentExtensionState()->GetUserVariables();
	if (!gameObject) {
		for (auto& entity : vars.GetAll()) {
			for (auto& var : entity.Value.Vars) {
				vars.MarkDirty(entity.Key, var.Key, var.Value);
			}
		}
	} else if (!key) {
		auto objectVars = vars.GetAll(*gameObject);
		if (objectVars != nullptr) {
			for (auto& var : *objectVars) {
				vars.MarkDirty(*gameObject, var.Key, var.Value);
			}
		}
	} else {
		auto var = vars.Get(*gameObject, *key);
		if (var != nullptr) {
			vars.MarkDirty(*gameObject, *key, *var);
		}
	}
}

void RegisterModVariable(lua_State* L, FixedString moduleUuid, FixedString name)
{
	auto vars = gExtender->GetCurrentExtensionState()->GetModVariables().GetOrCreateMod(moduleUuid);
	UserVariablePrototype proto;

	luaL_checktype(L, 3, LUA_TTABLE);
	proto.Flags = ParseUserVariableFlags(L, 3);

	vars->RegisterPrototype(name, proto);
}

UserReturn GetModVariables(lua_State* L, FixedString moduleUuid)
{
	auto& mv = gExtender->GetCurrentExtensionState()->GetModVariables();
	auto modIndex = mv.GuidToNetId(moduleUuid);
	if (!modIndex) {
		OsiError("Module '" << moduleUuid << "' is not loaded!");
		push(L, nullptr);
		return 1;
	}

	mv.GetOrCreateMod(moduleUuid);
	ModVariableHolderMetatable::Make(L, modIndex->second.Id);
	return 1;
}

void SyncModVariables()
{
	auto& vars = gExtender->GetCurrentExtensionState()->GetModVariables();
	vars.Flush(true);
}

void DirtyModVariables(std::optional<FixedString> moduleUuid, std::optional<FixedString> key)
{
	auto& vars = gExtender->GetCurrentExtensionState()->GetModVariables();
	if (!moduleUuid) {
		for (auto& mod : vars.GetAll()) {
			for (auto& var : mod.Value.GetAll()) {
				vars.MarkDirty(mod.Key, var.Key, var.Value);
			}
		}
	} else if (!key) {
		auto modVars = vars.GetAll(*moduleUuid);
		if (modVars != nullptr) {
			for (auto& var : *modVars) {
				vars.MarkDirty(*moduleUuid, var.Key, var.Value);
			}
		}
	} else {
		auto var = vars.Get(*moduleUuid, *key);
		if (var != nullptr) {
			vars.MarkDirty(*moduleUuid, *key, *var);
		}
	}
}

void RegisterVarsLib()
{
	DECLARE_MODULE(Vars, Both)
	BEGIN_MODULE()
	MODULE_FUNCTION(RegisterUserVariable)
	MODULE_FUNCTION(SyncUserVariables)
	MODULE_FUNCTION(DirtyUserVariables)

	MODULE_FUNCTION(RegisterModVariable)
	MODULE_FUNCTION(GetModVariables)
	MODULE_FUNCTION(SyncModVariables)
	MODULE_FUNCTION(DirtyModVariables)
	END_MODULE()
}

END_NS()
