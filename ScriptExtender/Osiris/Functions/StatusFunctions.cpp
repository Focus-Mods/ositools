#include <stdafx.h>
#include "FunctionLibrary.h"
#include <Extender/ScriptExtender.h>
#include <GameDefinitions/PropertyMaps/PropertyMaps.h>

namespace dse::esv
{
	FunctionHandle StatusIteratorEventHandle;
	FunctionHandle StatusAttemptEventHandle;
	FunctionHandle HitPrepareEventHandle;
	FunctionHandle HitEventHandle;
	FunctionHandle HealEventHandle;
	FunctionHandle ActionStateEnterHandle;
	FunctionHandle ActionStateExitHandle;

	esv::StatusMachine * GetStatusMachine(char const * gameObjectGuid)
	{
		auto character = GetEntityWorld()->GetCharacter(gameObjectGuid, false);
		if (character != nullptr) {
			return character->StatusMachine;
		}

		auto item = GetEntityWorld()->GetItem(gameObjectGuid, false);
		if (item != nullptr) {
			return item->StatusMachine;
		}

		OsiError("Character or item " << gameObjectGuid << " does not exist!");
		return nullptr;
	}

	namespace func
	{
		void IterateStatuses(OsiArgumentDesc const & args)
		{
			auto gameObjectGuid = args[0].String;
			auto eventName = args[1].String;

			auto statusMachine = GetStatusMachine(gameObjectGuid);
			if (statusMachine == nullptr) return;

			for (auto const status : statusMachine->Statuses) {
				auto eventArgs = OsiArgumentDesc::Create(OsiArgumentValue{ ValueType::String, eventName });
				eventArgs->Add(OsiArgumentValue{ ValueType::GuidString, gameObjectGuid });
				eventArgs->Add(OsiArgumentValue{ ValueType::String, status->StatusId.Str });
				eventArgs->Add(OsiArgumentValue{ (int64_t)status->StatusHandle });

				gExtender->GetServer().Osiris().GetCustomFunctionInjector().ThrowEvent(StatusIteratorEventHandle, eventArgs);

				delete eventArgs;
			}
		}

		bool ObjectHasStatusType(OsiArgumentDesc& args)
		{
			auto gameObjectGuid = args[0].String;
			auto statusType = args[1].String;
			auto& hasStatus = args[2];

			auto statusMachine = GetStatusMachine(gameObjectGuid);
			if (statusMachine == nullptr) return false;
			
			auto typeId = EnumInfo<StatusType>::Find(statusType);
			if (!typeId) {
				OsiError("Status type unknown: " << statusType);
				return false;
			}

			hasStatus.Set(0);
			for (auto const status : statusMachine->Statuses) {
				if (status->GetStatusId() == *typeId) {
					hasStatus.Set(1);
					break;
				}
			}

			return true;
		}

		bool StatusGetHandle(OsiArgumentDesc & args)
		{
			auto gameObjectGuid = args[0].String;
			auto statusId = args[1].String;

			auto statusMachine = GetStatusMachine(gameObjectGuid);
			if (statusMachine == nullptr) return false;

			FixedString statusIdFS(args[1].String);
			if (!statusIdFS) {
				// No fixed string with this ID --> invalid status name
				OsiWarn("Status " << args[1].String << " not in string table, possibly invalid status name?");
				return false;
			}

			for (auto const status : statusMachine->Statuses) {
				if (status->StatusId == statusIdFS) {
					args[2].Set((int64_t)status->StatusHandle);
					return true;
				}
			}

			return false;
		}

		void StatusPreventApply(OsiArgumentDesc const & args)
		{
			auto gameObject = GetEntityWorld()->GetGameObject(args[0].String);
			auto statusHandle = ObjectHandle{ args[1].Int64 };
			auto preventApply = args[2].Int32;

			if (gameObject == nullptr) {
				OsiError("Game object " << args[0].String << " does not exist!");
				return;
			}

			gExtender->GetStatusHelpers().PreventApply(gameObject, statusHandle, preventApply != 0);
		}


		bool ApplyActiveDefense(OsiArgumentDesc & args)
		{
			auto characterGuid = args[0].String;
			auto statusId = args[1].String;
			auto lifeTime = args[2].Float;

			auto character = GetEntityWorld()->GetCharacter(characterGuid);
			if (character == nullptr) {
				OsiError("Character " << characterGuid << " does not exist!");
				return false;
			}

			auto statusHandle = gExtender->GetStatusHelpers().ApplyActiveDefense(character, FixedString(statusId), lifeTime);
			if (statusHandle) {
				args[3].Set((int64_t)*statusHandle);
				return true;
			} else {
				return false;
			}
		}


		bool ApplyDamageOnMove(OsiArgumentDesc & args)
		{
			auto characterGuid = args[0].String;
			auto statusId = args[1].String;
			auto sourceCharacterGuid = args[2].String;
			auto lifeTime = args[3].Float;
			auto distancePerDamage = args[4].Float;

			auto character = GetEntityWorld()->GetCharacter(characterGuid);
			if (character == nullptr) {
				OsiError("Character " << characterGuid << " does not exist!");
				return false;
			}

			auto sourceCharacter = GetEntityWorld()->GetCharacter(sourceCharacterGuid);
			auto statusHandle = gExtender->GetStatusHelpers().ApplyDamageOnMove(character, FixedString(statusId), sourceCharacter, lifeTime, distancePerDamage);
			if (statusHandle) {
				args[5].Set((int64_t)*statusHandle);
				return true;
			} else {
				return false;
			}
		}



		template <OsiPropertyMapType Type>
		bool ActionStateGet(OsiArgumentDesc & args)
		{
			auto characterGuid = args[0].String;

			auto character = GetEntityWorld()->GetCharacter(characterGuid);
			if (character == nullptr
				|| character->ActionMachine == nullptr
				|| character->ActionMachine->Layers[0].State == nullptr) {
				return false;
			}

			auto state = character->ActionMachine->Layers[0].State;

			if (state->GetType() == ActionStateType::ASUseSkill) {
				auto useSkill = static_cast<ASUseSkill*>(state);
				if (useSkill->Skill != nullptr) {
					return OsirisPropertyMapGetRaw(gSkillStatePropertyMap, useSkill->Skill, args, 1, Type);
				} else {
					OsiError("No skill bound to ASUseSkill!");
					return false;
				}
			}

			auto propertyMap = state->GetPropertyMap();
			if (propertyMap != nullptr) {
				return OsirisPropertyMapGetRaw(*propertyMap, state, args, 1, Type, true);
			} else {
				OsiError("No property map available for action type " << (unsigned)state->GetType() << " yet!");
				return false;
			}
		}

		bool CharacterGetCurrentAction(OsiArgumentDesc & args)
		{
			auto characterGuid = args[0].String;
			auto & action = args[1];

			auto character = GetEntityWorld()->GetCharacter(characterGuid);
			if (character == nullptr
				|| character->ActionMachine == nullptr) {
				return false;
			}

			auto actionState = character->ActionMachine->Layers[0].State;

			if (actionState == nullptr) {
				action.Set("");
				return true;
			}

			auto typeName = actionState->GetTypeName();
			if (typeName == nullptr) return false;
			action.Set(typeName);
			return true;
		}
	}

	esv::Status * GetStatusHelper(OsiArgumentDesc const & args)
	{
		auto gameObjectGuid = args[0].String;
		ObjectHandle statusHandle{ args[1].Int64 };

		if (statusHandle.GetType() == DamageHelpers::HitHandleTypeId) {
			OsiError("Attempted to use a hit handle in a status function.");
			OsiError("For handles received from NRD_OnPrepareHit/NRD_PrepareHit use the NRD_HitGet... functions instead!");
			return nullptr;
		}

		esv::Status * status{ nullptr };
		auto character = GetEntityWorld()->GetCharacter(gameObjectGuid, false);
		if (character != nullptr) {
			status = character->GetStatus(statusHandle, true);
		} else {
			auto item = GetEntityWorld()->GetItem(gameObjectGuid, false);
			if (item != nullptr) {
				status = item->GetStatus(statusHandle, true);
			} else {
				OsiError("Character or item " << gameObjectGuid << " does not exist!");
				return nullptr;
			}
		}

		if (status == nullptr) {
			OsiError("No status found with handle " << (int64_t)statusHandle);
			return nullptr;
		}

		return status;
	}

	int32_t CustomFunctionLibrary::OnStatusGetEnterChance(esv::Status::GetEnterChanceProc* wrappedGetEnterChance,
		esv::Status * status, bool isEnterCheck)
	{
		LuaServerPin lua(ExtensionState::Get());
		if (lua) {
			auto enterChance = lua->StatusGetEnterChance(status, isEnterCheck);
			if (enterChance) {
				return *enterChance;
			}
		}

		return wrappedGetEnterChance(status, isEnterCheck);
	}

	int32_t CustomFunctionLibrary::OnGetHitChance(CDivinityStats_Character__GetHitChance * wrappedGetHitChance,
		CDivinityStats_Character * attacker, CDivinityStats_Character * target)
	{
		LuaVirtualPin lua(gExtender->GetCurrentExtensionState());
		if (lua) {
			auto hitChance = lua->GetHitChance(attacker, target);
			if (hitChance) {
				return *hitChance;
			}
		}

		return wrappedGetHitChance(attacker, target);
	}

	void CustomFunctionLibrary::OnStatusHealEnter(esv::Status * status)
	{
		auto statusHeal = static_cast<esv::StatusHeal *>(status);
		gExtender->GetStatusHelpers().ThrowStatusHealEnter(statusHeal);
	}

	void CustomFunctionLibrary::OnBeforeActionMachineSetState(esv::ActionMachine* self, uint64_t actionLayer, esv::ActionState* actionState, int* somePtr, bool force, bool setLayer)
	{
		if (actionLayer >= 3 || self->IsEntering[actionLayer]) return;

		auto curState = self->Layers[actionLayer].State;
		if (curState == nullptr || curState == actionState) return;

		if (!curState->CanExit() && !force) return;

		auto character = GetEntityWorld()->GetCharacter(self->CharacterHandle);
		if (character == nullptr) {
			OsiErrorS("ActionMachine has no character?");
			return;
		}

		auto type = curState->GetType();
		auto typeName = curState->GetTypeName();
		// Avoid event spam from idle/anim states
		if (type == esv::ActionStateType::ASIdle
			|| type == esv::ActionStateType::ASAnimation
			|| typeName == nullptr) {
			return;
		}

		auto eventArgs = OsiArgumentDesc::Create(OsiArgumentValue{ ValueType::GuidString, character->GetGuid()->Str });
		eventArgs->Add(OsiArgumentValue{ ValueType::String, typeName });
		gExtender->GetServer().Osiris().GetCustomFunctionInjector().ThrowEvent(ActionStateExitHandle, eventArgs);
		delete eventArgs;
	}

	void CustomFunctionLibrary::OnActionMachineResetState(esv::ActionMachine* self, bool force)
	{
		for (auto layer = 0; layer < 3; layer++) {
			if (self->IsEntering[layer]) continue;

			auto curState = self->Layers[layer].State;
			if (curState == nullptr || (!curState->CanExit() && !force)) continue;

			auto character = GetEntityWorld()->GetCharacter(self->CharacterHandle);
			if (character == nullptr) {
				OsiErrorS("ActionMachine has no character?");
				return;
			}

			auto type = curState->GetType();
			auto typeName = curState->GetTypeName();
			// Avoid event spam from idle/anim states
			if (type == esv::ActionStateType::ASIdle
				|| type == esv::ActionStateType::ASAnimation
				|| typeName == nullptr) {
				return;
			}

			auto eventArgs = OsiArgumentDesc::Create(OsiArgumentValue{ ValueType::GuidString, character->GetGuid()->Str });
			eventArgs->Add(OsiArgumentValue{ ValueType::String, typeName });
			gExtender->GetServer().Osiris().GetCustomFunctionInjector().ThrowEvent(ActionStateExitHandle, eventArgs);
			delete eventArgs;
		}
	}

	Projectile* CustomFunctionLibrary::OnShootProjectile(esv::ProjectileHelpers__ShootProjectile* next, ShootProjectileHelper* helper)
	{
		{
			LuaServerPin lua(ExtensionState::Get());
			if (lua) {
				lua->OnBeforeShootProjectile(helper);
			}
		}

		auto projectile = next(helper);

		if (projectile) {
			LuaServerPin lua(ExtensionState::Get());
			if (lua) {
				lua->OnShootProjectile(projectile);
			}
		}

		return projectile;
	}

	void CustomFunctionLibrary::OnProjectileExplode(Projectile* projectile)
	{
		// We're not sure yet whether the projectile exploded or was deflected, 
		// so install an OnHit hook to detect the actual hit event
		auto hitProxy = GameAlloc<ProxyProjectileHit>();

		// If the hit action has a ProxyProjectileHit VMT, don't hook the hit again.
		if (projectile->OnHitAction != nullptr &&
			*(void**)projectile->OnHitAction == *(void**)hitProxy) {
			GameDelete(hitProxy);
			return;
		}

		hitProxy->WrappedHit = projectile->OnHitAction;
		projectile->OnHitAction = hitProxy;
	}

	void CustomFunctionLibrary::OnActionMachineSetState(esv::ActionMachine * self, uint64_t actionLayer, esv::ActionState * actionState, int * somePtr, bool force, bool setLayer, bool succeeded)
	{
		if (!succeeded || actionState == nullptr || !setLayer) return;

		auto character = GetEntityWorld()->GetCharacter(self->CharacterHandle);
		if (character == nullptr) {
			OsiErrorS("ActionMachine has no character?");
			return;
		}

		auto type = actionState->GetType();
		auto typeName = actionState->GetTypeName();
		// Avoid event spam from idle/anim states
		if (type == esv::ActionStateType::ASIdle
			|| type == esv::ActionStateType::ASAnimation
			|| typeName == nullptr) {
			return;
		}

		auto eventArgs = OsiArgumentDesc::Create(OsiArgumentValue{ ValueType::GuidString, character->GetGuid()->Str });
		eventArgs->Add(OsiArgumentValue{ ValueType::String, typeName });

		gExtender->GetServer().Osiris().GetCustomFunctionInjector().ThrowEvent(ActionStateEnterHandle, eventArgs);

		delete eventArgs;
	}

	CDivinityStats_Object_Property_Data* CustomFunctionLibrary::OnParseSkillProperties(CRPGStatsManager::ParsePropertiesProc* next, CRPGStatsManager* self, STDString* str)
	{
		auto parsed = next(self, str);
		if (parsed != nullptr && parsed->TypeId == CRPGStats_Object_Property_Type::Status) {
			auto status = static_cast<CDivinityStats_Object_Property_Status*>(parsed);
			if (strncmp(status->Status.Str, "EXT:", 4) == 0) {
				auto extProp = GameAlloc<CRPGStats_Object_Property_Extender>();
				extProp->Context = status->Context;
				extProp->TypeId = CRPGStats_Object_Property_Type::Extender;
				extProp->Conditions = status->Conditions;
				extProp->PropertyName = FixedString(status->Status.Str + 4);
				extProp->Arg1 = status->StatusChance;
				extProp->Arg2 = status->Duration;
				extProp->Arg3 = status->StatsId;
				extProp->Arg4 = status->Arg4;
				extProp->Arg5 = status->Arg5;
				return extProp;
			}
		}

		return parsed;
	}

	void CustomFunctionLibrary::OnSkillFormatDescriptionParam(SkillPrototype::FormatDescriptionParamProc* next, SkillPrototype *skillPrototype,
		CDivinityStats_Character *tgtCharStats, eoc::Text *eocText, int paramIndex, bool isFromItem,
		float xmm9_4_0, FixedString * paramText, ObjectSet<STDString> * paramTexts)
	{
		// When fetching subproperties (recursively), paramTexts will be null.
		// We won't post these to Lua since the Lua scripts already processed the original (unwrapped) query
		if (paramTexts != nullptr) {
			ecl::LuaClientPin lua(ecl::ExtensionState::Get());
			if (lua) {
				auto replacement = lua->SkillGetDescriptionParam(skillPrototype, tgtCharStats, *paramTexts, isFromItem);
				if (replacement) {
					eocText->ReplaceParam(paramIndex, *replacement);
					return;
				}
			}
		}

		next(skillPrototype, tgtCharStats, eocText, paramIndex, isFromItem, xmm9_4_0, paramText, paramTexts);
	}

#if defined(OSI_EOCAPP)
	int CustomFunctionLibrary::OnGetSkillAPCost(SkillPrototype::GetAttackAPCostProc* next, SkillPrototype* self, CDivinityStats_Character* character, eoc::AiGrid* aiGrid,
		glm::vec3* position, float* radius, int* pElementalAffinity)
#else
	int CustomFunctionLibrary::OnGetSkillAPCost(SkillPrototype::GetAttackAPCostProc* next, SkillPrototype* self, CDivinityStats_Character* character, eoc::AiGrid* aiGrid,
		glm::vec3* position, float* radius, bool unused, int* pElementalAffinity)
#endif
	{
		LuaVirtualPin lua(gExtender->GetCurrentExtensionState());
		if (lua) {
			auto ap = lua->GetSkillAPCost(self, character, aiGrid, position, radius);
			if (ap) {
				if (pElementalAffinity != nullptr) {
					*pElementalAffinity = ap->second ? 1 : 0;
				}

				return ap->first;
			}
		}

#if defined(OSI_EOCAPP)
		return next(self, character, aiGrid, position, radius, pElementalAffinity);
#else
		return next(self, character, aiGrid, position, radius, unused, pElementalAffinity);
#endif
	}

	void CustomFunctionLibrary::OnStatusFormatDescriptionParam(StatusPrototype::FormatDescriptionParamProc* next, StatusPrototype *prototype,
		CRPGStats_ObjectInstance* owner, CRPGStats_ObjectInstance* statusSource, float multiplier,
		eoc::Text * text, int paramIndex, FixedString * param, ObjectSet<STDString> * paramSet)
	{
		ecl::LuaClientPin lua(ecl::ExtensionState::Get());
		if (lua) {
			auto replacement = lua->StatusGetDescriptionParam(prototype, owner, statusSource, *paramSet);
			if (replacement) {
				text->ReplaceParam(paramIndex, *replacement);
				return;
			}
		}

		next(prototype, owner, statusSource, multiplier, text, paramIndex, param, paramSet);
	}

	void CustomFunctionLibrary::OnUpdateTurnOrder(esv::TurnManager * self, uint8_t combatId)
	{
		LuaServerPin lua(ExtensionState::Get());
		if (lua) {
			lua->OnUpdateTurnOrder(self, combatId);
		}
	}


	void CustomFunctionLibrary::OnCreateUIObject(UIObjectManager* self, ObjectHandle* handle, unsigned int layerIndex,
		unsigned int creatorId, int flags, uint64_t resourceFlags, uint16_t playerId, ObjectHandle* result)
	{
		ecl::LuaClientPin lua(ecl::ExtensionState::Get());
		if (lua) {
			lua->OnCreateUIObject(*result);
		}
	}



	void CustomFunctionLibrary::OnExecutePropertyDataOnGroundHit(glm::vec3* position, uint64_t casterHandle,
		DamagePairList* damageList, CRPGStats_Object_Property_List* propertyList, DamageType damageType)
	{
		esv::LuaServerPin lua(esv::ExtensionState::Get());
		if (position && lua) {
			lua->OnExecutePropertyDataOnGroundHit(*position, ObjectHandle(casterHandle), damageList);
		}
	}


	void CustomFunctionLibrary::OnExecuteCharacterSetExtraProperties(CRPGStats_Object_Property_List* properties, uint64_t attackerHandle,
		ObjectSet<esv::Character*> const& targets, glm::vec3 const& impactOrigin, CRPGStats_Object_PropertyContext propertyContext,
		bool isFromItem, SkillPrototype* skillPrototype, HitDamageInfo* damageInfo, float statusStartTimer, esv::Character* refTarget,
		bool statusFlag0x40, float a12)
	{
		if (!properties || (unsigned)properties->AllPropertyContexts == 0
			|| properties->Properties.Primitives.Size == 0) {
			return;
		}

		esv::LuaServerPin lua(esv::ExtensionState::Get());
		if (lua) {
			for (auto prop : properties->Properties.Primitives) {
				if (prop->TypeId == CRPGStats_Object_Property_Type::Extender) {
					auto extProp = static_cast<CRPGStats_Object_Property_Extender*>(prop);
					for (auto const& target : targets) {
						ObjectHandle targetHandle;
						target->GetObjectHandle(targetHandle);
						lua->ExecutePropertyDataOnTarget(extProp, ObjectHandle{ attackerHandle }, targetHandle, 
							impactOrigin, isFromItem, skillPrototype, damageInfo);
					}
				}
			}
		}
	}


	void CustomFunctionLibrary::OnExecutePropertyDataOnPositionOnly(CRPGStats_Object_Property_List* properties, uint64_t attackerHandle,
		glm::vec3 const* position, float areaRadius, CRPGStats_Object_PropertyContext propertyContext, bool isFromItem,
		SkillPrototype* skillPrototype, HitDamageInfo* damageInfo, float unkn)
	{
		if (!properties || !position || (unsigned)properties->AllPropertyContexts == 0 
			|| properties->Properties.Primitives.Size == 0) {
			return;
		}

		esv::LuaServerPin lua(esv::ExtensionState::Get());
		if (lua) {
			for (auto prop : properties->Properties.Primitives) {
				if (prop->TypeId == CRPGStats_Object_Property_Type::Extender) {
					auto extProp = static_cast<CRPGStats_Object_Property_Extender*>(prop);
					lua->ExecutePropertyDataOnPosition(extProp, ObjectHandle{ attackerHandle }, *position, areaRadius, isFromItem,
						skillPrototype, damageInfo);
				}
			}
		}
	}


	esv::Item* CustomFunctionLibrary::OnGenerateTreasureItem(esv::ItemHelpers__GenerateTreasureItem* next,
		RPGStats_Treasure_Object_Info* treasureInfo, int level)
	{
		auto item = next(treasureInfo, level);

		if (item) {
			esv::LuaServerPin lua(esv::ExtensionState::Get());
			if (lua) {
				item = lua->OnGenerateTreasureItem(item);
			}
		}

		return item;
	}


	bool CustomFunctionLibrary::OnCraftingExecuteCombination(esv::CombineManager::ExecuteCombinationProc* next,
		esv::CombineManager* self, CraftingStationType craftingStation, ObjectSet<ObjectHandle>* ingredientHandles,
		esv::Character* character, uint8_t quantity, char openUI, FixedString* combinationId)
	{
		esv::LuaServerPin lua(esv::ExtensionState::Get());
		if (lua) {
			if (lua->OnBeforeCraftingExecuteCombination(craftingStation, *ingredientHandles, character, quantity, *combinationId)) {
				return true;
			}
		}

		bool ok = next(self, craftingStation, ingredientHandles, character, quantity, openUI, combinationId);

		if (lua) {
			lua->OnAfterCraftingExecuteCombination(craftingStation, *ingredientHandles, character, quantity, *combinationId, ok);
		}

		return ok;
	}

	void CustomFunctionLibrary::OnAppInputEvent(App* self, uint16_t& retval, InputEvent const& inputEvent)
	{
		ecl::LuaClientPin lua(ecl::ExtensionState::Get());
		if (lua) {
			lua->OnAppInputEvent(inputEvent);
		}
	}

	bool CustomFunctionLibrary::OnPeerModuleLoaded(LoadProtocol::HandleModuleLoadedProc* next, LoadProtocol* self,
		LoadProtocol::PeerModSettings& peerModSettings, ModuleSettings& hostModSettings)
	{
		auto validate = GetStaticSymbols().ModuleSettings__Validate;
		if (validate != nullptr
			// Don't validate local peer settings, as they can be wrong because of a bug with ArmorSets
			&& peerModSettings.peerId != 1) {
			ObjectSet<ModuleShortDesc> mismatches;

			bool hasEmptyHashes = false;
			for (auto const& mod : hostModSettings.Mods) {
				if (mod.MD5.empty()) {
					hasEmptyHashes = true;
				}
			}

			for (auto const& mod : peerModSettings.modSettings.Mods) {
				if (mod.MD5.empty()) {
					hasEmptyHashes = true;
				}
			}

			// Hashes work differently in vanilla / old extender, so ignore them if the peer has an old version
			auto peerVersion = gExtender->GetServer().GetNetworkManager().GetPeerVersion(peerModSettings.peerId);
			// TEMP WORKAROUND - disabled md5 checks until we can figure out why it fails randomly
			if (!peerVersion || *peerVersion < ScriptExtenderMessage::VerCorrectedHashes || hasEmptyHashes || true) {
				for (auto& mod : peerModSettings.modSettings.Mods) {
					mod.MD5 = "";
				}

				for (auto& mod : hostModSettings.Mods) {
					mod.MD5 = "";
				}
			}

			auto result = validate(hostModSettings, peerModSettings.modSettings, mismatches);

			if (result != 0) {
				STDWString userName = L"(Unknown)";
				auto gameServer = (*GetStaticSymbols().EoCServer)->GameServer;
				auto peer = gameServer->Peers.Find(peerModSettings.peerId);
				if (peer != nullptr) {
					peer->Users.Iterate([&userName](uint16_t const&, net::GameServer::UserInfo const& user) {
						userName = user.UserName;
					});
				}

				ERR("Validation rc = %d", result);

				INFO("Server modlist:");
				for (auto const& mod : hostModSettings.Mods) {
					INFO("%s - %s - %s - %d - %s", mod.ModuleUUID.Str, ToUTF8(mod.Folder).c_str(), ToUTF8(mod.Name).c_str(), mod.Version.Ver, mod.MD5.c_str());
				}

				INFO("Client modlist:");
				for (auto const& mod : peerModSettings.modSettings.Mods) {
					INFO("%s - %s - %s - %d - %s", mod.ModuleUUID.Str, ToUTF8(mod.Folder).c_str(), ToUTF8(mod.Name).c_str(), mod.Version.Ver, mod.MD5.c_str());
				}

				std::stringstream ss;
				switch (result) {
				case 2:
				{
					auto v0 = mismatches[0].Version;
					auto v1 = mismatches[1].Version;
					ss << "Client (" << ToUTF8(userName) << ") mod version of " << ToUTF8(mismatches[0].Name) << " (v"
						<< v1.Major() << "." << v1.Minor() << "." << v1.Revision() << "." << v1.Build()
						<< ") doesn't match servers (v" 
						<< v0.Major() << "." << v0.Minor() << "." << v0.Revision() << "." << v0.Build()
						<< ")";
					break;
				}

				case 3:
					ss << "Client (" << ToUTF8(userName) << ") mod hash of " << ToUTF8(mismatches[0].Name) << " (" << mismatches[1].MD5
						<< ") doesn't match servers (" << mismatches[0].MD5 << ")";
					break;

				case 1:
				default:
					ss << "Client (" << ToUTF8(userName) << ") mod settings doesn't match servers";
					break;
				}

				auto server = GetStaticSymbols().EoCServer;
				if (server != nullptr) {
					int32_t userId = peerModSettings.peerId << 16;
					// We use the vanilla HostRefuse message for non-extender clients or old extender
					// versions that don't support the kick message
					if (peerVersion && *peerVersion >= ScriptExtenderMessage::VerAddedKickMessage) {
						auto msg = gExtender->GetServer().GetNetworkManager().GetFreeMessage(UserId(userId));
						if (msg != nullptr) {
							auto postMsg = msg->GetMessage().mutable_s2c_kick();
							postMsg->set_message(ss.str().c_str());
							gExtender->GetServer().GetNetworkManager().Send(msg, UserId(userId));
						} else {
							OsiErrorS("Could not get free message!");
						}
					} else {
						auto host = (*server)->GameServer;
						auto msg = host->GetFreeMessage<net::HostRefuseMessage>();
						msg->ReasonCode = 4;
						host->VMT->SendToPeer(host, &userId, msg);
					}
				}

				gExtender->GetLibraryManager().ShowStartupError(FromUTF8(ss.str().c_str()), false);
			}
		}

		// Sync all dynamic stats to new peer as we didn't do so during savegame loading
		GetStaticSymbols().GetStats()->BroadcastSyncAll();

		return next(self, peerModSettings, hostModSettings);
	}


	void CustomFunctionLibrary::RegisterStatusFunctions()
	{
		auto & functionMgr = osiris_.GetCustomFunctionManager();

		auto iterateCharacterStatuses = std::make_unique<CustomCall>(
			"NRD_IterateStatuses",
			std::vector<CustomFunctionParam>{
				{ "ObjectGuid", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "Event", ValueType::String, FunctionArgumentDirection::In }
			},
			&func::IterateStatuses
		);
		functionMgr.Register(std::move(iterateCharacterStatuses));

		auto hasStatusType = std::make_unique<CustomQuery>(
			"NRD_ObjectHasStatusType",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusType", ValueType::String, FunctionArgumentDirection::In },
				{ "HasStatus", ValueType::Integer, FunctionArgumentDirection::Out },
			},
			&func::ObjectHasStatusType
		);
		functionMgr.Register(std::move(hasStatusType));

		auto getStatusHandle = std::make_unique<CustomQuery>(
			"NRD_StatusGetHandle",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusId", ValueType::String, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::Out },
			},
			&func::StatusGetHandle
		);
		functionMgr.Register(std::move(getStatusHandle));

		auto getStatusAttributeInt = std::make_unique<CustomQuery>(
			"NRD_StatusGetInt",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::Integer, FunctionArgumentDirection::Out },
			},
			&StatusGetAttribute<OsiPropertyMapType::Integer>
		);
		functionMgr.Register(std::move(getStatusAttributeInt));

		auto getStatusAttributeReal = std::make_unique<CustomQuery>(
			"NRD_StatusGetReal",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::Real, FunctionArgumentDirection::Out },
			},
			&StatusGetAttribute<OsiPropertyMapType::Real>
		);
		functionMgr.Register(std::move(getStatusAttributeReal));

		auto getStatusAttributeString = std::make_unique<CustomQuery>(
			"NRD_StatusGetString",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::String, FunctionArgumentDirection::Out },
			},
			&StatusGetAttribute<OsiPropertyMapType::String>
		);
		functionMgr.Register(std::move(getStatusAttributeString));

		auto getStatusAttributeGuidString = std::make_unique<CustomQuery>(
			"NRD_StatusGetGuidString",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::GuidString, FunctionArgumentDirection::Out },
			},
			&StatusGetAttribute<OsiPropertyMapType::GuidStringHandle>
		);
		functionMgr.Register(std::move(getStatusAttributeGuidString));

		auto setStatusAttributeInt = std::make_unique<CustomCall>(
			"NRD_StatusSetInt",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::Integer, FunctionArgumentDirection::In },
			},
			&StatusSetAttribute<OsiPropertyMapType::Integer>
		);
		functionMgr.Register(std::move(setStatusAttributeInt));

		auto setStatusAttributeReal = std::make_unique<CustomCall>(
			"NRD_StatusSetReal",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::Real, FunctionArgumentDirection::In },
			},
			&StatusSetAttribute<OsiPropertyMapType::Real>
		);
		functionMgr.Register(std::move(setStatusAttributeReal));

		auto setStatusAttributeString = std::make_unique<CustomCall>(
			"NRD_StatusSetString",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::String, FunctionArgumentDirection::In },
			},
			&StatusSetAttribute<OsiPropertyMapType::String>
		);
		functionMgr.Register(std::move(setStatusAttributeString));

		auto setStatusAttributeGuidString = std::make_unique<CustomCall>(
			"NRD_StatusSetGuidString",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::GuidString, FunctionArgumentDirection::In },
			},
			&StatusSetAttribute<OsiPropertyMapType::GuidStringHandle>
		);
		functionMgr.Register(std::move(setStatusAttributeGuidString));

		auto setStatusAttributeVector3 = std::make_unique<CustomCall>(
			"NRD_StatusSetVector3",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Attribute", ValueType::String, FunctionArgumentDirection::In },
				{ "X", ValueType::Real, FunctionArgumentDirection::In },
				{ "Y", ValueType::Real, FunctionArgumentDirection::In },
				{ "Z", ValueType::Real, FunctionArgumentDirection::In },
			},
			&StatusSetAttribute<OsiPropertyMapType::Vector3>
		);
		functionMgr.Register(std::move(setStatusAttributeVector3));

		auto statusPreventApply = std::make_unique<CustomCall>(
			"NRD_StatusPreventApply",
			std::vector<CustomFunctionParam>{
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "PreventApply", ValueType::Integer, FunctionArgumentDirection::In }
			},
			&func::StatusPreventApply
		);
		functionMgr.Register(std::move(statusPreventApply));

		auto statusIteratorEvent = std::make_unique<CustomEvent>(
			"NRD_StatusIteratorEvent",
			std::vector<CustomFunctionParam>{
				{ "Event", ValueType::String, FunctionArgumentDirection::In },
				{ "Object", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusId", ValueType::String, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
			}
		);
		StatusIteratorEventHandle = functionMgr.Register(std::move(statusIteratorEvent));

		auto statusAttemptEvent = std::make_unique<CustomEvent>(
			"NRD_OnStatusAttempt",
			std::vector<CustomFunctionParam>{
				{ "Target", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "StatusId", ValueType::String, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
				{ "Instigator", ValueType::GuidString, FunctionArgumentDirection::In },
			}
		);
		StatusAttemptEventHandle = functionMgr.Register(std::move(statusAttemptEvent));

		auto applyActiveDefense = std::make_unique<CustomQuery>(
			"NRD_ApplyActiveDefense",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "StatusId", ValueType::String, FunctionArgumentDirection::In },
				{ "LifeTime", ValueType::Real, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::Out },
			},
			&func::ApplyActiveDefense
		);
		functionMgr.Register(std::move(applyActiveDefense));

		auto applyDamageOnMove = std::make_unique<CustomQuery>(
			"NRD_ApplyDamageOnMove",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "StatusId", ValueType::String, FunctionArgumentDirection::In },
				{ "SourceCharacter", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "LifeTime", ValueType::Real, FunctionArgumentDirection::In },
				{ "DistancePerDamage", ValueType::Real, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::Out },
			},
			&func::ApplyDamageOnMove
		);
		functionMgr.Register(std::move(applyDamageOnMove));

		auto hitPrepareEvent = std::make_unique<CustomEvent>(
			"NRD_OnPrepareHit",
			std::vector<CustomFunctionParam>{
				{ "Target", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "Instigator", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "Damage", ValueType::Integer, FunctionArgumentDirection::In },
				{ "HitHandle", ValueType::Integer64, FunctionArgumentDirection::In },
			}
		);
		HitPrepareEventHandle = functionMgr.Register(std::move(hitPrepareEvent));

		auto hitEvent = std::make_unique<CustomEvent>(
			"NRD_OnHit",
			std::vector<CustomFunctionParam>{
				{ "Target", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "Instigator", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "Damage", ValueType::Integer, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
			}
		);
		HitEventHandle = functionMgr.Register(std::move(hitEvent));

		auto healEvent = std::make_unique<CustomEvent>(
			"NRD_OnHeal",
			std::vector<CustomFunctionParam>{
				{ "Target", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Instigator", ValueType::GuidString, FunctionArgumentDirection::In },
				{ "Amount", ValueType::Integer, FunctionArgumentDirection::In },
				{ "StatusHandle", ValueType::Integer64, FunctionArgumentDirection::In },
			}
		);
		HealEventHandle = functionMgr.Register(std::move(healEvent));


		auto actionStateEnterEvent = std::make_unique<CustomEvent>(
			"NRD_OnActionStateEnter",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Action", ValueType::String, FunctionArgumentDirection::In }
			}
		);
		ActionStateEnterHandle = functionMgr.Register(std::move(actionStateEnterEvent));

		auto actionStateExitEvent = std::make_unique<CustomEvent>(
			"NRD_OnActionStateExit",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Action", ValueType::String, FunctionArgumentDirection::In }
			}
		);
		ActionStateExitHandle = functionMgr.Register(std::move(actionStateExitEvent));

		auto characterGetCurrentAction = std::make_unique<CustomQuery>(
			"NRD_CharacterGetCurrentAction",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Action", ValueType::String, FunctionArgumentDirection::Out }
			},
			&func::CharacterGetCurrentAction
		);
		functionMgr.Register(std::move(characterGetCurrentAction));

		auto actionStateGetInt = std::make_unique<CustomQuery>(
			"NRD_ActionStateGetInt",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Property", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::Integer, FunctionArgumentDirection::Out }
			},
			&func::ActionStateGet<OsiPropertyMapType::Integer>
		);
		functionMgr.Register(std::move(actionStateGetInt));

		auto actionStateGetReal = std::make_unique<CustomQuery>(
			"NRD_ActionStateGetReal",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Property", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::Real, FunctionArgumentDirection::Out }
			},
			&func::ActionStateGet<OsiPropertyMapType::Real>
		);
		functionMgr.Register(std::move(actionStateGetReal));

		auto actionStateGetString = std::make_unique<CustomQuery>(
			"NRD_ActionStateGetString",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Property", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::String, FunctionArgumentDirection::Out }
			},
			&func::ActionStateGet<OsiPropertyMapType::String>
		);
		functionMgr.Register(std::move(actionStateGetString));

		auto actionStateGetGuidString = std::make_unique<CustomQuery>(
			"NRD_ActionStateGetGuidString",
			std::vector<CustomFunctionParam>{
				{ "Character", ValueType::CharacterGuid, FunctionArgumentDirection::In },
				{ "Property", ValueType::String, FunctionArgumentDirection::In },
				{ "Value", ValueType::GuidString, FunctionArgumentDirection::Out }
			},
			&func::ActionStateGet<OsiPropertyMapType::GuidStringHandle>
		);
		functionMgr.Register(std::move(actionStateGetGuidString));
	}

}
