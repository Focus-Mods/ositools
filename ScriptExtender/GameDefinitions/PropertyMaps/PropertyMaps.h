#pragma once

#include <Osiris/Shared/CustomFunctions.h>
#include <Osiris/Functions/FunctionLibrary.h>
#include <GameDefinitions/PropertyMaps/PropertyMap.h>
#include <GameDefinitions/GameObjects/Surface.h>
#include <GameDefinitions/GameObjects/RootTemplates.h>

namespace dse
{
	extern PropertyMap<esv::Status, void> gStatusPropertyMap;
	extern PropertyMap<esv::StatusConsumeBase, esv::Status> gStatusConsumeBasePropertyMap;
	extern PropertyMap<esv::StatusHit, esv::Status> gStatusHitPropertyMap;
	extern PropertyMap<esv::StatusDying, esv::Status> gStatusDyingPropertyMap;
	extern PropertyMap<esv::StatusHeal, esv::Status> gStatusHealPropertyMap;
	extern PropertyMap<esv::StatusCharmed, esv::Status> gStatusCharmedPropertyMap;
	extern PropertyMap<esv::StatusKnockedDown, esv::Status> gStatusKnockedDownPropertyMap;
	extern PropertyMap<esv::StatusSummoning, esv::Status> gStatusSummoningPropertyMap;
	extern PropertyMap<esv::StatusHealing, esv::StatusConsume> gStatusHealingPropertyMap;
	extern PropertyMap<esv::StatusThrown, esv::Status> gStatusThrownPropertyMap;
	extern PropertyMap<esv::StatusTeleportFall, esv::Status> gStatusTeleportFallPropertyMap;
	extern PropertyMap<esv::StatusCombat, esv::Status> gStatusCombatPropertyMap;
	extern PropertyMap<esv::StatusAoO, esv::Status> gStatusAoOPropertyMap;
	extern PropertyMap<esv::StatusSneaking, esv::Status> gStatusSneakingPropertyMap;
	extern PropertyMap<esv::StatusUnlock, esv::Status> gStatusUnlockPropertyMap;
	extern PropertyMap<esv::StatusBoost, esv::Status> gStatusBoostPropertyMap;
	extern PropertyMap<esv::StatusUnsheathed, esv::Status> gStatusUnsheathedPropertyMap;
	extern PropertyMap<esv::StatusStance, esv::StatusConsumeBase> gStatusStancePropertyMap;
	extern PropertyMap<esv::StatusLying, esv::Status> gStatusLyingPropertyMap;
	extern PropertyMap<esv::StatusInfectiousDiseased, esv::StatusConsumeBase> gStatusInfectiousDiseasedPropertyMap;
	extern PropertyMap<esv::StatusInvisible, esv::StatusConsumeBase> gStatusInvisiblePropertyMap;
	extern PropertyMap<esv::StatusRotate, esv::Status> gStatusRotatePropertyMap;
	extern PropertyMap<esv::StatusIdentify, esv::Status> gStatusIdentifyPropertyMap;
	extern PropertyMap<esv::StatusRepair, esv::Status> gStatusRepairPropertyMap;
	extern PropertyMap<esv::StatusMaterial, esv::Status> gStatusMaterialPropertyMap;
	extern PropertyMap<esv::StatusExplode, esv::Status> gStatusExplodePropertyMap;
	extern PropertyMap<esv::StatusAdrenaline, esv::StatusConsumeBase> gStatusAdrenalinePropertyMap;
	extern PropertyMap<esv::StatusShacklesOfPain, esv::StatusConsumeBase> gStatusShacklesOfPainPropertyMap;
	extern PropertyMap<esv::StatusShacklesOfPainCaster, esv::StatusConsumeBase> gStatusShacklesOfPainCasterPropertyMap;
	extern PropertyMap<esv::StatusDrain, esv::Status> gStatusDrainPropertyMap;
	extern PropertyMap<esv::StatusSpiritVision, esv::StatusConsumeBase> gStatusSpiritVisionPropertyMap;
	extern PropertyMap<esv::StatusSpirit, esv::Status> gStatusSpiritPropertyMap;
	extern PropertyMap<esv::StatusDamage, esv::StatusConsumeBase> gStatusDamagePropertyMap;
	extern PropertyMap<esv::StatusClimbing, esv::Status> gStatusClimbingPropertyMap;
	extern PropertyMap<esv::StatusIncapacitated, esv::StatusConsumeBase> gStatusIncapacitatedPropertyMap;
	extern PropertyMap<esv::StatusInSurface, esv::Status> gStatusInSurfacePropertyMap;
	extern PropertyMap<esv::StatusPolymorphed, esv::StatusConsumeBase> gStatusPolymorphedPropertyMap;
	extern PropertyMap<esv::StatusDamageOnMove, esv::StatusDamage> gStatusDamageOnMovePropertyMap;
	extern PropertyMap<esv::StatusChallenge, esv::StatusConsumeBase> gStatusChallengePropertyMap;
	extern PropertyMap<esv::StatusHealSharing, esv::StatusConsumeBase> gStatusHealSharingPropertyMap;
	extern PropertyMap<esv::StatusHealSharingCaster, esv::StatusConsumeBase> gStatusHealSharingCasterPropertyMap;
	extern PropertyMap<esv::StatusActiveDefense, esv::StatusConsume> gStatusActiveDefensePropertyMap;
	extern PropertyMap<esv::StatusSpark, esv::StatusConsume> gStatusSparkPropertyMap;

	extern PropertyMap<stats::HitDamageInfo, void> gHitDamageInfoPropertyMap;
	extern PropertyMap<esv::DamageHelpers, void> gDamageHelpersPropertyMap;
	extern PropertyMap<esv::ShootProjectileHelper, void> gShootProjectileHelperPropertyMap;
	extern PropertyMap<eoc::ItemDefinition, void> gEoCItemDefinitionPropertyMap;
	extern PropertyMap<stats::EquipmentAttributes, void> gEquipmentAttributesPropertyMap;
	extern PropertyMap<stats::EquipmentAttributesWeapon, stats::EquipmentAttributes> gEquipmentAttributesWeaponPropertyMap;
	extern PropertyMap<stats::EquipmentAttributesArmor, stats::EquipmentAttributes> gEquipmentAttributesArmorPropertyMap;
	extern PropertyMap<stats::EquipmentAttributesShield, stats::EquipmentAttributes> gEquipmentAttributesShieldPropertyMap;
	extern PropertyMap<stats::CharacterDynamicStat, void> gCharacterDynamicStatPropertyMap;
	extern PropertyMap<stats::Character, void> gCharacterStatsPropertyMap;
	extern PropertyMap<stats::Item, void> gItemStatsPropertyMap;
	extern PropertyMap<eoc::PlayerCustomData, void> gPlayerCustomDataPropertyMap;
	extern PropertyMap<esv::Character, void> gCharacterPropertyMap;
	extern PropertyMap<esv::Item, void> gItemPropertyMap;
	extern PropertyMap<esv::Projectile, void> gProjectilePropertyMap;
	extern PropertyMap<esv::Surface, void> gEsvSurfacePropertyMap;
	extern PropertyMap<Trigger, void> gTriggerPropertyMap;
	extern PropertyMap<SoundVolumeTriggerData, void> gSoundVolumeTriggerDataPropertyMap;

	extern PropertyMap<esv::SurfaceAction, void> gEsvSurfaceActionPropertyMap;
	extern PropertyMap<esv::TransformSurfaceAction, esv::SurfaceAction> gEsvTransformSurfaceActionPropertyMap;
	extern PropertyMap<esv::CreateSurfaceActionBase, esv::SurfaceAction> gEsvCreateSurfaceActionBasePropertyMap;
	extern PropertyMap<esv::CreateSurfaceAction, esv::CreateSurfaceActionBase> gEsvCreateSurfaceActionPropertyMap;
	extern PropertyMap<esv::ChangeSurfaceOnPathAction, esv::CreateSurfaceActionBase> gEsvChangeSurfaceOnPathActionPropertyMap;
	extern PropertyMap<esv::CreatePuddleAction, esv::CreateSurfaceActionBase> gEsvCreatePuddleActionPropertyMap;
	extern PropertyMap<esv::ExtinguishFireAction, esv::CreateSurfaceActionBase> gEsvExtinguishFireActionPropertyMap;
	extern PropertyMap<esv::RectangleSurfaceAction, esv::CreateSurfaceActionBase> gEsvRectangleSurfaceActionPropertyMap;
	extern PropertyMap<esv::PolygonSurfaceAction, esv::CreateSurfaceActionBase> gEsvPolygonSurfaceActionPropertyMap;
	extern PropertyMap<esv::SwapSurfaceAction, esv::CreateSurfaceActionBase> gEsvSwapSurfaceActionPropertyMap;
	extern PropertyMap<esv::ZoneAction, esv::CreateSurfaceActionBase> gEsvZoneActionPropertyMap;


	extern PropertyMap<esv::ASAttack, void> gASAttackPropertyMap;
	extern PropertyMap<esv::ASPrepareSkill, void> gASPrepareSkillPropertyMap;
	extern PropertyMap<esv::SkillState, void> gSkillStatePropertyMap;

	extern PropertyMap<ecl::Character, void> gEclCharacterPropertyMap;
	extern PropertyMap<ecl::Item, void> gEclItemPropertyMap;
	extern PropertyMap<ecl::Status, void> gEclStatusPropertyMap;

	extern PropertyMap<GameObjectTemplate, void> gGameObjectTemplatePropertyMap;
	extern PropertyMap<EoCGameObjectTemplate, GameObjectTemplate> gEoCGameObjectTemplatePropertyMap;
	extern PropertyMap<CharacterTemplate, EoCGameObjectTemplate> gCharacterTemplatePropertyMap;
	extern PropertyMap<ItemTemplate, EoCGameObjectTemplate> gItemTemplatePropertyMap;
	extern PropertyMap<ProjectileTemplate, EoCGameObjectTemplate> gProjectileTemplatePropertyMap;
	extern PropertyMap<CombatComponentTemplate, void> gCombatComponentTemplatePropertyMap;
	extern PropertyMap<SurfaceTemplate, GameObjectTemplate> gSurfaceTemplatePropertyMap;
	extern PropertyMap<TriggerTemplate, GameObjectTemplate> gTriggerTemplatePropertyMap;

	void InitPropertyMaps();

	enum class OsiPropertyMapType
	{
		Integer,
		Integer64,
		Real,
		String,
		GuidStringHandle,
		Vector3
	};


	bool OsirisPropertyMapSetRaw(PropertyMapBase const & propertyMap, void * obj,
		OsiArgumentDesc const & args, uint32_t firstArg, OsiPropertyMapType type, bool throwError = true);
	bool OsirisPropertyMapGetRaw(PropertyMapBase const & propertyMap, void * obj,
		OsiArgumentDesc & args, uint32_t firstArg, OsiPropertyMapType type, bool throwError = true);

	template <class T>
	bool OsirisPropertyMapGet(PropertyMapInterface<T> const & propertyMap, T * obj,
		OsiArgumentDesc & args, uint32_t firstArg, OsiPropertyMapType type, bool throwError = true)
	{
		return OsirisPropertyMapGetRaw(propertyMap, obj, args, firstArg, type, throwError);
	}

	template <class T>
	bool OsirisPropertyMapSet(PropertyMapInterface<T> const & propertyMap, T * obj,
		OsiArgumentDesc const & args, uint32_t firstArg, OsiPropertyMapType type, bool throwError = true)
	{
		return OsirisPropertyMapSetRaw(propertyMap, obj, args, firstArg, type, throwError);
	}

	bool LuaPropertyMapGet(lua_State * L, PropertyMapBase const & propertyMap, void * obj,
		char const * propertyName, bool throwError);
	bool LuaPropertyMapGet(lua_State* L, PropertyMapBase const& propertyMap, void* obj,
		FixedString const& propertyName, bool throwError);
	bool LuaPropertyMapSet(lua_State * L, int index, PropertyMapBase const & propertyMap,
		void * obj, char const * propertyName, bool throwError);
}
