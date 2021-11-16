#include <stdafx.h>
#include <GameDefinitions/Components/Projectile.h>
#include <GameDefinitions/GameObjects/Ai.h>
#include <GameDefinitions/GameObjects/Surface.h>
#include <GameDefinitions/Components/Trigger.h>
#include <GameDefinitions/CustomStats.h>
#include <Lua/Server/LuaBindingServer.h>
#include <Lua/Shared/LuaSerializers.h>
#include <Lua/Shared/LuaMethodHelpers.h>
#include <Extender/ScriptExtender.h>
#include <GameDefinitions/PropertyMaps/PropertyMaps.h>
#include <ScriptHelpers.h>
#include "resource.h"

BEGIN_SE()

ObjectSet<FixedString> IGameObject::LuaGetTags()
{
	ObjectSet<FixedString> tags;
	GetTags(tags);
	return tags;
}

bool IGameObject::LuaIsTagged(FixedString const& tag)
{
	return IsTagged(tag);
}

END_SE()

BEGIN_NS(lua)

// FIXME - deprecated; use state from L wherever possible
LifetimeHolder GetCurrentLifetime()
{
	if (gExtender->GetServer().IsInServerThread()) {
		return esv::lua::GetServerLifetime();
	}
	else {
		return ecl::lua::GetClientLifetime();
	}
}

LifetimePool& GetLifetimePool()
{
	if (gExtender->GetServer().IsInServerThread()) {
		return esv::lua::GetServerLifetimePool();
	}
	else {
		return ecl::lua::GetClientLifetimePool();
	}
}

END_NS()

namespace dse::esv::lua
{
	using namespace dse::lua;

	LifetimeHolder GetServerLifetime()
	{
		assert(gExtender->GetServer().IsInServerThread());
		return esv::ExtensionState::Get().GetLua()->GetCurrentLifetime();
	}

	LifetimePool& GetServerLifetimePool()
	{
		assert(gExtender->GetServer().IsInServerThread());
		return esv::ExtensionState::Get().GetLua()->GetLifetimePool();
	}

#include <Lua/Shared/LevelIteratorFunctions.inl>

	template <class Predicate>
	void GetTriggersGeneric(lua_State* L, FixedString const& requestedLevel, Predicate pred)
	{
		int index{ 1 };

		lua_newtable(L);
		FixedString levelName = requestedLevel;
		if (!levelName) {
			auto level = GetStaticSymbols().GetCurrentServerLevel();
			if (level == nullptr) {
				OsiError("No current level!");
				return;
			}

			levelName = level->LevelDesc->LevelName;
		}

		auto& helpers = GetEoCServer()->EntityManager->TriggerConversionHelpers;
		auto triggers = helpers.RegisteredTriggers.Find(levelName);
		if (triggers == nullptr) {
			OsiError("No triggers registered for level: " << levelName);
			return;
		}

		for (auto trigger : **triggers) {
			if (pred(trigger)) {
				auto guid = trigger->GetGuid();
				if (guid && *guid) {
					settable(L, index++, *guid);
				}
			}
		}
	}

	int GetAllTriggers(lua_State* L)
	{
		FixedString levelName;
		if (lua_gettop(L) >= 1) {
			levelName = get<FixedString>(L, 1);
		}

		GetTriggersGeneric(L, levelName, [](Trigger*) { return true; });
		return 1;
	}
}

#include <Lua/Server/ServerCharacter.inl>

namespace dse::lua
{
	using namespace dse::esv::lua;

	void PushHit(lua_State* L, HitDamageInfo const& hit)
	{
		lua_newtable(L);
		setfield(L, "Equipment", hit.Equipment);
		setfield(L, "TotalDamageDone", hit.TotalDamage);
		setfield(L, "DamageDealt", hit.DamageDealt);
		setfield(L, "DeathType", hit.DeathType);
		setfield(L, "DamageType", hit.DamageType);
		setfield(L, "AttackDirection", hit.AttackDirection);
		setfield(L, "ArmorAbsorption", hit.ArmorAbsorption);
		setfield(L, "LifeSteal", hit.LifeSteal);
		setfield(L, "EffectFlags", (int64_t)hit.EffectFlags);
		setfield(L, "HitWithWeapon", hit.HitWithWeapon);

		auto luaDamageList = DamageList::New(L);
		for (auto const& dmg : hit.DamageList) {
			luaDamageList->Get().SafeAdd(dmg);
		}
		
		lua_setfield(L, -2, "DamageList");
	}

	bool PopHit(lua_State* L, HitDamageInfo& hit, int index)
	{
		luaL_checktype(L, index, LUA_TTABLE);
		hit.Equipment = checked_getfield<uint32_t>(L, "Equipment", index);
		hit.TotalDamage = checked_getfield<int32_t>(L, "TotalDamageDone", index);
		hit.DamageDealt = checked_getfield<int32_t>(L, "DamageDealt", index);
		hit.DeathType = checked_getfield<DeathType>(L, "DeathType", index);
		hit.DamageType = checked_getfield<DamageType>(L, "DamageType", index);
		hit.AttackDirection = checked_getfield<uint32_t>(L, "AttackDirection", index);
		hit.ArmorAbsorption = checked_getfield<int32_t>(L, "ArmorAbsorption", index);
		hit.LifeSteal = checked_getfield<int32_t>(L, "LifeSteal", index);
		hit.HitWithWeapon = checked_getfield<bool>(L, "HitWithWeapon", index);
		hit.EffectFlags = (HitFlag)checked_getfield<uint32_t>(L, "EffectFlags", index);

		lua_getfield(L, index, "DamageList");
		auto damageList = DamageList::AsUserData(L, -1);
		lua_pop(L, 1);

		if (damageList == nullptr) {
			OsiErrorS("Missing 'DamageList' in Hit table");
			return false;
		} else {
			hit.DamageList.Clear();
			for (auto const& dmg : damageList->Get()) {
				hit.DamageList.AddDamage(dmg.DamageType, dmg.Amount);
			}
			return true;
		}
	}

	char const* const ObjectProxy<esv::Status>::MetatableName = "esv::Status";

	esv::Status* ObjectProxy<esv::Status>::GetPtr(lua_State* L)
	{
		if (obj_ == nullptr) luaL_error(L, "Status object no longer available");
		return obj_;
	}

	int ObjectProxy<esv::Status>::Index(lua_State* L)
	{
		if (obj_ == nullptr) return luaL_error(L, "Status object no longer available");

		StackCheck _(L, 1);
		auto prop = luaL_checkstring(L, 2);

		if (strcmp(prop, "StatusType") == 0) {
			push(L, obj_->GetStatusId());
			return 1;
		}

		if (obj_->GetStatusId() == StatusType::HIT && strcmp(prop, "Hit") == 0) {
			auto hit = static_cast<esv::StatusHit*>(obj_);
			PushHit(L, hit->DamageInfo);
			return 1;
		}

		auto& propertyMap = StatusToPropertyMap(obj_);
		auto fetched = LuaPropertyMapGet(L, propertyMap, obj_, prop, true);
		if (!fetched) push(L, nullptr);
		return 1;
	}

	int ObjectProxy<esv::Status>::NewIndex(lua_State* L)
	{
		StackCheck _(L, 0);

		if (obj_->GetStatusId() == StatusType::HIT
			&& strcmp(luaL_checkstring(L, 2), "Hit") == 0) {
			auto hit = static_cast<esv::StatusHit*>(obj_);
			HitDamageInfo damageInfo;
			PopHit(L, damageInfo, 3);
			hit->DamageInfo = damageInfo;
			return 0;
		}

		auto& propertyMap = StatusToPropertyMap(obj_);
		return GenericSetter(L, propertyMap);
	}


	char const* const ObjectProxy<esv::EsvTrigger>::MetatableName = "esv::Trigger";

	esv::EsvTrigger* ObjectProxy<esv::EsvTrigger>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		auto trigger = esv::GetEntityWorld()->GetTrigger(handle_);
		if (trigger == nullptr) luaL_error(L, "Trigger handle invalid");
		return static_cast<esv::EsvTrigger*>(trigger);
	}

	int ObjectProxy<esv::EsvTrigger>::Index(lua_State* L)
	{
		auto obj = Get(L);
		if (obj == nullptr) return luaL_error(L, "Trigger object no longer available");

		StackCheck _(L, 1);
		auto prop = get<FixedString>(L, 2);

		if (prop == GFS.strHandle) {
			ComponentHandle handle;
			obj->GetObjectHandle(handle);
			push(L, handle);
			return 1;
		}

		if (prop == GFS.strUUID) {
			push(L, *obj->GetGuid());
			return 1;
		}

		if (prop == GFS.strTriggerData) {
			auto atm = reinterpret_cast<esv::AtmosphereTrigger*>(obj);
			if (!obj->Template || !obj->Template->TriggerTypeData) {
				push(L, nullptr);
			} else if (obj->TriggerType == GFS.strTriggerAtmosphere) {
				auto triggerData = static_cast<AtmosphereTriggerData*>(obj->Template->TriggerTypeData);
				ObjectProxy<AtmosphereTriggerData>::New(L, GetServerLifetime(), triggerData);
			} else {
				LuaError("TriggerData for trigger type '" << obj->TriggerType << "' not supported yet!");
				push(L, nullptr);
			}

			return 1;
		}

		return GenericGetter(L, gTriggerPropertyMap);
	}

	int ObjectProxy<esv::EsvTrigger>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gTriggerPropertyMap);
	}


	char const* const ObjectProxy<AtmosphereTriggerData>::MetatableName = "ls::AtmosphereTriggerData";

	AtmosphereTriggerData* ObjectProxy<AtmosphereTriggerData>::GetPtr(lua_State* L)
	{
		if (obj_ == nullptr) luaL_error(L, "AtmosphereTriggerData object no longer available");
		return obj_;
	}

	int ObjectProxy<AtmosphereTriggerData>::Index(lua_State* L)
	{
		if (obj_ == nullptr) return luaL_error(L, "AtmosphereTriggerData object no longer available");

		StackCheck _(L, 1);
		auto prop = get<FixedString>(L, 2);

		if (prop == GFS.strAtmospheres) {
			return LuaWrite(L, obj_->Atmospheres);
		} else if (prop == GFS.strFadeTime) {
			return LuaWrite(L, obj_->FadeTime);
		} else {
			push(L, nullptr);
			return 1;
		}
	}

	int ObjectProxy<AtmosphereTriggerData>::NewIndex(lua_State* L)
	{
		StackCheck _(L, 0);
		auto prop = get<FixedString>(L, 2);

		if (prop == GFS.strAtmospheres) {
			lua_pushvalue(L, 3);
			LuaRead(L, obj_->Atmospheres);
			lua_pop(L, 1);
		} else if (prop == GFS.strFadeTime) {
			lua_pushvalue(L, 3);
			LuaRead(L, obj_->FadeTime);
			lua_pop(L, 1);
		} else {
			LuaError("Unsupported atmosphere property: " << prop);
		}

		return 0;
	}


	char const* const ObjectProxy<SoundVolumeTriggerData>::MetatableName = "esv::SoundVolumeTriggerData";

	SoundVolumeTriggerData* ObjectProxy<SoundVolumeTriggerData>::GetPtr(lua_State* L)
	{
		if (obj_ == nullptr) luaL_error(L, "SoundVolumeTriggerData object no longer available");
		return obj_;
	}

	int ObjectProxy<SoundVolumeTriggerData>::Index(lua_State* L)
	{
		if (obj_ == nullptr) return luaL_error(L, "SoundVolumeTriggerData object no longer available");

		return GenericGetter(L, gSoundVolumeTriggerDataPropertyMap);
	}

	int ObjectProxy<SoundVolumeTriggerData>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gSoundVolumeTriggerDataPropertyMap);
	}


#include <Lua/Shared/LuaShared.inl>

	char const* const ObjectProxy<esv::PlayerCustomData>::MetatableName = "esv::PlayerCustomData";

	esv::PlayerCustomData* ObjectProxy<esv::PlayerCustomData>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		auto character = esv::GetEntityWorld()->GetCharacter(handle_);
		if (character == nullptr) luaL_error(L, "Character handle invalid");

		if (character->PlayerData == nullptr
			|| !character->PlayerData->CustomData.Initialized) {
			OsiError("Character has no player data, or custom data was not initialized.");
			return nullptr;
		}

		return &character->PlayerData->CustomData;
	}

	int ObjectProxy<esv::PlayerCustomData>::Index(lua_State* L)
	{
		return GenericGetter(L, gPlayerCustomDataPropertyMap);
	}

	int ObjectProxy<esv::PlayerCustomData>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gPlayerCustomDataPropertyMap);
	}


	char const* const ObjectProxy<esv::Item>::MetatableName = "esv::Item";

	esv::Item* ObjectProxy<esv::Item>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		auto item = esv::GetEntityWorld()->GetItem(handle_);
		if (item == nullptr) luaL_error(L, "Item handle invalid");
		return item;
	}

	int ItemGetInventoryItems(lua_State* L)
	{
		StackCheck _(L, 1);
		auto self = get<ObjectProxy<esv::Item>*>(L, 1);

		// FIXME - GetInventoryItems(L, self->Get(L)->InventoryHandle);

		return 1;
	}

	int ItemGetNearbyCharacters(lua_State* L)
	{
		StackCheck _(L, 1);
		auto self = get<ObjectProxy<esv::Item>*>(L, 1);
		auto pos = self->Get(L)->WorldPos;
		auto distance = get<float>(L, 2);

		esv::lua::GetCharactersGenericOld(L, FixedString{}, [pos, distance](esv::Character* c) {
			return abs(glm::length(pos - c->WorldPos)) < distance;
		});
		return 1;
	}

	int ItemGetGeneratedBoosts(lua_State* L)
	{
		auto self = get<ObjectProxy<esv::Item>*>(L, 1);
		auto item = self->Get(L);
		if (!item) return 0;

		StackCheck _(L, 1);
		lua_newtable(L);
		int32_t index{ 1 };
		if (item->Generation != nullptr) {
			for (auto const& boost : item->Generation->Boosts) {
				settable(L, index++, boost);
			}
		}

		return 1;
	}

	int ItemSetGeneratedBoosts(lua_State* L)
	{
		StackCheck _(L, 0);
		auto self = get<ObjectProxy<esv::Item>*>(L, 1);
		auto item = self->Get(L);
		if (!item) return 0;

		CompactObjectSet<FixedString> boosts;
		LuaRead(L, boosts);

		if (item->Generation != nullptr) {
			item->Generation->Boosts = boosts;
		}

		return 0;
	}

	int ObjectProxy<esv::Item>::Index(lua_State* L)
	{
		auto item = Get(L);
		if (!item) return 0;

		StackCheck _(L, 1);
		auto propFS = get<FixedString>(L, 2);

		if (propFS == GFS.strGetInventoryItems) {
			lua_pushcfunction(L, &ItemGetInventoryItems);
			return 1;
		}

		if (propFS == GFS.strGetNearbyCharacters) {
			lua_pushcfunction(L, &ItemGetNearbyCharacters);
			return 1;
		}

		if (propFS == GFS.strGetDeltaMods) {
			lua_pushcfunction(L, &ItemGetDeltaMods<esv::Item>);
			return 1;
		}

		if (propFS == GFS.strSetDeltaMods) {
			lua_pushcfunction(L, &ItemSetDeltaMods<esv::Item>);
			return 1;
		}

		if (propFS == GFS.strGetGeneratedBoosts) {
			lua_pushcfunction(L, &ItemGetGeneratedBoosts);
			return 1;
		}

		if (propFS == GFS.strSetGeneratedBoosts) {
			lua_pushcfunction(L, &ItemSetGeneratedBoosts);
			return 1;
		}

		if (propFS == GFS.strHasTag) {
			lua_pushcfunction(L, &GameObjectHasTag<esv::Item>);
			return 1;
		}

		if (propFS == GFS.strGetTags) {
			lua_pushcfunction(L, &GameObjectGetTags<esv::Item>);
			return 1;
		}

		if (propFS == GFS.strGetStatus) {
			lua_pushcfunction(L, (&GameObjectGetStatus<esv::Item, esv::Status>));
			return 1;
		}

		if (propFS == GFS.strGetStatusByType) {
			lua_pushcfunction(L, (&GameObjectGetStatusByType<esv::Item, esv::Status>));
			return 1;
		}

		if (propFS == GFS.strGetStatuses) {
			lua_pushcfunction(L, (&GameObjectGetStatuses<esv::Item>));
			return 1;
		}

		if (propFS == GFS.strGetStatusObjects) {
			lua_pushcfunction(L, (&GameObjectGetStatusObjects<esv::Item, esv::Status>));
			return 1;
		}

		if (propFS == GFS.strStats) {
			if (item->Stats != nullptr) {
				ObjectProxy<CDivinityStats_Item>::New(L, item->Base.Component.Handle);
				return 1;
			} else {
				push(L, nullptr);
				return 1;
			}
		}

		if (propFS == GFS.strHandle) {
			push(L, item->Base.Component.Handle);
			return 1;
		}

		if (propFS == GFS.strRootTemplate) {
			ObjectProxy<ItemTemplate>::New(L, GetServerLifetime(), item->CurrentTemplate);
			return 1;
		}

		if (propFS == GFS.strDisplayName) {
			return GameObjectGetDisplayName<esv::Item>(L, item);
		}

		bool fetched = false;
		if (item->Stats != nullptr) {
			fetched = LuaPropertyMapGet(L, gItemStatsPropertyMap, item->Stats, propFS, false);
		}

		if (!fetched) {
			fetched = LuaPropertyMapGet(L, gItemPropertyMap, item, propFS, true);
		}

		if (!fetched) push(L, nullptr);
		return 1;
	}

	int ObjectProxy<esv::Item>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gItemPropertyMap);
	}


	char const* const ObjectProxy<eoc::ItemDefinition>::MetatableName = "eoc::ItemDefinition";

	eoc::ItemDefinition* ObjectProxy<eoc::ItemDefinition>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		luaL_error(L, "ItemDefinition object has expired!");
		return nullptr;
	}

	int ItemDefinitionResetProgression(lua_State* L)
	{
		StackCheck _(L, 0);
		auto self = ObjectProxy<eoc::ItemDefinition>::CheckedGet(L, 2);
		self->LevelGroupIndex = -1;
		self->RootGroupIndex = -1;
		self->NameIndex = -1;
		self->NameCool = 0;
		return 0;
	}

	int ObjectProxy<eoc::ItemDefinition>::Index(lua_State* L)
	{
		StackCheck _(L, 1);
		auto prop = get<FixedString>(L, 2);
		if (prop == GFS.strResetProgression) {
			lua_pushcfunction(L, &ItemDefinitionResetProgression);
			return 1;
		}

		if (prop == GFS.strGenerationBoosts) {
			LuaWrite(L, Get(L)->GenerationBoosts);
			return 1;
		}

		if (prop == GFS.strRuneBoosts) {
			LuaWrite(L, Get(L)->RuneBoosts);
			return 1;
		}

		if (prop == GFS.strDeltaMods) {
			LuaWrite(L, Get(L)->DeltaMods);
			return 1;
		}

		return GenericGetter(L, gEoCItemDefinitionPropertyMap);
	}

	int ObjectProxy<eoc::ItemDefinition>::NewIndex(lua_State* L)
	{
		StackCheck _(L, 0);
		auto prop = get<FixedString>(L, 2);
		if (prop == GFS.strGenerationBoosts) {
			lua_pushvalue(L, 3);
			LuaRead(L, Get(L)->GenerationBoosts);
			lua_pop(L, 1);
			return 0;
		}

		if (prop == GFS.strRuneBoosts) {
			lua_pushvalue(L, 3);
			LuaRead(L, Get(L)->RuneBoosts);
			lua_pop(L, 1);
			return 0;
		}

		if (prop == GFS.strDeltaMods) {
			lua_pushvalue(L, 3);
			LuaRead(L, Get(L)->DeltaMods);
			lua_pop(L, 1);
			return 0;
		}

		return GenericSetter(L, gEoCItemDefinitionPropertyMap);
	}



	char const* const ObjectProxy<esv::ShootProjectileHelper>::MetatableName = "esv::ShootProjectileRequest";

	esv::ShootProjectileHelper* ObjectProxy<esv::ShootProjectileHelper>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		luaL_error(L, "ShootProjectileRequest object has expired!");
		return nullptr;
	}

	int ObjectProxy<esv::ShootProjectileHelper>::Index(lua_State* L)
	{
		return GenericGetter(L, gShootProjectileHelperPropertyMap);
	}

	int ObjectProxy<esv::ShootProjectileHelper>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gShootProjectileHelperPropertyMap);
	}


	char const* const ObjectProxy<esv::Projectile>::MetatableName = "esv::Projectile";

	esv::Projectile* ObjectProxy<esv::Projectile>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		auto projectile = esv::GetEntityWorld()->GetProjectile(handle_);
		if (projectile == nullptr) luaL_error(L, "Projectile handle invalid");
		return projectile;
	}

	int ObjectProxy<esv::Projectile>::Index(lua_State* L)
	{
		auto projectile = Get(L);
		if (!projectile) return 0;

		StackCheck _(L, 1);
		auto prop = luaL_checkstring(L, 2);

		if (strcmp(prop, GFS.strHandle.Str) == 0) {
			push(L, projectile->Base.Component.Handle);
			return 1;
		}

		if (strcmp(prop, GFS.strRootTemplate.Str) == 0) {
			ObjectProxy<ProjectileTemplate>::New(L, GetServerLifetime(), projectile->ProjectileTemplate);
			return 1;
		}

		bool fetched = LuaPropertyMapGet(L, gProjectilePropertyMap, projectile, prop, true);
		if (!fetched) push(L, nullptr);
		return 1;
	}

	int ObjectProxy<esv::Projectile>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gProjectilePropertyMap);
	}


	char const* const ObjectProxy<esv::Surface>::MetatableName = "esv::Surface";

	esv::Surface* ObjectProxy<esv::Surface>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;

		auto level = GetStaticSymbols().GetCurrentServerLevel();
		if (level) {
			auto surface = level->SurfaceManager->Get(handle_);
			if (surface == nullptr) luaL_error(L, "Surface handle invalid");
			// FIXME - TEMP CAST
			return (esv::Surface*)surface;
		} else {
			return nullptr;
		}
	}

	int ObjectProxy<esv::Surface>::Index(lua_State* L)
	{
		auto surface = Get(L);
		auto prop = get<char const*>(L, 2);

		if (strcmp(prop, "RootTemplate") == 0) {
			auto tmpl = GetStaticSymbols().GetSurfaceTemplate(surface->SurfaceType);
			if (tmpl != nullptr) {
				ObjectProxy<SurfaceTemplate>::New(L, GetServerLifetime(), tmpl);
			} else {
				LuaError("Couldn't fetch surface template of type " << (unsigned)surface->SurfaceType);
				push(L, nullptr);
			}
			return 1;
		}

		return GenericGetter(L, gEsvSurfacePropertyMap);
	}

	int ObjectProxy<esv::Surface>::NewIndex(lua_State* L)
	{
		return GenericSetter(L, gEsvSurfacePropertyMap);
	}


	char const* const ObjectProxy<esv::SurfaceAction>::MetatableName = "esv::SurfaceAction";

	esv::SurfaceAction* ObjectProxy<esv::SurfaceAction>::GetPtr(lua_State* L)
	{
		if (obj_) return obj_;
		luaL_error(L, "SurfaceAction object not bound (maybe it was executed already?)");
		return nullptr;
	}

	PropertyMapBase& GetSurfaceActionPropertyMap(esv::SurfaceAction* action)
	{
		switch (action->VMT->GetTypeId(action)) {
		case SurfaceActionType::CreateSurfaceAction:
			return gEsvCreateSurfaceActionPropertyMap;
		case SurfaceActionType::CreatePuddleAction:
			return gEsvCreatePuddleActionPropertyMap;
		case SurfaceActionType::ExtinguishFireAction:
			return gEsvExtinguishFireActionPropertyMap;
		case SurfaceActionType::ZoneAction:
			return gEsvZoneActionPropertyMap;
		case SurfaceActionType::TransformSurfaceAction:
			return gEsvTransformSurfaceActionPropertyMap;
		case SurfaceActionType::ChangeSurfaceOnPathAction:
			return gEsvChangeSurfaceOnPathActionPropertyMap;
		case SurfaceActionType::RectangleSurfaceAction:
			return gEsvRectangleSurfaceActionPropertyMap;
		case SurfaceActionType::PolygonSurfaceAction:
			return gEsvPolygonSurfaceActionPropertyMap;
		case SurfaceActionType::SwapSurfaceAction:
			return gEsvSwapSurfaceActionPropertyMap;
		default:
			OsiError("No property map found for this surface type!");
			return gEsvSurfaceActionPropertyMap;
		}
	}

	int ObjectProxy<esv::SurfaceAction>::Index(lua_State* L)
	{
		auto action = Get(L);
		if (!action) return 0;

		return GenericGetter(L, GetSurfaceActionPropertyMap(action));
	}

	int ObjectProxy<esv::SurfaceAction>::NewIndex(lua_State* L)
	{
		StackCheck _(L, 0);
		auto action = Get(L);

		auto prop = luaL_checkstring(L, 2);

		if (strcmp(prop, "DamageList") == 0) {
			auto& damageList = DamageList::CheckUserData(L, 3)->Get();
			switch (action->VMT->GetTypeId(action)) {
			case SurfaceActionType::RectangleSurfaceAction:
			{
				auto act = static_cast<esv::RectangleSurfaceAction*>(action);
				act->DamageList.CopyFrom(damageList);
				break;
			}

			case SurfaceActionType::PolygonSurfaceAction:
			{
				auto act = static_cast<esv::PolygonSurfaceAction*>(action);
				act->DamageList.CopyFrom(damageList);
				break;
			}

			case SurfaceActionType::ZoneAction:
			{
				auto act = static_cast<esv::ZoneAction*>(action);
				act->DamageList.CopyFrom(damageList);
				break;
			}

			default:
				OsiError("This surface action type doesn't have a DamageList!");
			}

			return 0;
		}

		if (strcmp(prop, "Vertices") == 0) {
			if (action->VMT->GetTypeId(action) == SurfaceActionType::PolygonSurfaceAction) {
				auto act = static_cast<esv::PolygonSurfaceAction*>(action);
				act->PolygonVertices.Clear();
				luaL_checktype(L, 3, LUA_TTABLE);
				for (auto idx : iterate(L, 3)) {
					auto vec2 = get<glm::vec2>(L, idx);
					act->PolygonVertices.Add(vec2);
				}
			} else {
				OsiError("Vertices only supported for surface action type PolygonSurfaceAction!");
			}

			return 0;
		}

		auto const& propertyMap = GetSurfaceActionPropertyMap(action);
		return GenericSetter(L, propertyMap);
	}
}

namespace dse::esv::lua
{
	using namespace dse::lua;

	char const* const StatusHandleProxy::MetatableName = "esv::HStatus";

	esv::Status* StatusHandleProxy::Get(lua_State* L)
	{
		esv::Status* status{ nullptr };
		if (owner_.GetType() == (uint32_t)ObjectType::ServerCharacter) {
			auto character = GetEntityWorld()->GetCharacter(owner_);
			if (character == nullptr) {
				luaL_error(L, "Character handle invalid");
				return nullptr;
			}

			if (statusHandle_) {
				status = character->GetStatus(statusHandle_, true, isUnapplied_);
			} else {
				status = character->GetStatus(statusNetId_);
			}
		} else {
			auto item = GetEntityWorld()->GetItem(owner_);
			if (item == nullptr) {
				luaL_error(L, "Item handle invalid");
				return nullptr;
			}

			if (statusHandle_) {
				status = item->GetStatus(statusHandle_, true, isUnapplied_);
			} else {
				status = item->GetStatus(statusNetId_);
			}
		}

		if (status == nullptr) luaL_error(L, "Status handle invalid");
		return status;
	}

	int StatusHandleProxy::Index(lua_State* L)
	{
		auto status = Get(L);
		if (!status) return 0;

		StackCheck _(L, 1);
		auto prop = luaL_checkstring(L, 2);

		if (status->GetStatusId() == StatusType::HIT && strcmp(prop, "Hit") == 0) {
			auto hit = static_cast<esv::StatusHit*>(status);
			PushHit(L, hit->DamageInfo);
			return 1;
		}

		auto& propertyMap = StatusToPropertyMap(status);
		auto fetched = LuaPropertyMapGet(L, propertyMap, status, prop, true);
		if (!fetched) push(L, nullptr);
		return 1;
	}

	int StatusHandleProxy::NewIndex(lua_State* L)
	{
		StackCheck _(L, 0);
		auto status = Get(L);
		if (!status) return 0;

		auto prop = luaL_checkstring(L, 2);

		if (status->GetStatusId() == StatusType::HIT
			&& strcmp(prop, "Hit") == 0) {
			auto hit = static_cast<esv::StatusHit*>(status);
			HitDamageInfo damageInfo;
			PopHit(L, damageInfo, 3);
			hit->DamageInfo = damageInfo;
			return 0;
		}

		auto& propertyMap = StatusToPropertyMap(status);
		LuaPropertyMapSet(L, 3, propertyMap, status, prop, true);
		return 0;
	}


	char const * const TurnManagerCombatProxy::MetatableName = "esv::TurnManager::Combat";

	void TurnManagerCombatProxy::PopulateMetatable(lua_State * L)
	{
		lua_newtable(L);

		lua_pushcfunction(L, &GetCurrentTurnOrder);
		lua_setfield(L, -2, "GetCurrentTurnOrder");

		lua_pushcfunction(L, &GetNextTurnOrder);
		lua_setfield(L, -2, "GetNextTurnOrder");

		lua_pushcfunction(L, &UpdateCurrentTurnOrder);
		lua_setfield(L, -2, "UpdateCurrentTurnOrder");

		lua_pushcfunction(L, &UpdateNextTurnOrder);
		lua_setfield(L, -2, "UpdateNextTurnOrder");

		lua_pushcfunction(L, &GetAllTeams);
		lua_setfield(L, -2, "GetAllTeams");

		lua_setfield(L, -2, "__index");
	}

	void CombatTeamListToLua(lua_State * L, CompactSet<esv::TurnManager::CombatTeam *, MSVCMemoryAllocator> const & teams)
	{
		lua_newtable(L);

		for (uint32_t i = 0; i < teams.Size; i++) {
			push(L, i + 1);
			TurnManagerTeamProxy::New(L, teams[i]->TeamId);
			lua_settable(L, -3);
		}
	}

	int TurnManagerCombatProxy::Index(lua_State * L)
	{
		auto combat = Get();
		if (combat == nullptr) return luaL_error(L, "Combat no longer available");

		StackCheck _(L, 1);
		auto prop = luaL_checkstring(L, 2);
		if (strcmp(prop, "CombatId") == 0) {
			push(L, combatId_);
		} else if (strcmp(prop, "LevelName") == 0) {
			push(L, combat->LevelName.Str);
		} else if (strcmp(prop, "IsActive") == 0) {
			push(L, combat->IsActive);
		} else {
			OsiError("Combat has no attribute named " << prop);
			push(L, nullptr);
		}

		return 1;
	}

	int TurnManagerCombatProxy::GetCurrentTurnOrder(lua_State * L)
	{
		auto self = TurnManagerCombatProxy::CheckUserData(L, 1);
		auto combat = self->Get();
		if (!combat) return 0;

		StackCheck _(L, 1);
		CombatTeamListToLua(L, combat->CurrentRoundTeams);
		return 1;
	}

	int TurnManagerCombatProxy::GetNextTurnOrder(lua_State * L)
	{
		auto self = TurnManagerCombatProxy::CheckUserData(L, 1);
		auto combat = self->Get();
		if (!combat) return 0;

		StackCheck _(L, 1);
		CombatTeamListToLua(L, combat->NextRoundTeams);
		return 1;
	}

	void UpdateTurnOrder(lua_State * L, uint8_t combatId, int index,
		ObjectSet<esv::TurnManager::CombatTeam *, MSVCMemoryAllocator> & combatTeams,
		ObjectSet<eoc::CombatTeamId> & combatNotifies)
	{
		if (lua_type(L, index) != LUA_TTABLE) luaL_error(L, "UpdateTurnOrder() expects a table of teams as the first argument");

		std::vector<esv::TurnManager::CombatTeam *> teams;
		std::unordered_set<eoc::CombatTeamId> notifies;

		lua_pushnil(L);
		if (index < 0) index--;

		while (lua_next(L, index) != 0) {
			auto luaTeam = TurnManagerTeamProxy::CheckUserData(L, -1);
			if (luaTeam->TeamId().CombatId != combatId) {
				luaL_error(L, "Tried to add team from another combat");
			}

			auto team = luaTeam->Get();
			if (team == nullptr) {
				luaL_error(L, "Tried to add nonexistent team");
			}

			teams.push_back(team);
			notifies.insert(team->TeamId);
			lua_pop(L, 1);
		}

		for (auto team : combatTeams) {
			notifies.insert(team->TeamId);
		}

		for (auto notify : combatNotifies) {
			notifies.insert(notify);
		}

		combatTeams.Size = 0;
		for (auto const & team : teams) {
			combatTeams.Add(team);
		}

		combatNotifies.Size = 0;
		for (auto const & teamId : notifies) {
			combatNotifies.Add(teamId);
		}

		auto protocol = GetTurnBasedProtocol();
		if (protocol != nullptr) {
			// FIXME - filter duplicates
			protocol->UpdatedTurnOrderCombatIds.Add(combatId);
		}
	}

	int TurnManagerCombatProxy::UpdateCurrentTurnOrder(lua_State * L)
	{
		auto self = TurnManagerCombatProxy::CheckUserData(L, 1);
		auto combat = self->Get();
		if (!combat) return 0;

		StackCheck _(L, 0);
		UpdateTurnOrder(L, self->combatId_, 2, combat->CurrentRoundTeams, combat->CurrentTurnChangeNotificationTeamIds);
		return 0;
	}

	int TurnManagerCombatProxy::UpdateNextTurnOrder(lua_State * L)
	{
		auto self = TurnManagerCombatProxy::CheckUserData(L, 1);
		auto combat = self->Get();
		if (!combat) return 0;

		StackCheck _(L, 0);
		UpdateTurnOrder(L, self->combatId_, 2, combat->NextRoundTeams, combat->NextTurnChangeNotificationTeamIds);
		return 0;
	}

	int TurnManagerCombatProxy::GetAllTeams(lua_State * L)
	{
		auto self = TurnManagerCombatProxy::CheckUserData(L, 1);
		auto combat = self->Get();
		if (!combat) return 0;

		StackCheck _(L, 1);
		lua_newtable(L);

		uint32_t i = 1;
		for (auto const& team : combat->Teams) {
			push(L, i++);
			TurnManagerTeamProxy::New(L, eoc::CombatTeamId(team.Key));
			lua_settable(L, -3);
		}

		return 1;
	}


	char const * const TurnManagerTeamProxy::MetatableName = "esv::TurnManager::Team";

	int TurnManagerTeamProxy::Index(lua_State * L)
	{
		auto team = Get();
		if (team == nullptr) return luaL_error(L, "Team no longer available");

		StackCheck _(L, 1);
		auto prop = luaL_checkstring(L, 2);
		if (strcmp(prop, "TeamId") == 0) {
			push(L, (uint32_t)team->TeamId);
		} else if (strcmp(prop, "CombatId") == 0) {
			push(L, team->TeamId.CombatId);
		} else if (strcmp(prop, "Initiative") == 0) {
			push(L, (uint32_t)team->Initiative);
		} else if (strcmp(prop, "StillInCombat") == 0) {
			push(L, team->StillInCombat);
			// TODO - fetching CombatGroup?
		} else if (strcmp(prop, "Character") == 0) {
			auto character = team->EntityWrapper.GetCharacter();
			MakeObjectRef(L, character);
		} else if (strcmp(prop, "Item") == 0) {
			auto item = team->EntityWrapper.GetItem();
			if (item != nullptr) {
				ComponentHandle handle;
				item->GetObjectHandle(handle);
				ObjectProxy<esv::Item>::New(L, handle);
			} else {
				push(L, nullptr);
			}
		} else {
			OsiError("Combat team has no attribute named " << prop);
			push(L, nullptr);
		}

		return 1;
	}


	char const* const ItemConstructor::MetatableName = "ItemConstructor";

	int ItemConstructorConstructItem(lua_State* L)
	{
		auto self = ItemConstructor::CheckUserData(L, 1);

		auto item = GetStaticSymbols().esv__CreateItemFromParsed(&self->Get(), 0);
		if (item == nullptr) {
			OsiErrorS("Failed to create parsed item.");
			return 0;
		}

		ObjectProxy<esv::Item>::New(L, GetServerLifetime(), item);
		return 1;
	}

	int ItemConstructor::Index(lua_State* L)
	{
		StackCheck _(L, 1);
		if (lua_type(L, 2) == LUA_TSTRING) {
			auto func = get<FixedString>(L, 2);
			if (func == GFS.strConstruct) {
				lua_pushcfunction(L, &ItemConstructorConstructItem);
				return 1;
			}

			OsiError("ItemConstructor has no property named " << func.Str);
			push(L, nullptr);
			return 1;
		} else {
			auto idx = get<int>(L, 2);
			if (idx < 1 || idx > (int)definition_.Size) {
				return luaL_error(L, "Clone set only has %d elements", definition_.Size);
			}

			ObjectProxy<eoc::ItemDefinition>::New(L, GetServerLifetime(), &definition_[idx - 1]);
			return 1;
		}
	}


	void ExtensionLibraryServer::Register(lua_State * L)
	{
		ExtensionLibrary::Register(L);

		ObjectProxy<esv::Status>::RegisterMetatable(L);
		ObjectProxy<esv::PlayerCustomData>::RegisterMetatable(L);
		ObjectProxy<esv::Item>::RegisterMetatable(L);
		ObjectProxy<eoc::ItemDefinition>::RegisterMetatable(L);
		ObjectProxy<esv::Projectile>::RegisterMetatable(L);
		ObjectProxy<esv::EsvTrigger>::RegisterMetatable(L);
		ObjectProxy<AtmosphereTriggerData>::RegisterMetatable(L);
		ObjectProxy<SoundVolumeTriggerData>::RegisterMetatable(L);
		ObjectProxy<esv::ShootProjectileHelper>::RegisterMetatable(L);
		ObjectProxy<esv::Surface>::RegisterMetatable(L);
		ObjectProxy<esv::SurfaceAction>::RegisterMetatable(L);

		OsiFunctionNameProxy::RegisterMetatable(L);
		StatusHandleProxy::RegisterMetatable(L);
		TurnManagerCombatProxy::RegisterMetatable(L);
		TurnManagerTeamProxy::RegisterMetatable(L);
		ItemConstructor::RegisterMetatable(L);
		RegisterNameResolverMetatable(L);
		CreateNameResolver(L);
	}
	
	esv::Character* GetCharacter(lua_State* L, int index)
	{
		esv::Character* character = nullptr;
		switch (lua_type(L, index)) {
		case LUA_TLIGHTUSERDATA:
		{
			auto handle = get<ComponentHandle>(L, index);
			if (handle.GetType() == (uint32_t)ObjectType::ClientCharacter) {
				OsiError("Attempted to resolve client ComponentHandle on the server");
			} else {
				character = GetEntityWorld()->GetCharacter(handle);
			}
			break;
		}

		case LUA_TNUMBER:
		{
			auto value = lua_tointeger(L, index);
			if (value > 0xffffffff) {
				OsiError("Resolving integer object handles is deprecated since v52!")
				ComponentHandle handle{ value };
				if (handle.GetType() == (uint32_t)ObjectType::ClientCharacter) {
					OsiError("Attempted to resolve client ComponentHandle on the server");
				} else {
					character = GetEntityWorld()->GetCharacter(handle);
				}
			} else {
				NetId netId{ (uint32_t)value };
				character = GetEntityWorld()->GetCharacter(netId);
			}
			break;
		}

		case LUA_TSTRING:
		{
			auto guid = lua_tostring(L, index);
			character = GetEntityWorld()->GetCharacter(guid);
			break;
		}

		default:
			OsiError("Expected character UUID, Handle or NetId; got " << lua_typename(L, lua_type(L, 1)));
			break;
		}

		return character;
	}

	int GetCharacter(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve character handle in restricted context");
		}

		StackCheck _(L, 1);
		esv::Character* character = GetCharacter(L, 1);
		MakeObjectRef(L, character);
		return 1;
	}

	int GetItem(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve item handle in restricted context");
		}

		StackCheck _(L, 1);
		esv::Item* item = nullptr;
		switch (lua_type(L, 1)) {
		case LUA_TLIGHTUSERDATA:
		{
			auto handle = get<ComponentHandle>(L, 1);
			item = GetEntityWorld()->GetItem(handle);
			break;
		}

		case LUA_TNUMBER:
		{
			auto value = lua_tointeger(L, 1);
			if (value > 0xffffffff) {
				OsiError("Resolving integer object handles is deprecated since v52!")
				ComponentHandle handle{ value };
				if (handle.GetType() == (uint32_t)ObjectType::ClientItem) {
					OsiError("Attempted to resolve client ComponentHandle on the server");
				} else {
					item = GetEntityWorld()->GetItem(handle);
				}
			} else {
				NetId netId{ (uint32_t)value };
				item = GetEntityWorld()->GetItem(netId);
			}
			break;
		}

		case LUA_TSTRING:
		{
			auto guid = lua_tostring(L, 1);
			item = GetEntityWorld()->GetItem(guid);
			break;
		}

		default:
			OsiError("Expected item GUID or handle, got " << lua_typename(L, lua_type(L, 1)));
			push(L, nullptr);
			return 1;
		}

		if (item != nullptr) {
			ComponentHandle handle;
			item->GetObjectHandle(handle);
			ObjectProxy<esv::Item>::New(L, handle);
		} else {
			push(L, nullptr);
		}

		return 1;
	}

	int GetTrigger(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve trigger handle in restricted context");
		}

		StackCheck _(L, 1);
		Trigger* trigger = nullptr;
		switch (lua_type(L, 1)) {
		case LUA_TLIGHTUSERDATA:
		{
			auto handle = get<ComponentHandle>(L, 1);
			trigger = GetEntityWorld()->GetTrigger(handle);
			break;
		}

		case LUA_TSTRING:
		{
			auto guid = lua_tostring(L, 1);
			trigger = GetEntityWorld()->GetTrigger(guid);
			break;
		}

		default:
			OsiError("Expected trigger GUID or handle, got " << lua_typename(L, lua_type(L, 1)));
			push(L, nullptr);
			return 1;
		}

		if (trigger != nullptr) {
			ComponentHandle handle;
			trigger->GetObjectHandle(handle);
			ObjectProxy<esv::EsvTrigger>::New(L, handle);
		} else {
			push(L, nullptr);
		}

		return 1;
	}

	struct GetGameObjectResult
	{
		esv::Item* item = nullptr;
		esv::Character* character = nullptr;
		esv::Projectile* projectile = nullptr;
		Trigger* trigger = nullptr;
	};

	GetGameObjectResult GetGameObjectInternal(lua_State* L, int idx)
	{
		GetGameObjectResult result;
		switch (lua_type(L, idx)) {
		case LUA_TLIGHTUSERDATA:
		{
			auto handle = get<ComponentHandle>(L, idx);
			if (handle) {
				switch ((ObjectType)handle.GetType()) {
				case ObjectType::ServerCharacter:
					result.character = GetEntityWorld()->GetCharacter(handle);
					break;

				case ObjectType::ServerItem:
					result.item = GetEntityWorld()->GetItem(handle);
					break;

				case ObjectType::ServerProjectile:
					result.projectile = GetEntityWorld()->GetProjectile(handle);
					break;

				case ObjectType::ServerEocPointTrigger:
				case ObjectType::ServerEocAreaTrigger:
				case ObjectType::ServerStartTrigger:
				case ObjectType::ServerTeleportTrigger:
				case ObjectType::ServerEventTrigger:
				case ObjectType::ServerCrimeAreaTrigger:
				case ObjectType::ServerCrimeRegionTrigger:
				case ObjectType::ServerAtmosphereTrigger:
				case ObjectType::ServerAIHintAreaTrigger:
				case ObjectType::ServerMusicVolumeTrigger:
				case ObjectType::ServerSecretRegionTrigger:
				case ObjectType::ServerStatsAreaTrigger:
				case ObjectType::ServerSoundVolumeTrigger:
				case ObjectType::ServerRegionTrigger:
				case ObjectType::ServerExplorationTrigger:
					result.trigger = GetEntityWorld()->GetTrigger(handle);
					break;

				default:
					OsiError("Cannot resolve unsupported server handle type: " << handle.GetType());
					break;
				}
			}

			break;
		}

		case LUA_TNUMBER:
		{
			OsiError("Resolving integer object handles is deprecated since v52!")
			auto handle = ComponentHandle(lua_tointeger(L, idx));
			if (handle) {
				switch ((ObjectType)handle.GetType()) {
				case ObjectType::ServerCharacter:
					result.character = GetEntityWorld()->GetCharacter(handle);
					break;

				case ObjectType::ServerItem:
					result.item = GetEntityWorld()->GetItem(handle);
					break;

				case ObjectType::ServerProjectile:
					result.projectile = GetEntityWorld()->GetProjectile(handle);
					break;

				default:
					OsiError("Cannot resolve unsupported server handle type: " << handle.GetType());
					break;
				}
			}

			break;
		}

		case LUA_TSTRING:
		{
			auto guid = lua_tostring(L, idx);
			result.character = GetEntityWorld()->GetCharacter(guid, false);
			result.item = GetEntityWorld()->GetItem(guid, false);
			result.trigger = GetEntityWorld()->GetTrigger(guid, false);
			break;
		}

		default:
			OsiError("Expected object GUID or handle, got " << lua_typename(L, lua_type(L, idx)));
			break;
		}

		return result;
	}

	int GetGameObject(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve game object handle in restricted context");
		}

		StackCheck _(L, 1);
		auto result = GetGameObjectInternal(L, 1);

		if (result.item != nullptr) {
			ComponentHandle handle;
			result.item->GetObjectHandle(handle);
			ObjectProxy<esv::Item>::New(L, handle);
			return 1;
		} else if (result.character != nullptr) {
			MakeObjectRef(L, result.character);
			return 1;
		} else if (result.projectile != nullptr) {
			ComponentHandle handle;
			result.projectile->GetObjectHandle(handle);
			ObjectProxy<esv::Projectile>::New(L, handle);
			return 1;
		} else if (result.trigger != nullptr) {
			ComponentHandle handle;
			result.trigger->GetObjectHandle(handle);
			ObjectProxy<esv::EsvTrigger>::New(L, handle);
			return 1;
		} else {
			push(L, nullptr);
			return 1;
		}
	}

	int GetStatus(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve status handle in restricted context");
		}

		esv::Character* character = GetCharacter(L, 1);
		if (character == nullptr) return 0;

		StackCheck _(L, 1);
		esv::Status* status;
		if (lua_type(L, 2) == LUA_TLIGHTUSERDATA) {
			auto statusHandle = get<ComponentHandle>(L, 2);
			status = character->GetStatus(statusHandle, true);
			if (status != nullptr) {
				ComponentHandle characterHandle;
				character->GetObjectHandle(characterHandle);
				StatusHandleProxy::New(L, characterHandle, statusHandle);
				return 1;
			}

			OsiError("Character has no status with ComponentHandle 0x" << std::hex << statusHandle.Handle);
		} else {
			auto index = lua_tointeger(L, 2);

			// We need to keep integer status handle support since some extender Osiris events
			// (eg. NRD_OnHit, NRD_OnPrepareHit, etc.) use these handles and Osiris doesn't support lightuserdata
			if (index > 0xffffffff) {
				ComponentHandle statusHandle{ index };
				status = character->GetStatus(statusHandle, true);
				if (status != nullptr) {
					ComponentHandle characterHandle;
					character->GetObjectHandle(characterHandle);
					StatusHandleProxy::New(L, characterHandle, statusHandle);
					return 1;
				}

				OsiError("Character has no status with ComponentHandle 0x" << std::hex << statusHandle.Handle);
			} else {
				NetId statusNetId{ (uint32_t)index };
				status = character->GetStatus(statusNetId);
				if (status != nullptr) {
					ComponentHandle characterHandle;
					character->GetObjectHandle(characterHandle);
					StatusHandleProxy::New(L, characterHandle, statusNetId);
					return 1;
				}

				OsiError("Character has no status with NetId 0x" << std::hex << index);
			}
		}

		push(L, nullptr);
		return 1;
	}

	int GetCombat(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve combat ID in restricted context");
		}

		auto turnMgr = GetEntityWorld()->GetTurnManager();
		if (turnMgr == nullptr) {
			OsiErrorS("esv::TurnManager not available");
			return 0;
		}

		auto combatId = (uint8_t)luaL_checkinteger(L, 1);
		auto combat = turnMgr->Combats.Find(combatId);
		if (combat == nullptr) {
			OsiError("No combat found with ID " << (unsigned)combatId);
			return 0;
		}

		TurnManagerCombatProxy::New(L, combatId);
		return 1;
	}

	int GetSurface(lua_State* L)
	{
		auto lua = State::FromLua(L);
		if (lua->RestrictionFlags & State::RestrictHandleConversion) {
			return luaL_error(L, "Attempted to resolve item handle in restricted context");
		}

		auto handle = get<ComponentHandle>(L, 1);

		auto level = GetStaticSymbols().GetCurrentServerLevel();
		if (!level || !level->SurfaceManager) {
			OsiError("Current level not available yet!");
			return 0;
		}

		auto surface = level->SurfaceManager->Get(handle);
		if (surface != nullptr) {
			ObjectProxy<esv::Surface>::New(L, handle);
			return 1;
		} else {
			return 0;
		}
	}

	int GetAiGrid(lua_State* L)
	{
		auto level = GetStaticSymbols().GetCurrentServerLevel();
		if (!level || !level->AiGrid) {
			OsiError("Current level not available yet!");
			return 0;
		}

		ObjectProxy<eoc::AiGrid>::New(L, GetServerLifetime(), level->AiGrid);
		return 1;
	}

	int GetCurrentLevelData(lua_State* L)
	{
		auto level = GetStaticSymbols().GetCurrentServerLevel();
		if (!level || !level->LevelDesc) {
			OsiError("Current level not available yet!");
			return 0;
		}

		lua_newtable(L);
		settable(L, "LevelName", level->LevelDesc->LevelName);
		settable(L, "UniqueKey", level->LevelDesc->UniqueKey);
		settable(L, "CustomDisplayLevelName", level->LevelDesc->CustomDisplayLevelName);
		settable(L, "Type", level->LevelDesc->Type);
		return 1;
	}

	int GetSurfaceTransformRules(lua_State* L)
	{
		auto rules = GetStaticSymbols().eoc__SurfaceTransformActionsFromType;
		if (rules == nullptr) {
			OsiError("Surface transform rules not available!");
			return 0;
		}

		StackCheck _(L, 1);
		lua_newtable(L);

		for (auto i = 0; i < std::size(rules->SurfaceTypes); i++) {
			auto const& interactionSet = rules->SurfaceTypes[i].Interactions;

			if ((uint64_t)interactionSet[0].ActionableSurfaces[0] == 0) continue;

			lua_newtable(L);

			auto interactionIdx = 1;
			for (auto const& interaction : interactionSet) {
				if ((uint64_t)interaction.ActionableSurfaces[0] == 0) break;

				push(L, interactionIdx++);
				lua_newtable(L);

				setfield(L, "TransformType", interaction.ActionType);

				lua_newtable(L);
				int surfIdx = 1;
				for (auto const& surface : interaction.ActionableSurfaces) {
					if ((uint64_t)surface == 0) break;

					push(L, surfIdx++);
					push_flags(L, surface);
					lua_settable(L, -3);
				}

				lua_setfield(L, -2, "ActionableSurfaces");

				lua_settable(L, -3);
			}

			lua_setfield(L, -2, EnumInfo<SurfaceType>::Find((SurfaceType)i).Str);
		}

		return 1;
	}

	void UpdateSurfaceTransformInteraction(lua_State* L, int idx, eoc::SurfaceTransformInteractions::Interaction& interaction)
	{
		luaL_checktype(L, idx, LUA_TTABLE);
		auto actionType = checked_getfield<SurfaceTransformActionType>(L, "TransformType", idx);
		interaction.ActionType = actionType;

		lua_getfield(L, idx, "ActionableSurfaces");
		luaL_checktype(L, -1, LUA_TTABLE);
		int curActionIdx = 0;
		for (auto surfaceIdx : iterate(L, -1)) {
			if (curActionIdx >= std::size(interaction.ActionableSurfaces) - 1) {
				luaL_error(L, "At most %d actionable surfaces per transform type are supported", std::size(interaction.ActionableSurfaces) - 1);
			}

			auto surfaceFlags = checked_get_flags<ESurfaceFlag>(L, surfaceIdx);
			interaction.ActionableSurfaces[curActionIdx++] = surfaceFlags;
		}

		lua_pop(L, 1);
	}

	int UpdateSurfaceTransformRules(lua_State* L)
	{
		StackCheck _(L, 0);
		auto rules = GetStaticSymbols().eoc__SurfaceTransformActionsFromType;
		if (rules == nullptr) {
			OsiError("Surface transform rules not available!");
			return 0;
		}

		auto interactions = std::make_unique<eoc::SurfaceTransformInteractions>();
		memset(interactions.get(), 0, sizeof(eoc::SurfaceTransformInteractions));

		luaL_checktype(L, 1, LUA_TTABLE);
		for (auto idx : iterate(L, 1)) {
			auto surfaceType = get<SurfaceType>(L, idx - 1);
			auto& surfaceRules = interactions->SurfaceTypes[(unsigned)surfaceType].Interactions;

			luaL_checktype(L, -1, LUA_TTABLE);
			int curInteractionIdx = 0;
			for (auto interactionIdx : iterate(L, -1)) {
				if (curInteractionIdx >= std::size(surfaceRules) - 1) {
					luaL_error(L, "At most %d rules per surface are supported", std::size(surfaceRules) - 1);
				}

				UpdateSurfaceTransformInteraction(L, interactionIdx, surfaceRules[curInteractionIdx++]);
			}

		}

		*rules = *interactions;
		return 0;
	}

	int CreateSurfaceAction(lua_State* L)
	{
		auto type = get<SurfaceActionType>(L, 1);

		auto const& sym = GetStaticSymbols();
		if (!sym.esv__SurfaceActionFactory || !*sym.esv__SurfaceActionFactory || !sym.esv__SurfaceActionFactory__CreateAction) {
			OsiError("esv::SurfaceActionFactory not mapped!");
			return 0;
		}

		auto action = sym.esv__SurfaceActionFactory__CreateAction(*sym.esv__SurfaceActionFactory, type, 0);
		if (!action) {
			OsiError("Couldn't create surface action for some reason.");
			return 0;
		}

		ObjectProxy<SurfaceAction>::New(L, GetServerLifetime(), action);
		return 1;
	}

	int ExecuteSurfaceAction(lua_State* L)
	{
		StackCheck _(L, 0);
		auto action = ObjectProxy<SurfaceAction>::CheckUserData(L, 1);
		if (!action->Get(L)) {
			OsiError("Attempted to execute surface action more than once!");
			return 0;
		}

		auto level = GetStaticSymbols().GetCurrentServerLevel();
		if (!level) {
			OsiError("Attempted to execute surface action while no level is loaded!");
			return 0;
		}

		auto surfaceAction = action->Get(L);
		surfaceAction->Level = level;

		if (surfaceAction->VMT->GetTypeId(surfaceAction) == SurfaceActionType::TransformSurfaceAction) {
			auto transform = reinterpret_cast<TransformSurfaceAction*>(surfaceAction);
			GetStaticSymbols().esv__TransformSurfaceAction__Init(
				transform, transform->SurfaceTransformAction, transform->SurfaceLayer, transform->OriginSurface
			);
		}

		surfaceAction->VMT->Enter(surfaceAction);
		level->SurfaceManager->SurfaceActions.Add(surfaceAction);
		action->Unbind();

		return 0;
	}

	int CancelSurfaceAction(lua_State* L)
	{
		StackCheck _(L, 0);
		auto handle = get<ComponentHandle>(L, 1);
		
		if (handle.GetType() != (uint32_t)ObjectType::ServerSurfaceAction) {
			OsiError("Expected a surface action handle, got type " << handle.GetType());
			return 0;
		}

		auto factory = GetStaticSymbols().esv__SurfaceActionFactory;
		if (!factory || !*factory) {
			OsiError("SurfaceActionFactory not mapped!");
			return 0;
		}

		// FIXME - TEMP CAST
		auto action = (esv::SurfaceAction*)(*factory)->Get(handle);
		if (!action) {
			OsiWarn("No surface action found with handle " << std::hex << handle.Handle << "; maybe it already expired?");
			return 0;
		}

		switch (action->VMT->GetTypeId(action)) {
		case SurfaceActionType::CreateSurfaceAction:
		{
			auto act = static_cast<esv::CreateSurfaceAction*>(action);
			act->CurrentCellCount = act->SurfaceCells.Size;
			break;
		}
		case SurfaceActionType::CreatePuddleAction:
		{
			auto act = static_cast<esv::CreatePuddleAction*>(action);
			act->IsFinished = true;
			break;
		}
		case SurfaceActionType::ExtinguishFireAction:
		{
			auto act = static_cast<esv::ExtinguishFireAction*>(action);
			act->Percentage = 0.0f;
			break;
		}
		case SurfaceActionType::ZoneAction:
		{
			auto act = static_cast<esv::ZoneAction*>(action);
			act->CurrentCellCount = act->SurfaceCells.Size;
			break;
		}
		case SurfaceActionType::ChangeSurfaceOnPathAction:
		{
			auto act = static_cast<esv::ChangeSurfaceOnPathAction*>(action);
			act->IsFinished = true;
			break;
		}
		case SurfaceActionType::RectangleSurfaceAction:
		{
			auto act = static_cast<esv::RectangleSurfaceAction*>(action);
			act->CurrentCellCount = act->SurfaceCells.Size;
			break;
		}
		case SurfaceActionType::PolygonSurfaceAction:
		{
			auto act = static_cast<esv::PolygonSurfaceAction*>(action);
			act->LastSurfaceCellCount = act->SurfaceCells.Size;
			break;
		}
		case SurfaceActionType::SwapSurfaceAction:
		{
			auto act = static_cast<esv::SwapSurfaceAction*>(action);
			act->CurrentCellCount = act->SurfaceCells.Size;
			break;
		}
		case SurfaceActionType::TransformSurfaceAction:
		{
			auto act = static_cast<esv::TransformSurfaceAction*>(action);
			act->Finished = true;
			break;
		}
		default:
			OsiError("CancelSurfaceAction() not implemented for this surface action type!");
		}

		return 0;
	}

	int ExecuteSkillPropertiesOnTarget(lua_State* L)
	{
		StackCheck _(L, 0);
		auto skillId = get<FixedString>(L, 1);
		auto attacker = GetCharacter(L, 2);
		auto target = GetCharacter(L, 3);
		auto position = get<glm::vec3>(L, 4);
		auto propertyContext = get<CRPGStats_Object_PropertyContext>(L, 5);
		auto isFromItem = get<bool>(L, 6);

		SkillPrototype* skillProto{ nullptr };
		auto skillProtoMgr = GetStaticSymbols().eoc__SkillPrototypeManager;
		if (skillProtoMgr && *skillProtoMgr) {
			auto proto = (*skillProtoMgr)->Prototypes.Find(skillId);
			if (*proto) {
				skillProto = *proto;
			}
		}

		if (!skillProto) {
			LuaError("Couldn't find skill prototype for " << skillId);
			return 0;
		}

		auto exec = GetStaticSymbols().esv__ExecuteCharacterSetExtraProperties;
		auto skillProperties = skillProto->GetStats()->PropertyList.Find(GFS.strSkillProperties);

		if (!skillProperties) {
			LuaError("Skill " << skillId << " has no SkillProperties!");
			return 0;
		}

		if (!attacker || !target || !exec) {
			return 0;
		}

		ComponentHandle attackerHandle;
		attacker->GetObjectHandle(attackerHandle);
		ObjectSet<esv::Character*> targets;
		targets.Add(target);

		exec(*skillProperties, attackerHandle.Handle, targets, position, propertyContext, isFromItem,
			skillProto, nullptr, 0.0f, nullptr, false, 2.4f);
		return 0;
	}

	int ExecuteSkillPropertiesOnPosition(lua_State* L)
	{
		StackCheck _(L, 0);
		auto skillId = get<FixedString>(L, 1);
		auto attacker = GetCharacter(L, 2);
		auto position = get<glm::vec3>(L, 3);
		auto radius = get<float>(L, 4);
		auto propertyContext = get<CRPGStats_Object_PropertyContext>(L, 5);
		auto isFromItem = get<bool>(L, 6);

		SkillPrototype* skillProto{ nullptr };
		auto skillProtoMgr = GetStaticSymbols().eoc__SkillPrototypeManager;
		if (skillProtoMgr && *skillProtoMgr) {
			auto proto = (*skillProtoMgr)->Prototypes.Find(skillId);
			if (*proto) {
				skillProto = *proto;
			}
		}

		if (!skillProto) {
			LuaError("Couldn't find skill prototype for " << skillId);
			return 0;
		}

		auto exec = GetStaticSymbols().esv__ExecutePropertyDataOnPositionOnly;
		auto skillProperties = skillProto->GetStats()->PropertyList.Find(GFS.strSkillProperties);

		if (!skillProperties) {
			LuaError("Skill " << skillId << " has no SkillProperties!");
			return 0;
		}

		if (!attacker || !exec) {
			return 0;
		}

		ComponentHandle attackerHandle;
		attacker->GetObjectHandle(attackerHandle);

		exec(*skillProperties, attackerHandle.Handle, &position, radius, propertyContext, isFromItem, skillProto, nullptr, 2.4f);
		return 0;
	}

	int OsirisIsCallable(lua_State* L)
	{
		StackCheck _(L, 1);
		auto lua = State::FromLua(L);
		bool allowed = gExtender->GetServer().Osiris().IsStoryLoaded()
			&& ((lua->RestrictionFlags & State::RestrictOsiris) == 0);
		push(L, allowed);
		return 1;
	}

	int BroadcastMessage(lua_State * L)
	{
		StackCheck _(L, 0);
		auto channel = luaL_checkstring(L, 1);
		auto payload = luaL_checkstring(L, 2);

		esv::Character * excludeCharacter = nullptr;
		if (lua_gettop(L) > 2 && !lua_isnil(L, 3)) {
			// TODO - get<GUIDString> alias?
			auto excludeCharacterGuid = luaL_checkstring(L, 3);
			excludeCharacter = GetEntityWorld()->GetCharacter(excludeCharacterGuid);
			if (excludeCharacter == nullptr) return 0;
		}

		auto & networkMgr = gExtender->GetServer().GetNetworkManager();
		auto msg = networkMgr.GetFreeMessage(ReservedUserId);
		if (msg != nullptr) {
			auto postMsg = msg->GetMessage().mutable_post_lua();
			postMsg->set_channel_name(channel);
			postMsg->set_payload(payload);
			if (excludeCharacter != nullptr) {
				networkMgr.Broadcast(msg, excludeCharacter->UserID);
			} else {
				networkMgr.Broadcast(msg, ReservedUserId);
			}
		} else {
			OsiErrorS("Could not get free message!");
		}

		return 0;
	}

	void PostMessageToUserInternal(UserId userId, char const* channel, char const* payload)
	{
		auto& networkMgr = gExtender->GetServer().GetNetworkManager();
		auto msg = networkMgr.GetFreeMessage(userId);
		if (msg != nullptr) {
			auto postMsg = msg->GetMessage().mutable_post_lua();
			postMsg->set_channel_name(channel);
			postMsg->set_payload(payload);
			networkMgr.Send(msg, userId);
		}
		else {
			OsiErrorS("Could not get free message!");
		}
	}

	int PostMessageToClient(lua_State * L)
	{
		StackCheck _(L, 0);
		auto characterGuid = luaL_checkstring(L, 1);
		auto channel = luaL_checkstring(L, 2);
		auto payload = luaL_checkstring(L, 3);

		auto character = GetEntityWorld()->GetCharacter(characterGuid);
		if (character == nullptr) return 0;

		if (character->UserID == ReservedUserId) {
			OsiError("Attempted to send message to character " << characterGuid << " that has no user assigned!");
			return 0;
		}

		PostMessageToUserInternal(character->UserID, channel, payload);
		return 0;
	}

	int PostMessageToUser(lua_State* L)
	{
		StackCheck _(L, 0);
		auto userId = get<int>(L, 1);
		auto channel = luaL_checkstring(L, 2);
		auto payload = luaL_checkstring(L, 3);

		if (UserId(userId) == ReservedUserId) {
			OsiError("Attempted to send message to reserved user ID!");
			return 0;
		}

		PostMessageToUserInternal(UserId(userId), channel, payload);
		return 0;
	}

	int PlayerHasExtender(lua_State* L)
	{
		auto characterGuid = luaL_checkstring(L, 1);

		auto character = GetEntityWorld()->GetCharacter(characterGuid);
		if (character == nullptr || character->UserID == ReservedUserId) return 0;

		// FIXME - access server from Lua context!
		auto& networkMgr = gExtender->GetServer().GetNetworkManager();
		bool hasExtender = networkMgr.CanSendExtenderMessages(character->UserID.GetPeerId());
		push(L, hasExtender);
		return 1;
	}

	int GetGameState(lua_State* L)
	{
		auto state = GetStaticSymbols().GetServerState();
		if (state) {
			push(L, *state);
		} else {
			lua_pushnil(L);
		}

		return 1;
	}

	int DumpNetworking(lua_State* L)
	{
		auto server = (*GetStaticSymbols().esv__EoCServer)->GameServer;
		auto loadProto = (esv::LoadProtocol*)server->Protocols[4];

		INFO(" === NETWORK DUMP === ");
		INFO("LoadProtocol state: %d", loadProto->State);
		INFO("Received LEVEL_START: %d", loadProto->NumLevelStartPlayers);
		INFO("Received SWAP_READY: %d", loadProto->NumSwapReadyPeers);

		INFO("ModuleLoaded responses:");
		for (auto const& load : loadProto->OS_ModuleLoaded) {
			INFO("    Peer %d", load.PeerId);
		}

		INFO("SessionLoaded responses:");
		for (auto const& load : loadProto->OS_SessionLoaded) {
			INFO("    Peer %d (%s)", load.PeerId, load.field_8.Str);
		}

		INFO("LevelLoaded responses:");
		for (auto const& load : loadProto->OS_LevelLoaded) {
			INFO("    Peer %d (%s)", load.PeerId, load.field_8.Str);
		}

		INFO("FileTransferOutbox items:");
		for (auto const& item : loadProto->OS_FileTransferOutbox) {
			INFO("    Peer %d, user %d", item.PeerId, item.UserId_M);
			for (auto const& path : item.OS_FixedString) {
				INFO("        FS %s", path.Str);
			}
			for (auto const& path : item.OS_Path) {
				INFO("        Path %s", path.Name.c_str());
			}
		}

		return 0;
	}

	int RegisterOsirisListener(lua_State* L)
	{
		StackCheck _(L, 0);
		auto name = get<char const*>(L, 1);
		auto arity = get<int>(L, 2);
		auto typeName = get<char const*>(L, 3);
		luaL_checktype(L, 4, LUA_TFUNCTION);

		OsirisHookSignature::HookType type;
		if (strcmp(typeName, "before") == 0) {
			type = OsirisHookSignature::BeforeTrigger;
		} else if (strcmp(typeName, "after") == 0) {
			type = OsirisHookSignature::AfterTrigger;
		} else if (strcmp(typeName, "beforeDelete") == 0) {
			type = OsirisHookSignature::BeforeDeleteTrigger;
		} else if (strcmp(typeName, "afterDelete") == 0) {
			type = OsirisHookSignature::AfterDeleteTrigger;
		} else {
			return luaL_error(L, "Hook type must be 'before', 'beforeDelete', 'after' or 'afterDelete'");
		}

		LuaServerPin lua(ExtensionState::Get());
		RegistryEntry handler(L, 4);
		lua->GetOsirisCallbacks().Subscribe(name, arity, type, std::move(handler));
		return 0;
	}


	int CreateItemConstructor(lua_State* L)
	{
		StackCheck _(L, 1);
		auto type = lua_type(L, 1);
		if (type == LUA_TSTRING) {
			auto templateGuid = get<char const*>(L, 1);
			auto constructor = ItemConstructor::New(L);
			if (!script::CreateItemDefinition(templateGuid, constructor->Get())) {
				lua_pop(L, 1);
				push(L, nullptr);
			}
		} else {
			auto item = ObjectProxy<esv::Item>::CheckedGet(L, 1);
			bool recursive{ false };
			if (lua_gettop(L) > 1) {
				recursive = get<bool>(L, 2);
			}

			auto constructor = ItemConstructor::New(L);
			if (!script::ParseItem(item, constructor->Get(), recursive)) {
				lua_pop(L, 1);
				push(L, nullptr);
			}
		}

		return 1;
	}

	int PrepareStatus(lua_State* L)
	{
		StackCheck _(L, 1);

		auto gameObj = GetGameObjectInternal(L, 1);
		auto statusId = get<FixedString>(L, 2);
		float lifeTime = get<float>(L, 3);

		StatusMachine* statusMachine{ nullptr };
		ComponentHandle ownerHandle;
		if (gameObj.character != nullptr) {
			statusMachine = gameObj.character->StatusMachine;
			gameObj.character->GetObjectHandle(ownerHandle);
		} else if (gameObj.item != nullptr) {
			statusMachine = gameObj.item->StatusMachine;
			gameObj.item->GetObjectHandle(ownerHandle);
		} else if (gameObj.trigger != nullptr || gameObj.projectile != nullptr) {
			OsiError("Attempted to send message to reserved user ID!");
			push(L, nullptr);
			return 1;
		}

		auto status = gExtender->GetStatusHelpers().PrepareStatus(statusMachine, statusId, lifeTime);
		if (!status) {
			push(L, nullptr);
			return 1;
		}

		StatusHandleProxy::New(L, ownerHandle, status->StatusHandle, true);
		return 1;
	}

	int ApplyStatus(lua_State* L)
	{
		auto status = StatusHandleProxy::CheckUserData(L, 1);
		StatusMachine* statusMachine{ nullptr };

		auto ownerHandle = status->OwnerHandle();
		if (ownerHandle.GetType() == (uint32_t)ObjectType::ServerCharacter) {
			auto character = GetEntityWorld()->GetCharacter(ownerHandle);
			if (character) {
				statusMachine = character->StatusMachine;
			}
		} else if (ownerHandle.GetType() == (uint32_t)ObjectType::ServerItem) {
			auto item = GetEntityWorld()->GetItem(ownerHandle);
			if (item) {
				statusMachine = item->StatusMachine;
			}
		}

		if (statusMachine == nullptr) {
			OsiError("No StatusMachine found for this status!");
			return 0;
		}

		auto applyStatus = GetStaticSymbols().esv__StatusMachine__ApplyStatus;
		if (applyStatus == nullptr) {
			OsiErrorS("esv::StatusMachine::ApplyStatus not found!");
			return 0;
		}

		// FIXME - TEMP CAST
		auto statusObj = (esv::Status*)statusMachine->Get(status->StatusHandle());
		if (!statusObj) {
			OsiError("No status found with this handle!");
			return 0;
		}

		if ((statusObj->Flags2 & StatusFlags2::Started) == StatusFlags2::Started) {
			OsiError("Trying to apply status that was already started!");
			return 0;
		}

		applyStatus(statusMachine, statusObj);
		return 0;
	}

	int CreateCustomStat(lua_State* L)
	{
		StackCheck _(L, 1);
		auto statName = get<char const*>(L, 1);
		auto statDescription = get<char const*>(L, 2);

		auto statsId = CustomStatHelpers::CreateStat(statName, statDescription);
		if (statsId) {
			push(L, *statsId);
		} else {
			push(L, nullptr);
		}

		return 1;
	}

	int GetCustomStatById(lua_State* L)
	{
		StackCheck _(L, 1);
		auto statId = get<char const*>(L, 1);

		auto statDefn = CustomStatHelpers::FindStatDefinitionById(statId);
		if (statDefn) {
			lua_newtable(L);
			settable(L, "Id", statDefn->Id);
			settable(L, "Name", statDefn->Name);
			settable(L, "Description", statDefn->Description);
		}
		else {
			push(L, nullptr);
		}

		return 1;
	}

	int GetAllCustomStats(lua_State* L)
	{
		StackCheck _(L, 1);

		auto entityWorld = GetEntityWorld();
		auto statSystem = entityWorld->GetCustomStatSystem();

		lua_newtable(L);
		int idx{ 1 };
		for (auto const& defn : statSystem->CreatedDefinitions) {
			auto statDefn = entityWorld->GetCustomStatDefinitionComponent(defn.Handle);
			if (statDefn != nullptr) {
				settable(L, idx++, statDefn->Id);
			}
		}

		for (auto const& defn : statSystem->InSyncDefinitions) {
			auto statDefn = entityWorld->GetCustomStatDefinitionComponent(defn.Handle);
			if (statDefn != nullptr) {
				settable(L, idx++, statDefn->Id);
			}
		}

		return 1;
	}

	int GetCustomStatByName(lua_State* L)
	{
		StackCheck _(L, 1);
		auto statName = get<char const*>(L, 1);

		auto statDefn = CustomStatHelpers::FindStatDefinitionByName(statName);
		if (statDefn) {
			lua_newtable(L);
			settable(L, "Id", statDefn->Id);
			settable(L, "Name", statDefn->Name);
			settable(L, "Description", statDefn->Description);
		}
		else {
			push(L, nullptr);
		}

		return 1;
	}


	void ExtensionLibraryServer::RegisterLib(lua_State * L)
	{
		static const luaL_Reg extLib[] = {
			{"Version", GetExtensionVersionWrapper},
			{"GameVersion", GetGameVersionWrapper},
			{"MonotonicTime", MonotonicTimeWrapper},
			{"Include", Include},
			{"NewCall", NewCall},
			{"NewQuery", NewQuery},
			{"NewEvent", NewEvent},
			{"Print", OsiPrint},
			{"PrintWarning", OsiPrintWarning},
			{"PrintError", OsiPrintError},
			{"HandleToDouble", HandleToDoubleWrapper},
			{"DoubleToHandle", DoubleToHandleWrapper},
			{"GetHandleType", GetHandleTypeWrapper},

			{"SaveFile", SaveFileWrapper},
			{"LoadFile", LoadFileWrapper},

			{"IsModLoaded", IsModLoadedWrapper},
			{"GetModLoadOrder", GetModLoadOrder},
			{"GetModInfo", GetModInfo},

			{"DebugBreak", LuaDebugBreakWrapper},

			{"GetStatEntries", GetStatEntries},
			{"GetStatEntriesLoadedBefore", GetStatEntriesLoadedBefore},
			{"GetSkillSet", GetSkillSet},
			{"UpdateSkillSet", UpdateSkillSet},
			{"GetEquipmentSet", GetEquipmentSet},
			{"UpdateEquipmentSet", UpdateEquipmentSet},
			{"GetTreasureTable", GetTreasureTable},
			{"UpdateTreasureTable", UpdateTreasureTable},
			{"GetTreasureCategory", GetTreasureCategory},
			{"UpdateTreasureCategory", UpdateTreasureCategory},
			{"GetItemCombo", GetItemCombo},
			{"UpdateItemCombo", UpdateItemCombo},
			{"GetItemComboPreviewData", GetItemComboPreviewData},
			{"UpdateItemComboPreviewData", UpdateItemComboPreviewData},
			{"GetItemComboProperty", GetItemComboProperty},
			{"UpdateItemComboProperty", UpdateItemComboProperty},
			{"GetItemGroup", GetItemGroup},
			{"GetNameGroup", GetNameGroup},

			{"StatGetAttribute", StatGetAttribute},
			{"StatSetAttribute", StatSetAttribute},
			{"StatAddCustomDescription", StatAddCustomDescriptionWrapper},
			{"GetStat", GetStat},
			{"CreateStat", CreateStat},
			{"SyncStat", SyncStatWrapper},
			{"StatSetPersistence", StatSetPersistenceWrapper},
			{"StatSetPersistence", StatSetPersistenceWrapper},
			{"GetDeltaMod", GetDeltaMod},
			{"UpdateDeltaMod", UpdateDeltaMod},
			{"EnumIndexToLabel", EnumIndexToLabel},
			{"EnumLabelToIndex", EnumLabelToIndex},
			{"ExecuteSkillPropertiesOnTarget", ExecuteSkillPropertiesOnTarget},
			{"ExecuteSkillPropertiesOnPosition", ExecuteSkillPropertiesOnPosition},

			{"GetSurfaceTransformRules", GetSurfaceTransformRules},
			{"UpdateSurfaceTransformRules", UpdateSurfaceTransformRules},
			{"CreateSurfaceAction", CreateSurfaceAction},
			{"ExecuteSurfaceAction", ExecuteSurfaceAction},
			{"CancelSurfaceAction", CancelSurfaceAction},

			{"GetAllCharacters", GetAllCharacters},
			{"GetCharactersAroundPosition", GetCharactersAroundPosition},
			{"GetAllItems", GetAllItems},
			{"GetItemsAroundPosition", GetItemsAroundPosition},
			{"CreateItemConstructor", CreateItemConstructor},
			{"GetAllTriggers", GetAllTriggers},

			{"PrepareStatus", PrepareStatus},
			{"ApplyStatus", ApplyStatus},

			{"GetAllCustomStats", GetAllCustomStats},
			{"GetCustomStatByName", GetCustomStatByName},
			{"GetCustomStatById", GetCustomStatById},
			{"CreateCustomStat", CreateCustomStat},

			{"GetCharacter", GetCharacter},
			{"GetItem", GetItem},
			{"GetTrigger", GetTrigger},
			{"GetGameObject", GetGameObject},
			{"GetStatus", GetStatus},
			{"GetCombat", GetCombat},
			{"GetSurface", GetSurface},
			{"GetAiGrid", GetAiGrid},
			{"GetCurrentLevelData", GetCurrentLevelData},
			{"NewDamageList", NewDamageList},
			{"GetSurfaceTemplate", GetSurfaceTemplate},
			{"OsirisIsCallable", OsirisIsCallable},
			{"IsDeveloperMode", IsDeveloperModeWrapper},
			{"GetGameMode", GetGameModeWrapper},
			{"GetDifficulty", GetDifficultyWrapper},
			{"Random", LuaRandom},
			{"Round", LuaRoundWrapper},
			{"GenerateIdeHelpers", GenerateIdeHelpersWrapper},

			// EXPERIMENTAL FUNCTIONS
			{"EnableExperimentalPropertyWrites", EnableExperimentalPropertyWritesWrapper},
			{"DumpStack", DumpStackWrapper},
			{"DumpNetworking", DumpNetworking},
			{"ShowErrorAndExitGame", ShowErrorAndExitGameWrapper},

			{"GetGameState", GetGameState},
			{"AddPathOverride", AddPathOverrideWrapper},
			{"AddVoiceMetaData", AddVoiceMetaDataWrapper},
			{"GetTranslatedString", GetTranslatedStringWrapper},
			{"GetTranslatedStringFromKey", GetTranslatedStringFromKeyWrapper},
			{"CreateTranslatedString", CreateTranslatedStringWrapper},
			{"CreateTranslatedStringKey", CreateTranslatedStringKeyWrapper},
			{"CreateTranslatedStringHandle", CreateTranslatedStringHandleWrapper},

			{"BroadcastMessage", BroadcastMessage},
			{"PostMessageToClient", PostMessageToClient},
			{"PostMessageToUser", PostMessageToUser},
			{"PlayerHasExtender", PlayerHasExtender},

			{"RegisterOsirisListener", RegisterOsirisListener},
			{0,0}
		};

		luaL_newlib(L, extLib); // stack: lib
		lua_setglobal(L, "Ext"); // stack: -

		RegisterSharedLibraries(L);
	}



	ServerState::ServerState(ExtensionState& state)
		: osiris_(state)
	{
		StackCheck _(L, 0);

		library_.Register(L);

		auto baseLib = GetBuiltinLibrary(IDR_LUA_BUILTIN_LIBRARY);
		LoadScript(baseLib, "BuiltinLibrary.lua");
		auto eventLib = GetBuiltinLibrary(IDR_LUA_EVENT);
		LoadScript(eventLib, "Event.lua");
		auto serverLib = GetBuiltinLibrary(IDR_LUA_BUILTIN_LIBRARY_SERVER);
		LoadScript(serverLib, "BuiltinLibraryServer.lua");
		auto gameMathLib = GetBuiltinLibrary(IDR_LUA_GAME_MATH);
		LoadScript(gameMathLib, "Game.Math.lua");
		auto gameTooltipLib = GetBuiltinLibrary(IDR_LUA_GAME_TOOLTIP);
		LoadScript(gameTooltipLib, "Game.Tooltip.lua");

		lua_getglobal(L, "Ext"); // stack: Ext
		StatsExtraDataProxy::New(L); // stack: Ext, "ExtraData", ExtraDataProxy
		lua_setfield(L, -2, "ExtraData"); // stack: Ext
		lua_pop(L, 1); // stack: -

		// Ext is not writeable after loading SandboxStartup!
		auto sandbox = GetBuiltinLibrary(IDR_LUA_SANDBOX_STARTUP);
		LoadScript(sandbox, "SandboxStartup.lua");

#if !defined(OSI_NO_DEBUGGER)
		auto debugger = gExtender->GetLuaDebugger();
		if (debugger) {
			debugger->ServerStateCreated(this);
		}
#endif
	}

	ServerState::~ServerState()
	{
		if (gExtender) {
#if !defined(OSI_NO_DEBUGGER)
			auto debugger = gExtender->GetLuaDebugger();
			if (debugger) {
				debugger->ServerStateDeleted();
			}
#endif

			// FIXME - HANDLE IN SERVER LOGIC!
			gExtender->GetServer().Osiris().GetCustomFunctionManager().ClearDynamicEntries();
		}
	}


	std::optional<int32_t> ServerState::StatusGetEnterChance(esv::Status* status, bool isEnterCheck)
	{
		StatusGetEnterChanceEventParams params{ status, isEnterCheck };
		ThrowEvent(*this, "StatusGetEnterChance", params);
		return params.EnterChance;
	}

	void PushPendingHit(lua_State* L, PendingHit const& hit)
	{
		lua_newtable(L);
		setfield(L, "HitId", hit.Id);

		if (hit.CapturedCharacterHit) {
			ObjectProxy<CDivinityStats_Item>::New(L, GetServerLifetime(), hit.WeaponStats);
			lua_setfield(L, -2, "Weapon");
			PushHit(L, hit.CharacterHit);
			lua_setfield(L, -2, "Hit");
			setfield(L, "HitType", hit.HitType);
			setfield(L, "NoHitRoll", hit.NoHitRoll);
			setfield(L, "ProcWindWalker", hit.ProcWindWalker);
			setfield(L, "ForceReduceDurability", hit.ForceReduceDurability);
			setfield(L, "HighGround", hit.HighGround);
			setfield(L, "CriticalRoll", hit.CriticalRoll);
		}

		if (hit.Status) {
			MakeObjectRef(L, hit.Status);
			lua_setfield(L, -2, "HitStatus");
		}
	}

	void ServerState::OnStatusHitEnter(esv::StatusHit* hit, PendingHit* context)
	{
		StatusHitEnterEventParams params{ hit, context };
		ThrowEvent(*this, "StatusHitEnter", params);
	}

	bool ServerState::ComputeCharacterHit(CDivinityStats_Character * target,
		CDivinityStats_Character *attacker, CDivinityStats_Item *weapon, DamagePairList *damageList,
		HitType hitType, bool noHitRoll, bool forceReduceDurability, HitDamageInfo *hit,
		CRPGStats_Object_Property_List *skillProperties, HighGroundBonus highGroundFlag, CriticalRoll criticalRoll)
	{
		StackCheck _(L, 0);
		Restriction restriction(*this, RestrictOsiris);

		PushExtFunction(L, "_ComputeCharacterHit"); // stack: fn

		MakeObjectRef(L, target);
		ItemOrCharacterPushPin luaAttacker(L, attacker);

		ObjectProxy<CDivinityStats_Item> * luaWeapon = nullptr;
		if (weapon != nullptr) {
			luaWeapon = ObjectProxy<CDivinityStats_Item>::New(L, GetServerLifetime(), weapon);
		} else {
			lua_pushnil(L);
		}
		UnbindablePin _2(luaWeapon);

		auto luaDamageList = DamageList::New(L);
		for (auto const& dmg : *damageList) {
			luaDamageList->Get().SafeAdd(dmg);
		}

		push(L, hitType);
		push(L, noHitRoll);
		push(L, forceReduceDurability);

		PushHit(L, *hit);

		auto alwaysBackstab = skillProperties != nullptr
			&& skillProperties->Properties.Find(FixedString("AlwaysBackstab")) != nullptr;
		push(L, alwaysBackstab);

		push(L, highGroundFlag);
		push(L, criticalRoll);

		if (CallWithTraceback(L, 11, 1) != 0) { // stack: succeeded
			LuaError("ComputeCharacterHit handler failed: " << lua_tostring(L, -1));
			lua_pop(L, 1);
			return false;
		}

		int isnil = lua_isnil(L, -1);

		bool ok;
		if (isnil) {
			ok = false;
		} else if (lua_type(L, -1) == LUA_TTABLE) {
			lua_getfield(L, -1, "EffectFlags");
			auto effectFlags = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "TotalDamageDone");
			auto totalDamageDone = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "ArmorAbsorption");
			auto armorAbsorption = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "LifeSteal");
			auto lifeSteal = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_getfield(L, -1, "DamageList");
			auto damageList = DamageList::AsUserData(L, -1);
			lua_pop(L, 1);

			if (damageList == nullptr) {
				OsiErrorS("Missing 'DamageList' in table returned from ComputeCharacterHit");
				ok = false;
			} else {
				hit->EffectFlags = (HitFlag)effectFlags;
				hit->TotalDamage = (int32_t)totalDamageDone;
				hit->ArmorAbsorption = (int32_t)armorAbsorption;
				hit->LifeSteal = (int32_t)lifeSteal;
				hit->DamageList.Clear();
				for (auto const& dmg : damageList->Get()) {
					hit->DamageList.AddDamage(dmg.DamageType, dmg.Amount);
				}
				ok = true;
			}
		} else {
			OsiErrorS("ComputeCharacterHit must return a table");
			ok = false;
		}

		lua_pop(L, 1); // stack: -
		return ok;
	}

	bool ServerState::OnCharacterApplyDamage(esv::Character* target, HitDamageInfo& hit, ComponentHandle attackerHandle,
			CauseType causeType, glm::vec3& impactDirection, PendingHit* context)
	{
		CRPGStats_Object* attacker{ nullptr };
		if (attackerHandle) {
			auto attackerChar = GetEntityWorld()->GetCharacter(attackerHandle, false);
			if (attackerChar) {
				attacker = attackerChar->Stats;
			} else {
				auto attackerItem = GetEntityWorld()->GetItem(attackerHandle, false);
				if (attackerItem) {
					attacker = attackerItem->Stats;
				} else {
					OsiError("Could not resolve attacker handle: " << std::hex << attackerHandle.Handle);
				}
			}
		}

		BeforeCharacterApplyDamageEventParams evt{
			target, attacker, &hit, causeType, impactDirection, context
		};
		ThrowEvent(*this, "BeforeCharacterApplyDamage", evt);
		return evt.Handled;
	}


	void ServerState::OnGameStateChanged(GameState fromState, GameState toState)
	{
		GameStateChangeEventParams params{ fromState, toState };
		ThrowEvent(*this, "GameStateChanged", params);
	}


	esv::Item* ServerState::OnGenerateTreasureItem(esv::Item* item)
	{
		StackCheck _(L, 0);
		PushExtFunction(L, "_TreasureItemGenerated"); // stack: fn

		TreasureItemGeneratedEventParams params{ item, nullptr };
		ThrowEvent(*this, "TreasureItemGenerated", params);

		if (!params.ResultingItem) {
			return nullptr;
		}

		if (params.ResultingItem->ParentInventoryHandle) {
			OsiError("TreasureItemGenerated must return an item that's not already in an inventory");
			return item;
		}

		if (params.ResultingItem->CurrentLevel && params.ResultingItem->CurrentLevel.GetString()[0]) {
			OsiError("TreasureItemGenerated must return an item that's not in the level");
			return item;
		}

		return params.ResultingItem;
	}


	bool ServerState::OnBeforeCraftingExecuteCombination(CraftingStationType craftingStation, ObjectSet<ComponentHandle> const& ingredients,
		esv::Character* character, uint8_t quantity, FixedString const& combinationId)
	{
		BeforeCraftingExecuteCombinationEventParams params{ character, craftingStation, combinationId, quantity };

		for (auto ingredientHandle : ingredients) {
			auto ingredient = GetEntityWorld()->GetItem(ingredientHandle);
			if (ingredient) {
				params.Items.Add(ingredient);
			}
		}

		ThrowEvent(*this, "BeforeCraftingExecuteCombination", params);

		return params.Processed;
	}


	void ServerState::OnAfterCraftingExecuteCombination(CraftingStationType craftingStation, ObjectSet<ComponentHandle> const& ingredients,
		esv::Character* character, uint8_t quantity, FixedString const& combinationId, bool succeeded)
	{
		AfterCraftingExecuteCombinationEventParams params{ character, craftingStation, combinationId, quantity, succeeded };

		for (auto ingredientHandle : ingredients) {
			auto ingredient = GetEntityWorld()->GetItem(ingredientHandle);
			if (ingredient) {
				params.Items.Add(ingredient);
			}
		}

		ThrowEvent(*this, "AfterCraftingExecuteCombination", params);
	}


	void ServerState::OnBeforeShootProjectile(ShootProjectileHelper* helper)
	{
		BeforeShootProjectileEventParams params{ helper };
		ThrowEvent(*this, "BeforeShootProjectile", params);
	}


	void ServerState::OnShootProjectile(Projectile* projectile)
	{
		ShootProjectileEventParams params{ projectile };
		ThrowEvent(*this, "OnShootProjectile", params);
	}

	void PushGameObject(lua_State* L, ComponentHandle handle)
	{
		if (!handle) {
			push(L, nullptr);
			return;
		}

		switch (handle.GetType()) {
		case (uint32_t)ObjectType::ServerCharacter:
		{
			auto character = esv::GetEntityWorld()->GetCharacter(handle);
			MakeObjectRef(L, character);
			break;
		}

		case (uint32_t)ObjectType::ServerItem:
			ObjectProxy<esv::Item>::New(L, handle);
			break;

		case (uint32_t)ObjectType::ServerProjectile:
			ObjectProxy<esv::Projectile>::New(L, handle);
			break;

		default:
			push(L, nullptr);
			LuaError("Don't know how to push handle of type " << handle.GetType());
			break;
		}
	}


	void ServerState::OnProjectileHit(Projectile* projectile, ComponentHandle const& hitObject, glm::vec3 const& position)
	{
		ProjectileHitEventParams params{ projectile, hitObject, position };
		ThrowEvent(*this, "OnProjectileHit", params);
	}


	void ServerState::OnExecutePropertyDataOnGroundHit(glm::vec3& position, ComponentHandle casterHandle, DamagePairList* damageList)
	{
		ExecutePropertyDataOnGroundHitEventParams params{ position, casterHandle, damageList };
		ThrowEvent(*this, "OnExecutePropertyDataOnGroundHit", params);
	}


	void ServerState::ExecutePropertyDataOnTarget(CRPGStats_Object_Property_Extender* prop, ComponentHandle attackerHandle,
		ComponentHandle target, glm::vec3 const& impactOrigin, bool isFromItem, SkillPrototype * skillProto,
		HitDamageInfo const* damageInfo)
	{
		ExecutePropertyDataOnTargetEventParams params{ 
			prop, attackerHandle, target, impactOrigin, isFromItem, skillProto, damageInfo
		};
		ThrowEvent(*this, "OnExecutePropertyDataOnTarget", params);
	}


	void ServerState::ExecutePropertyDataOnPosition(CRPGStats_Object_Property_Extender* prop, ComponentHandle attackerHandle,
		glm::vec3 const& position, float areaRadius, bool isFromItem, SkillPrototype * skillPrototype,
		HitDamageInfo const* damageInfo)
	{
		ExecutePropertyDataOnPositionEventParams params{
			prop, attackerHandle, position, areaRadius, isFromItem, skillPrototype, damageInfo
		};
		ThrowEvent(*this, "OnExecutePropertyDataOnPosition", params);
	}


	bool ServerState::OnUpdateTurnOrder(esv::TurnManager * self, uint8_t combatId)
	{
		StackCheck _(L, 0);
		Restriction restriction(*this, RestrictOsiris);

		auto turnMgr = GetEntityWorld()->GetTurnManager();
		if (!turnMgr) {
			OsiErrorS("Couldn't fetch turn manager");
			return false;
		}

		auto combat = turnMgr->Combats.Find(combatId);
		if (combat == nullptr) {
			OsiError("No combat found with ID " << (unsigned)combatId);
			return false;
		}

		PushExtFunction(L, "_CalculateTurnOrder"); // stack: fn

		TurnManagerCombatProxy::New(L, combatId); // stack: fn, combat
		CombatTeamListToLua(L, combat->NextRoundTeams);

		if (CallWithTraceback(L, 2, 1) != 0) { // stack: retval
			LuaError("OnUpdateTurnOrder handler failed: " << lua_tostring(L, -1));
			lua_pop(L, 1);
			return false;
		}

		int isnil = lua_isnil(L, -1);

		bool ok = false;
		if (!isnil) {
			try {
				UpdateTurnOrder(L, combatId, -1, combat->NextRoundTeams, combat->NextTurnChangeNotificationTeamIds);
				ok = true;
			} catch (Exception &) {
				OsiError("UpdateTurnOrder failed");
			}
		}

		lua_pop(L, 1); // stack: -
		return ok;
	}


	std::optional<STDString> ServerState::GetModPersistentVars(STDString const& modTable)
	{
		StackCheck _(L, 0);
		Restriction restriction(*this, RestrictAll);
		LifetimePin _p(lifetimeStack_);

		PushExtFunction(L, "_GetModPersistentVars");
		push(L, modTable);

		if (CallWithTraceback(L, 1, 1) != 0) {
			ERR("Ext._GetModPersistentVars failed: %s", lua_tostring(L, -1));
			lua_pop(L, 1);
			return {};
		}

		if (lua_type(L, -1) == LUA_TSTRING) {
			return get<char const*>(L, -1);
		}

		return {};
	}


	void ServerState::RestoreModPersistentVars(STDString const& modTable, STDString const& vars)
	{
		StackCheck _(L, 0);
		Restriction restriction(*this, RestrictAll);
		LifetimePin _p(lifetimeStack_);

		PushExtFunction(L, "_RestoreModPersistentVars");
		push(L, modTable);
		push(L, vars);

		CheckedCall(L, 2, "Ext.RestoreModPersistentVars");
	}


	void ServerState::OnGameSessionLoading()
	{
		osiris_.GetIdentityAdapterMap().UpdateAdapters();

		State::OnGameSessionLoading();
	}

	void ServerState::StoryFunctionMappingsUpdated()
	{
		auto helpers = library_.GenerateOsiHelpers();
		LoadScript(helpers, "bootstrapper");
	}
}

namespace dse::esv
{

	ExtensionState & ExtensionState::Get()
	{
		return gExtender->GetServer().GetExtensionState();
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
		return GetModManagerServer();
	}

	void ExtensionState::OnGameSessionLoading()
	{
		dynamicStats_.clear();
		persistentStats_.clear();
		cachedPersistentVars_.clear();
		dse::ExtensionStateBase::OnGameSessionLoading();
	}

	void ExtensionState::Reset()
	{
		ExtensionStateBase::Reset();
		DamageHelpers.Clear();
	}

	void ExtensionState::DoLuaReset()
	{
		Lua.reset();
		Lua = std::make_unique<lua::ServerState>(*this);
		Lua->StoryFunctionMappingsUpdated();
	}

	void ExtensionState::LuaStartup()
	{
		ExtensionStateBase::LuaStartup();

		LuaServerPin lua(*this);
		auto gameState = GetStaticSymbols().GetServerState();
		if (gameState
			&& (*gameState == esv::GameState::LoadLevel
				|| (*gameState == esv::GameState::LoadModule && WasStatLoadTriggered())
				|| *gameState == esv::GameState::LoadGMCampaign
				|| *gameState == esv::GameState::LoadSession
				|| *gameState == esv::GameState::Sync
				|| *gameState == esv::GameState::Paused
				|| *gameState == esv::GameState::Running
				|| *gameState == esv::GameState::GameMasterPause)) {
			lua->OnModuleResume();
		}
	}

	void ExtensionState::MarkPersistentStat(FixedString const& statId)
	{
		persistentStats_.insert(statId);
	}

	void ExtensionState::UnmarkPersistentStat(FixedString const& statId)
	{
		auto it = persistentStats_.find(statId);
		if (it != persistentStats_.end()) {
			persistentStats_.erase(it);
		}
	}

	void ExtensionState::MarkDynamicStat(FixedString const& statId)
	{
		dynamicStats_.insert(statId);
	}


	std::optional<STDString> ExtensionState::GetModPersistentVars(FixedString const& mod)
	{
		auto modTable = GetModTable(mod);
		if (modTable) {
			LuaServerPin lua(*this);
			if (lua) {
				auto vars = lua->GetModPersistentVars(*modTable);
				if (vars) {
					return vars;
				}
			}
		}

		// If we failed to retrieve vars from the Lua state for some reason, 
		// return the persistent vars we loaded from the savegame instead
		auto cachedIt = cachedPersistentVars_.find(mod);
		if (cachedIt != cachedPersistentVars_.end()) {
			ERR("Persistent variables for mod %s could not be retrieved, saving cached values!", mod.Str);
			return cachedIt->second;
		} else {
			return {};
		}
	}

	void ExtensionState::RestoreModPersistentVars(FixedString const& mod, STDString const& vars)
	{
		cachedPersistentVars_.insert(std::make_pair(mod, vars));

		auto modTable = GetModTable(mod);
		if (modTable) {
			DEBUG("Restoring persistent vars for mod %s (%ld bytes)", mod.Str, vars.size());
			LuaServerPin lua(*this);
			if (lua) {
				lua->RestoreModPersistentVars(*modTable, vars);
			}
		} else {
			ERR("Savegame has persistent variables for mod %s, but it is not loaded or has no ModTable! Variables may be lost on next save!", mod.Str);
		}
	}

	std::optional<STDString> ExtensionState::GetModTable(FixedString const& mod)
	{
		auto const& configs = GetConfigs();
		auto configIt = configs.find(mod);
		if (configIt != configs.end() && !configIt->second.ModTable.empty()) {
			return configIt->second.ModTable;
		} else {
			return {};
		}
	}

	std::unordered_set<FixedString> ExtensionState::GetPersistentVarMods()
	{
		std::unordered_set<FixedString> names;
		for (auto const& kv : cachedPersistentVars_) {
			names.insert(kv.first);
		}

		for (auto const& config : GetConfigs()) {
			if (config.second.MinimumVersion >= 43 && !config.second.ModTable.empty()) {
				names.insert(config.first);
			}
		}

		return names;
	}

	void ExtensionState::StoryLoaded()
	{
		DEBUG("ExtensionStateServer::StoryLoaded()");
		LuaServerPin lua(*this);
		if (lua) {
			lua->Osiris().StoryLoaded();
		}
	}

	void ExtensionState::StoryFunctionMappingsUpdated()
	{
		LuaServerPin lua(*this);
		if (lua) {
			lua->StoryFunctionMappingsUpdated();
		}
	}

	void ExtensionState::StorySetMerging(bool isMerging)
	{
		LuaServerPin lua(*this);
		if (lua) {
			lua->Osiris().StorySetMerging(isMerging);
		}
	}
}
