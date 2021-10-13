#include <stdafx.h>
#include "FunctionLibrary.h"
#include <OsirisProxy.h>
#include <GameDefinitions/Projectile.h>
#include <GameDefinitions/Symbols.h>

namespace dse
{
	ecl::EoCClient * GetEoCClient()
	{
		auto clientPtr = GetStaticSymbols().EoCClient;
		if (clientPtr == nullptr || *clientPtr == nullptr) {
			return nullptr;
		} else {
			return *clientPtr;
		}
	}

	ModManager * GetModManagerClient()
	{
		auto client = GetEoCClient();
		if (client == nullptr || client->ModManager == nullptr) {
			return nullptr;
		} else {
			return client->ModManager;
		}
	}

	esv::EoCServer * GetEoCServer()
	{
		auto serverPtr = GetStaticSymbols().EoCServer;
		if (serverPtr == nullptr || *serverPtr == nullptr) {
			return nullptr;
		} else {
			return *serverPtr;
		}
	}

	ModManager * GetModManagerServer()
	{
		auto server = GetEoCServer();
		if (server == nullptr || server->ModManagerServer == nullptr) {
			return nullptr;
		} else {
			return server->ModManagerServer;
		}
	}

	FixedString ToFixedString(const char * s)
	{
		if (s == nullptr) {
			OsiErrorS("Attempted to look up a null string!");
			return FixedString{};
		}

		auto stringTable = GetStaticSymbols().GetGlobalStringTable();
		if (stringTable == nullptr) {
			OsiErrorS("Global string table not available!");
			return FixedString{};
		}

		auto str = stringTable->Find(s, strlen(s));
		return FixedString(str, FixedString::FromPool{});
	}

	FixedString MakeFixedString(const char * s)
	{
		auto str = ToFixedString(s);
		if (!str) {
			auto createFixedString = GetStaticSymbols().CreateFixedString;
			if (createFixedString != nullptr) {
#if defined(OSI_EOCAPP)
				str = FixedString(createFixedString(s, -1), FixedString::FromPool{});
#else
				createFixedString(&str, s, -1);
#endif
				if (!str) {
					OsiErrorS("Failed to register FixedString in global string table?!");
				}
			} else {
				OsiErrorS("ls::FixedString::Create not available!");
			}
		}

		return str;
	}

	bool IsHexChar(char c)
	{
		return (c >= '0' && c <= '9')
			|| (c >= 'a' && c <= 'f');
	}

	bool IsValidGuidString(const char * s)
	{
		auto len = strlen(s);
		if (len < 36) return false;

		auto guidPos = len - 36;
		unsigned const nibbles[5] = { 8, 4, 4, 4, 12 };

		for (auto n = 0; n < 5; n++) {
			for (unsigned i = 0; i < nibbles[n]; i++) {
				if (!IsHexChar(s[guidPos++])) return false;
			}

			if (n != 4 && s[guidPos++] != '-') return false;
		}

		return true;
	}

	FixedString NameGuidToFixedString(char const * nameGuid)
	{
		if (nameGuid == nullptr) {
			return FixedString{};
		}

		auto nameLen = strlen(nameGuid);
		if (!IsValidGuidString(nameGuid)) {
			OsiError("GUID (" << nameGuid << ") malformed!");
			return FixedString{};
		}

		auto guid = nameGuid + nameLen - 36;

		auto stringTable = GetStaticSymbols().GetGlobalStringTable();
		if (stringTable == nullptr) {
			OsiErrorS("Global string table not available!");
			return FixedString{};
		}

		return FixedString(stringTable->Find(guid, 36), FixedString::FromPool{});
	}

	namespace esv
	{
		IEoCServerObject* EntityWorld::GetGameObject(char const* nameGuid, bool logError)
		{
			if (nameGuid == nullptr) {
				OsiError("Attempted to look up object with null name!");
				return nullptr;
			}

			auto character = GetCharacter(nameGuid, false);
			if (character != nullptr) {
				return character;
			}

			auto item = GetItem(nameGuid, false);
			if (item != nullptr) {
				return item;
			}

			if (logError) {
				OsiError("No EoC server object found with GUID '" << nameGuid << "'");
			}

			return nullptr;
		}

		IEoCServerObject* EntityWorld::GetGameObject(ObjectHandle handle, bool logError)
		{
			if (!handle) {
				return nullptr;
			}

			switch ((ObjectType)handle.GetType()) {
			case ObjectType::ServerCharacter:
				return GetCharacter(handle, logError);

			case ObjectType::ServerItem:
				return GetItem(handle, logError);

			case ObjectType::ServerProjectile:
				return GetProjectile(handle, logError);

			default:
				OsiError("GameObjects with handle type " << handle.GetType() << " not supported!");
				return nullptr;
			}
		}

		EntityWorld* GetEntityWorld()
		{
			auto server = GetEoCServer();
			if (server == nullptr) {
				OsiErrorS("EoCServer not available!");
				return nullptr;
			} else {
				return server->EntityWorld;
			}
		}

		esv::TurnBasedProtocol * GetTurnBasedProtocol()
		{
			auto server = GetStaticSymbols().EoCServer;
			if (server == nullptr || *server == nullptr) return nullptr;

			auto gameServer = (*server)->GameServer;
			if (gameServer == nullptr) return nullptr;

			if (gameServer->Protocols.Size <= 22) return nullptr;
			return (esv::TurnBasedProtocol *)gameServer->Protocols[22];
		}

		esv::Inventory * FindInventoryByHandle(ObjectHandle const & handle, bool logError)
		{
			auto inventoryMgr = GetStaticSymbols().GetServerInventoryFactory();
			if (inventoryMgr != nullptr) {
				return inventoryMgr->Get(handle);
			} else {
				return nullptr;
			}
		}

		esv::GameAction * FindGameActionByHandle(ObjectHandle const & handle)
		{
			auto actionMgr = GetStaticSymbols().GetGameActionManager();

			for (auto action : actionMgr->GameActions) {
				if (action->MyHandle == handle) {
					return action;
				}
			}

			return nullptr;
		}

		ShootProjectileApiHelper::ShootProjectileApiHelper()
		{
			Helper.Random = (uint8_t)rand();
			Helper.DamageList = nullptr;
			Helper.CasterLevel = -1;
			Helper.HitObject = nullptr;
			Helper.IsTrap = false;
			Helper.UnknownFlag1 = true;
			Helper.StatusClearChance = 0.0f;
			Helper.IsFromItem = false;
			Helper.IsStealthed = false;
			Helper.IgnoreObjects = false;
		}

		void ShootProjectileApiHelper::SetGuidString(FixedString const& prop, char const * value)
		{
			auto obj = GetEntityWorld()->GetGameObject(value);

			if (obj == nullptr) {
				OsiError("GUID '" << value << "' is not a valid item or object");
				return;
			}

			ObjectHandle handle;
			obj->GetObjectHandle(handle);
			glm::vec3 position = reinterpret_cast<esv::Character*>(obj)->WorldPos;

			if (prop == GFS.strSourcePosition) {
				Helper.StartPosition = position;
				HasStartPosition = true;
			}
			else if (prop == GFS.strTargetPosition) {
				Helper.EndPosition = position;
				HasEndPosition = true;
			}
			else if (prop == GFS.strHitObjectPosition) {
				HitObject.Position = position;
				HasHitObjectPosition = true;
			}
			else if (prop == GFS.strCaster) {
				Helper.Caster = handle;
			}
			else if (prop == GFS.strSource) {
				Helper.Source = handle;
			}
			else if (prop == GFS.strTarget) {
				Helper.Target = handle;
			}
			else if (prop == GFS.strHitObject) {
				HitObject.Target = handle;
				HasHitObject = true;
			}
			else {
				OsiError("Unknown object property '" << prop << "'");
			}
		}

		void ShootProjectileApiHelper::SetVector(FixedString const& prop, glm::vec3 const & value)
		{
			if (prop == GFS.strSourcePosition) {
				Helper.StartPosition = value;
				HasStartPosition = true;
			}
			else if (prop == GFS.strTargetPosition) {
				Helper.EndPosition = value;
				HasEndPosition = true;
			}
			else if (prop == GFS.strHitObjectPosition) {
				HitObject.Position = value;
				HasHitObjectPosition = true;
			}
			else {
				OsiError("Unknown vector3 property '" << prop << "'");
			}
		}

		void ShootProjectileApiHelper::AddDamage(DamageType type, int32_t amount)
		{
			DamageList.AddDamage(type, amount);
			HasDamageList = true;
		}

		bool ShootProjectileApiHelper::Shoot()
		{
			if (!HasStartPosition) {
				OsiErrorS("No start position!");
				return false;
			}

			if (!HasEndPosition) {
				OsiErrorS("No end position!");
				return false;
			}

			if (!Helper.SkillId) {
				OsiErrorS("No skill id!");
				return false;
			}

			if (HasHitObject)
			{
				Helper.HitObject = &HitObject;
				if (!HasHitObjectPosition) {
					HitObject.Position = Helper.StartPosition;
				}
			}

			if (HasDamageList) {
				Helper.DamageList = &DamageList;
			}

			auto shoot = GetStaticSymbols().esv__ProjectileHelpers__ShootProjectile;
			if (shoot == nullptr) {
				OsiErrorS("ShootProjectile helper not found!");
				return false;
			}

			auto projectile = shoot(&Helper);
			if (projectile) {
				projectile->AlwaysDamage = AlwaysDamage;
				projectile->CanDeflect = CanDeflect;
				return true;
			} else {
				return false;
			}
		}
	}

	namespace ecl
	{
		EntityWorld* GetEntityWorld()
		{
			auto client = GetEoCClient();
			if (client == nullptr) {
				OsiErrorS("GetEoCClient not available!");
				return nullptr;
			}
			else {
				return client->EntityWorld;
			}
		}

		ecl::Inventory* FindInventoryByHandle(ObjectHandle const& handle, bool logError)
		{
			auto inventoryMgr = GetStaticSymbols().GetClientInventoryFactory();
			if (inventoryMgr != nullptr) {
				return inventoryMgr->Get(handle);
			}
			else {
				return nullptr;
			}
		}
	}
}
