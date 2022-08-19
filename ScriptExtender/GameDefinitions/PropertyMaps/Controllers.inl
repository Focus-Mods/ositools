BEGIN_CLS(esv::BaseController)
P_RO(TypeId)
P_RO(Character)
END_CLS()


BEGIN_CLS(esv::StatusController)
INHERIT(esv::BaseController)
P(Flags)
P_RO(SummoningTransactionId)
P_RO(PolymorphingTransactionId)
P_RO(ActionTransactionId)
P_RO(TeleportFallingTransactionId)
P_RO(DeathAnimationTransactionId)
P_RO(SteerToEnemyTransactionId)
P_RO(Flags2)
P(CombatStartPosition)
P(KnockDownQueued)
P(CombatStartPositionFloodDone)
P(ResurrectedEvent)
END_CLS()


BEGIN_CLS(esv::TaskController)
INHERIT(esv::BaseController)
P_REF(Tasks)
P_RO(RemoveNextTask_M)
P_RO(UpdateInProgress)
P_RO(FlushRequested)
END_CLS()


BEGIN_CLS(esv::Task)
P_RO(Character)
P_RO(TaskTypeId)
P_RO(TaskState)
P_RO(Failed)
P_RO(Flags)
END_CLS()


BEGIN_CLS(esv::OsirisTeleportToLocationTask)
INHERIT(esv::Task)
P(Position)
P(PreviousLevel)
P(Level)
P(SetRotation)
P(LeaveCombat)
P(FindPosition)
P(FindFleePosition)
P(Executed)
P(UnchainFollowers)
P(Rotation)
P(ArriveEvent)
END_CLS()


BEGIN_CLS(esv::MoveTask)
INHERIT(esv::Task)
P_RO(MoveTransactionId)
P(CurrentTarget)
P(ArriveEvent)
P_REF(Checkpoints)
P(TimeSpent)
P(TimeTargetFound)
END_CLS()


BEGIN_CLS(esv::OsirisMoveToObjectTask)
INHERIT(esv::MoveTask)
P(MinDistance)
P(MaxDistance)
P(Target)
P(SpeedMultiplier)
P(DefaultSpeed)
P(IncreaseSpeed)
END_CLS()


BEGIN_CLS(esv::OsirisMoveInRangeTask)
INHERIT(esv::MoveTask)
P(Target)
P(TargetPos)
P(MinRange)
P(WantedRange)
P(MaxRange)
P(CachedTargetPos)
P(CachedTarget)
P(CachedCloseEnough)
P(CachedResult)
P(MustBeInTrigger)
P(FallbackMoveCloser)
P(AttackMove)
P_REF(HintTriggers)
P_REF(ProjectileTemplate)
P_REF(Skill)
END_CLS()


BEGIN_CLS(esv::OsirisMoveToLocationTask)
INHERIT(esv::MoveTask)
P(TargetRotation)
P(MinDistance)
P(MaxDistance)
P(TargetToIgnore)
P(TargetRotationSet)
END_CLS()


BEGIN_CLS(esv::OsirisFleeTask)
INHERIT(esv::MoveTask)
P(FleeFromRelation)
P(FleeFromRelationRange)
P(StartPosition)
P(FleeFromTileStates)
P_REF(SurfacePathInfluences)
P(OutOfSight)
P(StartedMoving)
END_CLS()


BEGIN_CLS(esv::OsirisPlayAnimationTask)
INHERIT(esv::Task)
P(OriginalAnimation)
P(Animation)
P(EndAnimation)
P(FinishedEvent)
P(WaitForCompletion)
P(ExitOnFinish)
P(NoBlend)
P_RO(ActionTransactionId)
P(Time)
P(Timer)
P(AnimationDuration)
P_REF(AnimationNames)
P(CurrentTime)
END_CLS()


BEGIN_CLS(esv::OsirisAttackTask)
INHERIT(esv::Task)
P(Target)
P(TargetPos)
P(ArriveEvent)
P_RO(BehaviorTransactionId)
P(WithoutMove)
P(AlwaysHit)
END_CLS()


BEGIN_CLS(esv::OsirisUseSkillTask)
INHERIT(esv::Task)
P(Skill)
P(Force)
P(Target)
P(TargetPos)
P_RO(BehaviorTransactionId)
P(Success)
P(IgnoreHasSkill)
P(IgnoreChecks)
END_CLS()


BEGIN_CLS(esv::OsirisAppearTask)
INHERIT(esv::Task)
P(Target)
P(TargetPos)
P(Angle)
P_RO(SpawnState)
P(PlayerSpawn)
P(OutOfSight)
P(OnTrail)
P(FinishedEvent)
P(Animation)
END_CLS()


BEGIN_CLS(esv::OsirisDisappearTask)
INHERIT(esv::Task)
P(Target)
P(Angle)
P(OutOfSight)
P(OffStage)
P(Running)
P(ValidTarget)
P(TargetPos)
P(FinishedEvent)
P(SpeedMultiplier)
P(DefaultSpeed)
P(IncreaseSpeed)
P(DisappearCount)
END_CLS()


BEGIN_CLS(esv::OsirisFollowNPCTask)
INHERIT(esv::Task)
P(Target)
END_CLS()


BEGIN_CLS(esv::OsirisWanderTask)
INHERIT(esv::Task)
P(Range)
P(Trigger)
P(Anchor)
P(Duration)
P(Running)
P(Start)
END_CLS()


BEGIN_CLS(esv::OsirisSteerTask)
INHERIT(esv::Task)
P(Target)
P(TargetPos)
P(AngleTolerance)
P(LookAt)
P(SnapToTarget)
P_RO(SteeringTransactionId)
END_CLS()


BEGIN_CLS(esv::OsirisDropTask)
INHERIT(esv::Task)
P(Item)
P(TargetPos)
END_CLS()


BEGIN_CLS(esv::OsirisPickupItemTask)
INHERIT(esv::Task)
P(Item)
P(ArriveEvent)
P_RO(BehaviorTransactionId)
END_CLS()


BEGIN_CLS(esv::OsirisUseItemTask)
INHERIT(esv::Task)
P(Item)
P(ArriveEvent)
P_RO(BehaviorTransactionId)
END_CLS()


BEGIN_CLS(esv::OsirisMoveItemTask)
INHERIT(esv::Task)
P(Item)
P(Position)
P(Amount)
P(ArriveEvent)
P_RO(BehaviorTransactionId)
END_CLS()


BEGIN_CLS(esv::OsirisResurrectTask)
INHERIT(esv::Task)
P(HPPercentage)
P(IsResurrected)
P(Animation)
END_CLS()


BEGIN_CLS(esv::OsirisMoveToAndTalkTask)
INHERIT(esv::Task)
P(Target)
P_RO(BehaviorTransactionId)
P(Movement)
P(DialogInstanceID)
P(IsAutomatedDialog)
P(Timeout)
END_CLS()