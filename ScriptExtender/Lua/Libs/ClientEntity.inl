#include <Lua/Shared/LuaMethodHelpers.h>
#include <Extender/ScriptExtender.h>
#include <GameDefinitions/GameObjects/Player.h>

/// <lua_module>Entity</lua_module>
BEGIN_NS(ecl::lua::ecs)

void MakeLegacyClientCharacterObjectRef(lua_State* L, ecl::Character* value)
{
	if (value) {
		ObjectProxy2::MakeHandle<ecl::Character>(L, value->Base.Component.Handle, State::FromLua(L)->GetCurrentLifetime());
	} else {
		push(L, nullptr);
	}
}

void MakeLegacyClientItemObjectRef(lua_State* L, ecl::Item* value)
{
	if (value) {
		ObjectProxy2::MakeHandle<ecl::Item>(L, value->Base.Component.Handle, State::FromLua(L)->GetCurrentLifetime());
	} else {
		push(L, nullptr);
	}
}

Character* LuaGetCharacter(lua_State* L, int index)
{
	switch (lua_type(L, index)) {
	case LUA_TLIGHTUSERDATA:
	{
		auto handle = get<ComponentHandle>(L, index);
		return GetEntityWorld()->GetComponent<ecl::Character>(handle);
	}

	case LUA_TNUMBER:
	{
		NetId netId{ (uint32_t)lua_tointeger(L, index) };
		return GetEntityWorld()->GetComponent<ecl::Character>(netId);
	}

	case LUA_TSTRING:
	{
		auto guid = lua_tostring(L, index);
		return GetEntityWorld()->GetComponent<ecl::Character>(guid);
	}

	default:
		OsiError("Expected character UUID, Handle or NetId");
		return nullptr;
	}
}

UserReturn GetCharacterLegacy(lua_State* L)
{
	ecl::Character* character = LuaGetCharacter(L, 1);
	MakeLegacyClientCharacterObjectRef(L, character);
	return 1;
}

Character* GetCharacter(lua_State* L)
{
	return LuaGetCharacter(L, 1);
}

Item* LuaGetItem(lua_State* L, int index)
{
	switch (lua_type(L, index)) {
	case LUA_TLIGHTUSERDATA:
	{
		auto handle = get<ComponentHandle>(L, index);
		return GetEntityWorld()->GetComponent<ecl::Item>(handle);
	}

	case LUA_TNUMBER:
	{
		NetId netId{ (uint32_t)lua_tointeger(L, index) };
		return GetEntityWorld()->GetComponent<ecl::Item>(netId);
	}

	case LUA_TSTRING:
	{
		auto guid = lua_tostring(L, index);
		return GetEntityWorld()->GetComponent<ecl::Item>(guid);
	}

	default:
		OsiError("Expected item UUID, Handle or NetId; got " << lua_typename(L, lua_type(L, index)));
		return nullptr;
	}
}

UserReturn GetItemLegacy(lua_State* L)
{
	ecl::Item* item = LuaGetItem(L, 1);
	MakeLegacyClientItemObjectRef(L, item);
	return 1;
}

Item* GetItem(lua_State* L)
{
	return LuaGetItem(L, 1);
}

CombatComponent* GetCombatComponent(ComponentHandle handle)
{
	return GetEntityWorld()->GetComponent<CombatComponent>(handle);
}

Inventory* GetInventory(ComponentHandle handle)
{
	auto factory = *GetStaticSymbols().ecl__InventoryFactory;
	return factory->Get(handle);
}

Status* GetStatus(lua_State* L)
{
	auto character = LuaGetCharacter(L, 1);
	if (character == nullptr) return 0;

	if (lua_type(L, 2) == LUA_TLIGHTUSERDATA) {
		auto statusHandle = get<ComponentHandle>(L, 2);
		return character->GetStatus(statusHandle);
	} else {
		auto index = lua_tointeger(L, 2);
		NetId statusNetId{ (uint32_t)index };
		return character->GetStatus(statusNetId);
	}
}

IEoCClientObject* GetGameObject(lua_State* L)
{
	switch (lua_type(L, 1)) {
	case LUA_TLIGHTUSERDATA:
	{
		auto handle = get<ComponentHandle>(L, 1);
		return GetEntityWorld()->GetGameObject(handle);
	}

	case LUA_TSTRING:
	{
		auto guid = lua_tostring(L, 1);
		return GetEntityWorld()->GetGameObject(guid);
	}

	default:
		OsiError("Expected object GUID or handle, got " << lua_typename(L, lua_type(L, 1)));
		return nullptr;
	}
}

Projectile* LuaGetProjectile(lua_State* L, int index)
{
	auto level = GetStaticSymbols().GetCurrentClientLevel();
	if (!level || !level->EntityManager || !level->EntityManager->ProjectileConversionHelpers.Factory) {
		OsiError("No current level!");
		return nullptr;
	}

	switch (lua_type(L, index)) {
	case LUA_TLIGHTUSERDATA:
	{
		auto handle = get<ComponentHandle>(L, index);
		return level->EntityManager->ProjectileConversionHelpers.Factory->Get(handle);
	}

	case LUA_TNUMBER:
	{
		NetId netId{ (uint32_t)lua_tointeger(L, index) };
		return level->EntityManager->ProjectileConversionHelpers.Factory->FindByNetId(netId);
	}

	default:
		OsiError("Expected client projectile Handle or NetId; got " << lua_typename(L, lua_type(L, index)));
		return nullptr;
	}
}

Projectile* GetProjectile(lua_State* L)
{
	return LuaGetProjectile(L, 1);
}

eoc::AiGrid* GetAiGrid()
{
	auto level = GetStaticSymbols().GetCurrentClientLevel();
	if (!level || !level->AiGrid) {
		OsiError("Current level not available yet!");
		return 0;
	}

	return level->AiGrid;
}

eoc::VisionGrid* GetVisionGrid()
{
	auto level = GetStaticSymbols().GetCurrentClientLevel();
	if (!level || !level->VisionGrid) {
		OsiError("Current level not available yet!");
		return 0;
	}

	return level->VisionGrid;
}

Level* GetCurrentLevel()
{
	return GetStaticSymbols().GetCurrentClientLevel();
}

PlayerManager* GetPlayerManager()
{
	return (ecl::PlayerManager*)*GetStaticSymbols().ls__PlayerManager__Instance;
}

TurnManager* GetTurnManager()
{
	return GetEntityWorld()->GetTurnManager();
}


ComponentHandle NullHandle()
{
	return ComponentHandle(ComponentHandle::NullHandle);
}

void RegisterEntityLib()
{
	DECLARE_MODULE(Entity, Client)
	BEGIN_MODULE()
	MODULE_FUNCTION(NullHandle)
	MODULE_FUNCTION(GetCharacter)
	MODULE_FUNCTION(GetCharacterLegacy)
	MODULE_FUNCTION(GetItem)
	MODULE_FUNCTION(GetItemLegacy)
	MODULE_FUNCTION(GetCombatComponent)
	MODULE_FUNCTION(GetInventory)
	MODULE_FUNCTION(GetStatus)
	MODULE_FUNCTION(GetGameObject)
	MODULE_FUNCTION(GetProjectile)
	MODULE_FUNCTION(GetAiGrid)
	MODULE_FUNCTION(GetVisionGrid)
	MODULE_FUNCTION(GetCurrentLevel)
	MODULE_FUNCTION(GetPlayerManager)
	MODULE_FUNCTION(GetTurnManager)
	END_MODULE()
}

END_NS()
