HOOK_DEFN(esv__Status__GetEnterChanceHook, esv__Status__GetEnterChance, esv::Status::GetEnterChanceProc, WrappableFunction)
HOOK_DEFN(esv__StatusHit__Enter, esv__StatusHit__Enter, esv::Status::EnterProc, PreHookableFunction)
HOOK_DEFN(esv__StatusHit__Setup, esv__StatusHit__Setup, esv::StatusHit::SetupProc, PreHookableFunction)
HOOK_DEFN(esv__StatusHeal__Enter, esv__StatusHeal__Enter, esv::Status::EnterProc, PreHookableFunction)
HOOK_DEFN(esv__StatusMachine__ApplyStatus, esv__StatusMachine__ApplyStatus, esv::StatusMachine::ApplyStatusProc, WrappableFunction)
HOOK_DEFN(esv__StatusMachine__Update, esv__StatusMachine__Update, esv::StatusMachine::UpdateProc, PreHookableFunction)
HOOK_DEFN(esv__StatusMachine__DeleteStatusByHandle, esv__StatusMachine__DeleteStatusByHandle, esv::StatusMachine::DeleteStatusByHandleProc, PreHookableFunction)

HOOK_DEFN(esv__ProjectileHelpers__ShootProjectile, esv__ProjectileHelpers__ShootProjectile, esv::ProjectileHelpers__ShootProjectile, WrappableFunction)
HOOK_DEFN(esv__Projectile__Explode, esv__Projectile__Explode, esv::Projectile::ExplodeProc, PreHookableFunction)
HOOK_DEFN(esv__ActionMachine__SetState, esv__ActionMachine__SetState, esv::ActionMachine::SetStateProc, HookableFunction)
HOOK_DEFN(esv__ActionMachine__ResetState, esv__ActionMachine__ResetState, esv::ActionMachine::ResetStateProc, PreHookableFunction)
HOOK_DEFN(RPGStats__ParseProperties, RPGStats__ParseProperties, stats::RPGStats::ParsePropertiesProc, WrappableFunction)
HOOK_DEFN(SkillPrototype__FormatDescriptionParam, SkillPrototype__FormatDescriptionParam, stats::SkillPrototype::FormatDescriptionParamProc, WrappableFunction)
HOOK_DEFN(SkillPrototype__GetSkillDamage, SkillPrototype__GetSkillDamage, stats::SkillPrototype::GetSkillDamageProc, WrappableFunction)
HOOK_DEFN(SkillPrototype__GetAttackAPCost, SkillPrototype__GetAttackAPCost, stats::SkillPrototype::GetAttackAPCostProc, WrappableFunction)
HOOK_DEFN(StatusPrototype__FormatDescriptionParam, StatusPrototype__FormatDescriptionParam, stats::StatusPrototype::FormatDescriptionParamProc, WrappableFunction)
HOOK_DEFN(esv__ExecutePropertyDataOnGroundHit, esv__ExecutePropertyDataOnGroundHit, esv::ExecutePropertyDataOnGroundHitProc, PostHookableFunction)
HOOK_DEFN(esv__ExecutePropertyDataOnPositionOnly, esv__ExecutePropertyDataOnPositionOnly, esv::ExecutePropertyDataOnPositionOnlyProc, PostHookableFunction)
HOOK_DEFN(esv__ExecuteCharacterSetExtraProperties, esv__ExecuteCharacterSetExtraProperties, esv::ExecuteCharacterSetExtraPropertiesProc, PostHookableFunction)

HOOK_DEFN(esv__AiHelpers__PeekAction, esv__AiHelpers__PeekAction, esv::AiHelpers::PeekActionProc, PreHookableFunction)
HOOK_DEFN(esv__AiHelpers__SortActions, esv__AiHelpers__SortActions, esv::AiHelpers::SortActionsProc, WrappableFunction)

HOOK_DEFN(esv__TurnManager__UpdateTurnOrder, esv__TurnManager__UpdateTurnOrder, esv::TurnManager::UpdateTurnOrderProc, PostHookableFunction)

HOOK_DEFN(UIObjectManager__CreateUIObject, UIObjectManager__CreateUIObject, UIObjectManager::CreateUIObjectProc, PostHookableFunction)

HOOK_DEFN(esv__Character_Hit, esv__Character__Hit, esv::Character::HitProc, WrappableFunction)
HOOK_DEFN(esv__Character_ApplyDamageHook, esv__Character__ApplyDamage, esv::Character::ApplyDamageProc, WrappableFunction)
HOOK_DEFN(CDivinityStats_Character__HitInternal, CDivinityStats_Character__HitInternal, stats::Character::HitInternalProc, WrappableFunction)

HOOK_DEFN(esv__ItemHelpers__GenerateTreasureItem, esv__ItemHelpers__GenerateTreasureItem, esv::ItemHelpers__GenerateTreasureItem, WrappableFunction)
HOOK_DEFN(esv__CombineManager__ExecuteCombination, esv__CombineManager__ExecuteCombination, esv::CombineManager::ExecuteCombinationProc, WrappableFunction)
HOOK_DEFN(esv__LoadProtocol__HandleModuleLoaded, esv__LoadProtocol__HandleModuleLoaded, esv::LoadProtocol::HandleModuleLoadedProc, WrappableFunction)
HOOK_DEFN(Module__Hash, Module__Hash, Module::HashProc, WrappableFunction)
HOOK_DEFN(App__OnInputEvent, App__OnInputEvent, App::VMT::OnInputEventProc, PreHookableFunction)
HOOK_DEFN(FileReader__ctor, ls__FileReader__ctor, FileReader::CtorProc, WrappableFunction)
HOOK_DEFN(ecl__InventoryProtocol__PostUpdate, ecl__InventoryProtocol__PostUpdate, ecl::InventoryProtocol::PostUpdateProc, WrappableFunction)
HOOK_DEFN(ls__InputManager__InjectInput, ls__InputManager__InjectInput, InputManager::InjectInputProc, PreHookableFunction)
HOOK_DEFN(ecl__EquipmentVisualsSystem__CreateVisuals, ecl__EquipmentVisualsSystem__CreateVisuals, ecl::EquipmentVisualsSystem::CreateVisualsProc, PreHookableFunction)
