#pragma once

#include <GameDefinitions/Base/Base.h>
#include <GameDefinitions/Enumerations.h>

namespace dse
{

	namespace esv
	{
		struct GameAction : public ProtectedGameObject<GameAction>
		{
			void * VMT;
			FixedString SomeFS;
			NetId NetID;
			ObjectSet<uint16_t> PeerIDClassNames;
			ComponentHandle MyHandle;
			GameActionType ActionType;
			bool Active;
			bool Dirty;
			float ActivateTimer;
		};

		struct GameActionManager : public ProtectedGameObject<GameActionManager>
		{
			uint8_t Unmapped[0x130];
			Set<GameAction *> GameActions;
		};

		struct TornadoAction : public GameAction
		{
			FixedString SkillId;
			ComponentHandle OwnerHandle;
			glm::vec3 Position;
			glm::vec3 Target;
			float TurnTimer;
			bool Finished;
			bool IsFromItem;
			float HitRadius;
			uint64_t _Unk2;
			ObjectSet<glm::vec3> AnchorList;
			uint64_t Anchor;
			float Interpolation;
			uint32_t _Unk3;
			ComponentHandle SurfaceActionHandle;
			ObjectSet<ComponentHandle> HitCharacterHandles;
			ObjectSet<ComponentHandle> HitItemHandles;
			FixedString CleanseStatuses;
			float StatusClearChance;
		};

		struct StormAction : public GameAction
		{
			ComponentHandle OwnerHandle;
			glm::vec3 Position;
			float LifeTime;
			FixedString SkillId;
			float TurnTimer;
			float StrikeTimer;
			bool Finished;
			bool IsFromItem;
			uint64_t Unkn[3 * 5];
			ObjectSet<FixedString> FSSet;
			ObjectSet<void *> ProjectileTargetDescSet;
			ObjectSet<void *> StrikeSet;
		};

		struct RainAction : public GameAction
		{
			ComponentHandle OwnerHandle;
			glm::vec3 Position;
			float AreaRadius;
			float LifeTime;
			float Duration;
			bool FirstTick;
			FixedString SkillId;
			float ConsequencesStartTime;
			float TurnTimer;
			bool Finished;
			bool IsFromItem;
			FixedString SkillProperties;
		};

		struct WallAction : public GameAction
		{
			FixedString SkillId;
			ComponentHandle OwnerHandle;
			glm::vec3 Target;
			glm::vec3 Source;
			float LifeTime;
			ObjectSet<void *> Walls;
			float TurnTimer;
			bool Finished;
			bool IsFromItem;
			uint64_t Unk1;
			uint64_t Unk2;
			uint32_t Unk3;
			uint32_t Unk4;
		};

		struct StatusDomeAction : public GameAction
		{
			ComponentHandle OwnerHandle;
			glm::vec3 Position;
			uint8_t _Pad3[4];
			float LifeTime;
			FixedString SkillId;
			bool Finished;
			void * SkillStatusAura;
		};

		struct GameObjectMoveAction : public GameAction
		{
			void * PathMover;
			ComponentHandle ObjectToMove;
			bool DoneMoving;
			uint32_t U3[2];
			ComponentHandle CasterCharacterHandle;
			FixedString BeamEffectName;
			ComponentHandle SomeHandle;
			uint64_t _U4[8];
		};

		typedef void * (*GameActionManager__CreateAction)(GameActionManager * GameActionManager, GameActionType actionId, uint64_t SomeHandle);
#if defined(OSI_EOCAPP)
		typedef void(*GameActionManager__AddAction)(Set<GameAction *> * GameActionManager, esv::GameAction ** Action);
#else
		typedef void(*GameActionManager__AddAction)(GameActionManager * GameActionManager, esv::GameAction * Action);
#endif
		typedef void(*TornadoAction__Setup)(void * TornadoAction);
		typedef void(*GameObjectMoveAction__Setup)(void * Action, ComponentHandle & ObjectToMove, glm::vec3 * TargetPosition);

	}
}