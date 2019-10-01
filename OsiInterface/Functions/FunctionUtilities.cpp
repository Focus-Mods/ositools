#include <stdafx.h>
#include "FunctionLibrary.h"
#include <OsirisProxy.h>

namespace osidbg
{
	FixedString ToFixedString(const char * s)
	{
		auto stringTable = gOsirisProxy->GetLibraryManager().GetGlobalStringTable();
		if (stringTable == nullptr) {
			Debug("ToFixedString(): Global string table not available!");
			return FixedString(nullptr);
		}

		auto str = stringTable->Find(s, strlen(s));
		return FixedString(str);
	}

	char const * NameGuidToFixedString(std::string const & nameGuid)
	{
		if (nameGuid.size() < 36) {
			Debug("NameGuidToFixedString(): GUID (%s) too short!", nameGuid.c_str());
			return nullptr;
		}

		auto guid = nameGuid.substr(nameGuid.size() - 36, 36);

		auto stringTable = gOsirisProxy->GetLibraryManager().GetGlobalStringTable();
		if (stringTable == nullptr) {
			Debug("NameGuidToFixedString(): Global string table not available!");
			return nullptr;
		}

		return stringTable->Find(guid.c_str(), guid.size());
	}

	EsvCharacter * FindCharacterByNameGuid(std::string const & nameGuid)
	{
		auto stringPtr = NameGuidToFixedString(nameGuid);
		if (stringPtr == nullptr) {
			OsiError("Could not map GUID '" << nameGuid << "' to FixedString");
			return nullptr;
		}

		auto charFactory = gOsirisProxy->GetLibraryManager().GetCharacterFactory();
		if (charFactory == nullptr) {
			OsiError("CharacterFactory not available!");
			return nullptr;
		}

		FixedString fs;
		fs.Str = stringPtr;

		auto component = charFactory->Entities->Components.Buf[0x0E].component->FindComponentByGuid(&fs);
		if (component != nullptr) {
			return (EsvCharacter *)((uint8_t *)component - 8);
		}
		else {
			OsiError("No Character component found with GUID '" << nameGuid << "'");
			return nullptr;
		}
	}

	EsvCharacter * FindCharacterByHandle(ObjectHandle const & handle)
	{
		auto charFactory = gOsirisProxy->GetLibraryManager().GetCharacterFactory();
		if (charFactory == nullptr) {
			OsiError("CharacterFactory not available!");
			return nullptr;
		}

		auto component = charFactory->Entities->Components.Buf[0x0E].component->FindComponentByHandle(&handle);
		if (component != nullptr) {
			return (EsvCharacter *)((uint8_t *)component - 8);
		}
		else {
			OsiError("No Character component found with this handle (0x" << std::hex << handle.Handle << ")");
			return nullptr;
		}
	}

	EsvItem * FindItemByNameGuid(std::string const & nameGuid)
	{
		auto stringPtr = NameGuidToFixedString(nameGuid);
		if (stringPtr == nullptr) {
			OsiError("Could not map GUID '" << nameGuid << "' to FixedString");
			return nullptr;
		}

		auto itemFactory = gOsirisProxy->GetLibraryManager().GetItemFactory();
		if (itemFactory == nullptr) {
			OsiError("ItemFactory not available!");
			return nullptr;
		}

		FixedString fs;
		fs.Str = stringPtr;

		auto component = itemFactory->Entities->Components.Buf[0x0D].component->FindComponentByGuid(&fs);
		if (component != nullptr) {
			return (EsvItem *)((uint8_t *)component - 8);
		}
		else {
			OsiError("No Item component found with GUID '" << nameGuid << "'");
			return nullptr;
		}
	}

	EsvItem * FindItemByHandle(ObjectHandle const & handle)
	{
		auto itemFactory = gOsirisProxy->GetLibraryManager().GetItemFactory();
		if (itemFactory == nullptr) {
			OsiError("ItemFactory not available!");
			return nullptr;
		}

		auto component = itemFactory->Entities->Components.Buf[0x0D].component->FindComponentByHandle(&handle);
		if (component != nullptr) {
			return (EsvItem *)((uint8_t *)component - 8);
		}
		else {
			OsiError("No Item component found with this handle (0x" << std::hex << handle.Handle << ")");
			return nullptr;
		}
	}

	EsvGameAction * FindGameActionByHandle(ObjectHandle const & handle)
	{
		auto const & lib = gOsirisProxy->GetLibraryManager();
		auto actionMgr = lib.GetGameActionManager();

		for (uint32_t i = 0; i < actionMgr->GameActions.Size; i++) {
			auto action = actionMgr->GameActions.Buf[i];
			if (action->MyHandle == handle) {
				return action;
			}
		}

		OsiError("Nm game action found with handle " << (int64_t)handle);
		return nullptr;
	}

	ShootProjectileApiHelper::ShootProjectileApiHelper()
	{
		Helper.Random = (uint8_t)rand();
		Helper.DamageList = nullptr;
		Helper.CasterLevel = -1;
		Helper.CollisionInfo = nullptr;
		Helper.IsTrap = false;
		Helper.UnknownFlag1 = true;
		Helper.StatusClearChance = 0.0f;
		Helper.HasCaster = false;
		Helper.IsStealthed = false;
		Helper.UnknownFlag2 = false;
	}

	void ShootProjectileApiHelper::SetInt(char const * prop, int32_t value)
	{
		if (strcmp(prop, "CasterLevel") == 0) {
			Helper.CasterLevel = value;
		}
		else if (strcmp(prop, "StatusClearChance") == 0) {
			Helper.StatusClearChance = value / 100.0f;
		}
		else if (strcmp(prop, "IsTrap") == 0) {
			Helper.IsTrap = value ? 1 : 0;
		}
		else if (strcmp(prop, "UnknownFlag1") == 0) {
			Helper.UnknownFlag1 = value ? 1 : 0;
		}
		else if (strcmp(prop, "HasCaster") == 0) {
			Helper.HasCaster = value ? 1 : 0;
		}
		else if (strcmp(prop, "IsStealthed") == 0) {
			Helper.IsStealthed = value ? 1 : 0;
		}
		else if (strcmp(prop, "UnknownFlag2") == 0) {
			Helper.UnknownFlag2 = value ? 1 : 0;
		}
		else {
			OsiError("Unknown int property: '" << prop << "'");
		}
	}

	void ShootProjectileApiHelper::SetGuidString(char const * prop, char const * value)
	{
		auto character = FindCharacterByNameGuid(value);
		auto item = FindItemByNameGuid(value);

		if (character == nullptr && item == nullptr) {
			OsiError("GUID '" << value << "' is not a valid item or object");
			return;
		}

		ObjectHandle handle;
		glm::vec3 position;

		if (character != nullptr) {
			character->GetObjectHandle(&handle);
			position = character->WorldPos;
		}
		else
		{
			item->GetObjectHandle(&handle);
			position = item->WorldPos;
		}

		if (strcmp(prop, "SourcePosition") == 0) {
			Helper.StartPosition = position;
			HasStartPosition = true;
		}
		else if (strcmp(prop, "TargetPosition") == 0) {
			Helper.EndPosition = position;
			HasEndPosition = true;
		}
		else if (strcmp(prop, "Source") == 0) {
			if (character != nullptr) {
				Helper.SourceCharacter = handle;
			}
		}
		else if (strcmp(prop, "Target") == 0) {
			Helper.Target = handle;
		}
		else if (strcmp(prop, "Target2") == 0) {
			Helper.Target2 = handle;
		}
		else {
			OsiError("Unknown object property '" << prop << "'");
		}
	}

	void ShootProjectileApiHelper::SetVector(char const * prop, glm::vec3 const & value)
	{
		if (strcmp(prop, "SourcePosition") == 0) {
			Helper.StartPosition = value;
			HasStartPosition = true;
		}
		else if (strcmp(prop, "TargetPosition") == 0) {
			Helper.EndPosition = value;
			HasEndPosition = true;
		}
		else {
			OsiError("Unknown vector3 property '" << prop << "'");
		}
	}

	void ShootProjectileApiHelper::SetString(char const * prop, char const * value)
	{
		auto fs = ToFixedString(value);
		if (!fs) {
			OsiError("Could not map value '" << value << "' to FixedString");
			return;
		}

		if (strcmp(prop, "SkillId") == 0) {
			Helper.SkillId = fs;
		}
		else if (strcmp(prop, "FS2") == 0) {
			Helper.FS2 = fs;
		}
		else {
			OsiError("Unknown string property '" << prop << "'");
		}
	}

	bool ShootProjectileApiHelper::Shoot()
	{
		if (!HasStartPosition) {
			OsiError("No start position!");
			return false;
		}

		if (!HasEndPosition) {
			OsiError("No end position!");
			return false;
		}

		if (!Helper.SkillId.Str) {
			OsiError("No skill id!");
			return false;
		}

		auto shoot = gOsirisProxy->GetLibraryManager().ShootProjectile;
		if (shoot == nullptr) {
			OsiError("ShootProjectile helper not found!");
			return false;
		}

		shoot(&Helper, Helper.StatusClearChance);
		return true;
	}
}
