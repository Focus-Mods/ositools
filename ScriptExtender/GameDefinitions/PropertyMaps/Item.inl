BEGIN_CLS(eoc::ItemDefinition)
P_RO(Version)
P_RO(NetID)
P_RO(ItemNetId)
P_RO(UUID)
P(RootTemplate)
P_RO(RootTemplateType)
P(OriginalRootTemplate)
P_RO(OriginalRootTemplateType)
P(WorldRot)
P(Scale_M)
P_RO(InventoryNetID)
P_RO(InventorySubContainerNetID)
P(Slot)
P(Amount)
P(GoldValueOverwrite)
P(WeightValueOverwrite)
P(DamageTypeOverwrite)
P(HP)
P(ItemType)
P(CustomDisplayName)
P(CustomDescription)
P(CustomBookContent)
P(GenerationStatsId)
P(GenerationItemType)
P(GenerationRandom)
P(GenerationLevel)
P_REF(GenerationBoostSet)
P_RO(LevelGroupIndex)
P_RO(RootGroupIndex)
P_RO(NameIndex)
P_RO(NameCool)
P(StatsLevel)
P(Key)
P(LockLevel)
P(StatsEntryName)
P(EquipmentStatsType)
P(HasModifiedSkills)
P(Skills)
P_REF(Tags)
P_REF(RuneBoostSet)
P_REF(DeltaModSet)
P_REF(PinnedContainerTags)
P(IsGlobal)
P(Active)
P(HasGeneratedStats)
P(CanBeUsed)
P(IsPinnedContainer)
P(CanBeMoved)
P(CanBePickedUp)
P(Invisible)
P(CustomRequirements)
P(Known)
P(IsIdentified)
P(GMFolding)
P(Floating)
P(CanUseRemotely)

P_FUN(ResetProgression, ResetProgression)

#if defined(GENERATING_PROPMAP)
pm.AddRawProperty("GenerationBoosts",
	&(GenericGetOffsetProperty<decltype(PM::ObjectType::GenerationBoostSet)>),
	&(GenericSetOffsetProperty<decltype(PM::ObjectType::GenerationBoostSet)>),
	offsetof(PM::ObjectType, GenerationBoostSet)
);
pm.AddRawProperty("RuneBoosts",
	&(GenericGetOffsetProperty<decltype(PM::ObjectType::RuneBoostSet)>),
	&(GenericSetOffsetProperty<decltype(PM::ObjectType::RuneBoostSet)>),
	offsetof(PM::ObjectType, RuneBoostSet)
);
pm.AddRawProperty("DeltaMods",
	&(GenericGetOffsetProperty<decltype(PM::ObjectType::DeltaModSet)>),
	&(GenericSetOffsetProperty<decltype(PM::ObjectType::DeltaModSet)>),
	offsetof(PM::ObjectType, DeltaModSet)
);
#endif

END_CLS()

BEGIN_CLS(esv::ItemGeneration)
P_RO(Base)
P_RO(ItemType)
P_RO(Random)
P_RO(Level)
P_RO(Boosts)
END_CLS()

BEGIN_CLS(esv::Item)
INHERIT(IEoCServerObject)
P_RO(WorldPos)
P_GETTER_SETTER(Flags, LuaGetFlags, LuaSetFlags)
P_BITMASK_GETTER_SETTER(Flags, LuaHasFlag, LuaSetFlag)
P(Flags2)
P_BITMASK(Flags2)
/*
// Make dangerous flags read-only
propertyMap.Flags[GFS.strActivated].Flags &= ~kPropWrite;
propertyMap.Flags[GFS.strOffStage].Flags &= ~kPropWrite;
propertyMap.Flags[GFS.strDestroyed].Flags &= ~kPropWrite;
propertyMap.Flags[GFS.strGlobal].Flags &= ~kPropWrite;
*/
P_RO(CurrentLevel)
// Available via IGameObject
// P_RO(Scale)
P_REF(AI)
P_REF(CurrentTemplate)
P(OriginalTemplateType)
P(CustomDisplayName)
P(CustomDescription)
P(CustomBookContent)
P(StatsId)
P_REF(Stats)
P_REF(StatsFromName)
P_REF(Generation)
P_RO(InventoryHandle)
P_RO(ParentInventoryHandle)
P_RO(Slot)
P(Amount)
P(Vitality)
P(Armor)
P_RO(InUseByCharacterHandle)
P(UserId)
P(Key)
P(LockLevel)
P_REF(StatusMachine)
P_RO(VisualResourceID)
P_RO(OwnerHandle)
P_RO(OriginalOwnerCharacter)
// FIXME - Sockets?
P(ComputedVitality)
P(Rarity)
P(GoldValueOverwrite)
P(WeightValueOverwrite)
P_REF(Tags)
P_RO(TeleportTargetOverride)
P(TreasureLevel)
P_RO(LevelOverride)
P_RO(ForceSynch)
P_RO(TeleportUseCount)
P_RO(PreviousLevel)

// v55 compatibility
PN_REF(RootTemplate, CurrentTemplate)

P_FUN(GetInventoryItems, GetInventoryItemGuids)
P_FUN(GetNearbyCharacters, GetNearbyCharacters)
P_FUN(GetDeltaMods, GetDeltaMods)
P_FUN(SetDeltaMods, LuaSetDeltaMods)
P_FUN(GetGeneratedBoosts, GetGeneratedBoosts)
P_FUN(SetGeneratedBoosts, LuaSetGeneratedBoosts)

P_FALLBACK(&esv::Item::LuaFallbackGet, &esv::Item::LuaFallbackSet)
END_CLS()


BEGIN_CLS(esv::Inventory)
P_RO(GUID)
P_RO(NetID)
P_RO(Handle)
P_RO(EquipmentSlots)
P_RO(ParentHandle)
P_RO(CachedGoldAmount)
P_RO(CachedWeight)
P_RO(IsGlobal)
P_REF(ItemsBySlot)
P_REF(Views)
P_REF(UpdateViews)
P_REF(BuyBackAmounts)
P_REF(TimeItemAddedToInventory)
P_REF(PinnedContainers)
END_CLS()


BEGIN_CLS(esv::InventoryView)
P_RO(NetID)
P_RO(Handle)
P_RO(Owner)
P_RO(ParentType)
P_REF(Parents)
P_RO(ViewId)
P_REF(Items)
P_REF(ItemIndices)
P_REF(PinnedContainerTags)
END_CLS()


BEGIN_CLS(esv::ItemMover)
P_REF(Movements)
END_CLS()


BEGIN_CLS(esv::ItemMovement::InventoryAddParams)
P(OwnerCharacterHandle)
P(Flags)
P(InventoryNetId)
P(Slot)
END_CLS()


BEGIN_CLS(esv::ItemMovement)
P_RO(ItemHandle)
P_RO(MoverHandle)
P(Moving)
P(MovingToInventory)
P(MovingInWorld)
P(HeightForced)
P(AiBounds)
P(WakePhysics)
P(DoHitTest)
P_REF(InventoryAdd)
P(MoveEventName)
END_CLS()


BEGIN_CLS(ecl::Item)
INHERIT(IEoCClientReplicatedObject)
P_RO(WorldPos)
P_GETTER_SETTER(Flags, LuaGetFlags, LuaSetFlags)
P_BITMASK_GETTER_SETTER(Flags, LuaHasFlag, LuaSetFlag)
P_RO(PhysicsFlags)
P_BITMASK(PhysicsFlags)
P_RO(GravityTimer)
P_RO(UnknownTimer)
P_RO(FallTimer)
P_RO(WakePosition)
P_RO(AIBoundSize)
P_RO(CurrentLevel)
// Available via IGameObject
// P_RO(Scale)
P_REF(Physics)
P_REF(AI)
P_REF(CurrentTemplate)
P_REF(Stats)
P_RO(StatsId)
P_REF(StatsFromName)
P_RO(InventoryHandle)
P_RO(InventoryParentHandle)
P_RO(CurrentSlot)
P(Amount)
P(Vitality)
P_REF(StatusMachine)
P_RO(InUseByCharacterHandle)
P_RO(InUseByUserId)
P(KeyName)
P(LockLevel)
P_RO(OwnerCharacterHandle)
P_RO(CachedItemDescription)
P_REF(Tags)
P(Flags2)
P_BITMASK(Flags2)
P(Level)
P(ItemType)
P(GoldValueOverride)
P(BaseWeightOverwrite)
P(ItemColorOverride)
P_REF(CustomDisplayName)
P_REF(CustomDescription)
P_REF(CustomBookContent)
#if defined(OSI_EOCAPP)
P(Icon)
#endif

// v55 compatibility
PN_REF(RootTemplate, CurrentTemplate)

P_FUN(GetInventoryItems, GetInventoryItemGuids)
P_FUN(GetOwnerCharacter, GetOwnerCharacter)
P_FUN(GetDeltaMods, GetDeltaMods)

P_FALLBACK(&ecl::Item::LuaFallbackGet, &ecl::Item::LuaFallbackSet)
END_CLS()


BEGIN_CLS(ecl::Inventory)
P_RO(GUID)
P_RO(NetID)
P_RO(OwnerCharacterHandleUI)
P_RO(EquipmentSlots)
P_RO(ParentHandle)
P_RO(Flags)
P_REF(ItemsBySlot)
// P_REF(Views)
P_REF(UpdateViews)
// P_REF(OfferedAmounts)
// P_REF(BuyBackAmounts)
P_REF(PinnedContainers)
END_CLS()


BEGIN_CLS(ecl::InventoryView)
P_RO(NetID)
P_RO(Handle)
P_RO(ParentNetId_M)
P_REF(ParentInventories)
P_REF(ItemHandles)
P_REF(ItemNetIdToIndex)
END_CLS()


