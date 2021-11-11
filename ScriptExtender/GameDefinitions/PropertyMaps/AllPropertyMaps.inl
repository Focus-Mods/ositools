#include <GameDefinitions/PropertyMaps/Events.inl>
#include <GameDefinitions/PropertyMaps/Statuses.inl>
#include <GameDefinitions/PropertyMaps/Character.inl>
#include <GameDefinitions/PropertyMaps/Item.inl>
#include <GameDefinitions/PropertyMaps/Stats.inl>
#include <GameDefinitions/PropertyMaps/Surface.inl>
#include <GameDefinitions/PropertyMaps/RootTemplates.inl>


BEGIN_CLS(HitDamageInfo)
P(Equipment)
P(TotalDamage)
P(DamageDealt)
P(DeathType)
P(DamageType)
P(AttackDirection)
P(ArmorAbsorption)
P(LifeSteal)
P(EffectFlags)
P(HitWithWeapon)
END_CLS()


BEGIN_CLS(esv::DamageHelpers)
P(SimulateHit)
P(HitType)
P(NoHitRoll)
P(ProcWindWalker)
P(ForceReduceDurability)
P(HighGround)
P(CriticalRoll)
P(HitReason)
P(DamageSourceType)
P(Strength)
END_CLS()


BEGIN_CLS(esv::ShootProjectileHelper)
P(SkillId)
P(Caster)
P(Source)
P(Target)
P(StartPosition)
P(EndPosition)
P(Random)
P(CasterLevel)
P(IsTrap)
P(UnknownFlag1)
P(CleanseStatuses)
P(StatusClearChance)
P(IsFromItem)
P(IsStealthed)
P(IgnoreObjects)
END_CLS()


BEGIN_CLS(esv::Projectile)
// EoCServerObject
P_RO(NetID)
P_RO(MyGuid)
// Projectile
P_RO(CasterHandle)
P_RO(SourceHandle)
P_RO(TargetObjectHandle)
P_RO(HitObjectHandle)
P(SourcePosition)
P(TargetPosition)
P(DamageType)
P(DamageSourceType)
P(LifeTime)
P(HitInterpolation)
P(ExplodeRadius0)
P(ExplodeRadius1)
P(DeathType)
P(SkillId)
P_RO(WeaponHandle)
P_RO(MovingEffectHandle)
P(SpawnEffect)
P(SpawnFXOverridesImpactFX)
P_RO(EffectHandle)
P(RequestDelete)
P_RO(Launched)
P(IsTrap)
P(UseCharacterStats)
P(ReduceDurability)
P(AlwaysDamage)
P(ForceTarget)
P(IsFromItem)
P(DivideDamage)
P(IgnoreRoof)
P(CanDeflect)
P(IgnoreObjects)
P(CleanseStatuses)
P(StatusClearChance)
P_RO(Position)
P_RO(PrevPosition)
P_RO(Velocity)
P_RO(Scale)
P_RO(CurrentLevel)
END_CLS()



BEGIN_CLS(Trigger)
// P_RO(Handle) // Defunct, use GetObjectHandle() instead
// P_RO(UUID) // Defunct, use GetGuid() instead
P_RO(SyncFlags)
P_RO(Translate)
P_RO(TriggerType)
P_RO(IsGlobal)
P_RO(Level)
END_CLS()


BEGIN_CLS(SoundVolumeTriggerData)
P_RO(AmbientSound)
P_RO(Occlusion)
P_RO(AuxBus1)
P_RO(AuxBus2)
P_RO(AuxBus3)
P_RO(AuxBus4)
END_CLS()


BEGIN_CLS(esv::ASAttack)
P_RO(TargetHandle)
P_RO(TargetPosition)
P_RO(IsFinished)
P_RO(AlwaysHit)
P_RO(TimeRemaining)
P_RO(AnimationFinished)
P_RO(TotalHits)
P_RO(TotalHitOffHand)
P_RO(TotalShoots)
P_RO(TotalShootsOffHand)
P_RO(HitCount)
P_RO(HitCountOffHand)
P_RO(ShootCount)
P_RO(ShootCountOffHand)
P_RO(MainWeaponHandle)
P_RO(OffWeaponHandle)
P_RO(MainHandHitType)
P_RO(OffHandHitType)
P_RO(ProjectileUsesHitObject)
P_RO(ProjectileStartPosition)
P_RO(ProjectileTargetPosition)
P_RO(DamageDurability)
END_CLS()


BEGIN_CLS(esv::ASPrepareSkill)
P_RO(SkillId)
P_RO(PrepareAnimationInit)
P_RO(PrepareAnimationLoop)
P_RO(IsFinished)
P_RO(IsEntered)
END_CLS()


BEGIN_CLS(esv::SkillState)
P_RO(SkillId)
P_RO(CharacterHandle)
P_RO(SourceItemHandle)
P_RO(CanEnter)
P_RO(IsFinished)
P_RO(IgnoreChecks)
P_RO(IsStealthed)
P_RO(PrepareTimerRemaining)
P_RO(ShouldExit)
P_RO(CleanseStatuses)
P_RO(StatusClearChance)
P_RO(CharacterHasSkill)
END_CLS()

// FIXME - placeholders
BEGIN_CLS(eoc::AiGrid)
END_CLS()

// FIXME - placeholders
BEGIN_CLS(UIObject)
END_CLS()

// FIXME - placeholders
BEGIN_CLS(DamagePairList)
END_CLS()

