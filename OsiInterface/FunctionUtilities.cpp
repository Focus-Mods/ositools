#include "stdafx.h"
#include "FunctionLibrary.h"
#include "OsirisProxy.h"
#include <random>

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
			Debug("FindCharacterByNameGuid(): Could not map GUID (%s) to FixedString", nameGuid.c_str());
			return nullptr;
		}

		auto charFactory = gOsirisProxy->GetLibraryManager().GetCharacterFactory();
		if (charFactory == nullptr) {
			Debug("FindCharacterByNameGuid(): CharacterFactory not available!");
			return nullptr;
		}

		FixedString fs;
		fs.Str = stringPtr;

		auto component = charFactory->Entities->Components.Buf[0x0E].component->FindComponentByGuid(&fs);
		if (component != nullptr) {
			return (EsvCharacter *)((uint8_t *)component - 8);
		}
		else {
			Debug("FindCharacterByNameGuid(): No Character component found with this GUID (%s)", nameGuid.c_str());
			return nullptr;
		}
	}

	EsvCharacter * FindCharacterByHandle(ObjectHandle const & handle)
	{
		auto charFactory = gOsirisProxy->GetLibraryManager().GetCharacterFactory();
		if (charFactory == nullptr) {
			Debug("FindCharacterByHandle(): CharacterFactory not available!");
			return nullptr;
		}

		auto component = charFactory->Entities->Components.Buf[0x0E].component->FindComponentByHandle(&handle);
		if (component != nullptr) {
			return (EsvCharacter *)((uint8_t *)component - 8);
		}
		else {
			Debug("FindCharacterByHandle(): No Character component found with this handle (%x)", handle.Handle);
			return nullptr;
		}
	}

	EsvItem * FindItemByNameGuid(std::string const & nameGuid)
	{
		auto stringPtr = NameGuidToFixedString(nameGuid);
		if (stringPtr == nullptr) {
			Debug("FindItemByNameGuid(): Could not map GUID (%s) to FixedString", nameGuid.c_str());
			return nullptr;
		}

		auto itemFactory = gOsirisProxy->GetLibraryManager().GetItemFactory();
		if (itemFactory == nullptr) {
			Debug("FindItemByNameGuid(): ItemFactory not available!");
			return nullptr;
		}

		FixedString fs;
		fs.Str = stringPtr;

		auto component = itemFactory->Entities->Components.Buf[0x0D].component->FindComponentByGuid(&fs);
		if (component != nullptr) {
			return (EsvItem *)((uint8_t *)component - 8);
		}
		else {
			Debug("FindItemByNameGuid(): No Item component found with this GUID (%s)", nameGuid.c_str());
			return nullptr;
		}
	}

	EsvItem * FindItemByHandle(ObjectHandle const & handle)
	{
		auto itemFactory = gOsirisProxy->GetLibraryManager().GetItemFactory();
		if (itemFactory == nullptr) {
			Debug("FindItemByHandle(): GetItemFactory not available!");
			return nullptr;
		}

		auto component = itemFactory->Entities->Components.Buf[0x0D].component->FindComponentByHandle(&handle);
		if (component != nullptr) {
			return (EsvItem *)((uint8_t *)component - 8);
		}
		else {
			Debug("FindItemByHandle(): No Item component found with this handle (%x)", handle.Handle);
			return nullptr;
		}
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
			Debug("ShootProjectileApiHelper::SetInt(): Unknown int property: %s", prop);
		}
	}

	void ShootProjectileApiHelper::SetGuidString(char const * prop, char const * value)
	{
		auto character = FindCharacterByNameGuid(value);
		auto item = FindItemByNameGuid(value);

		if (character == nullptr && item == nullptr) {
			Debug("ShootProjectileApiHelper::SetGuidString(): GUID is not a valid item or object (%s)", value);
			return;
		}

		ObjectHandle handle;
		float const * position;

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
			Helper.StartPosition[0] = position[0];
			Helper.StartPosition[1] = position[1];
			Helper.StartPosition[2] = position[2];
			HasStartPosition = true;
		}
		else if (strcmp(prop, "TargetPosition") == 0) {
			Helper.EndPosition[0] = position[0];
			Helper.EndPosition[1] = position[1];
			Helper.EndPosition[2] = position[2];
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
			Debug("ShootProjectileApiHelper::SetGuidString(): Unknown object property: %s", prop);
		}
	}

	void ShootProjectileApiHelper::SetVector(char const * prop, float const * value)
	{
		if (strcmp(prop, "SourcePosition") == 0) {
			Helper.StartPosition[0] = value[0];
			Helper.StartPosition[1] = value[1];
			Helper.StartPosition[2] = value[2];
			HasStartPosition = true;
		}
		else if (strcmp(prop, "TargetPosition") == 0) {
			Helper.EndPosition[0] = value[0];
			Helper.EndPosition[1] = value[1];
			Helper.EndPosition[2] = value[2];
			HasEndPosition = true;
		}
		else {
			Debug("ShootProjectileApiHelper::SetVector(): Unknown vector3 property: %s", prop);
		}
	}

	void ShootProjectileApiHelper::SetString(char const * prop, char const * value)
	{
		auto fs = ToFixedString(value);
		if (!fs) {
			Debug("ShootProjectileApiHelper::SetString(): Could not map string to FixedString: '%s'", value);
			return;
		}

		if (strcmp(prop, "SkillId") == 0) {
			Helper.SkillId = fs;
		}
		else if (strcmp(prop, "FS2") == 0) {
			Helper.FS2 = fs;
		}
		else {
			Debug("ShootProjectileApiHelper::SetString(): Unknown string property: %s", prop);
		}
	}

	bool ShootProjectileApiHelper::Shoot()
	{
		if (!HasStartPosition) {
			Debug("ShootProjectileApiHelper::Shoot(): No start position!");
			return false;
		}

		if (!HasEndPosition) {
			Debug("ShootProjectileApiHelper::Shoot(): No end position!");
			return false;
		}

		if (!Helper.SkillId.Str) {
			Debug("ShootProjectileApiHelper::Shoot(): No skill id!");
			return false;
		}

		auto shoot = gOsirisProxy->GetLibraryManager().ShootProjectile;
		if (shoot == nullptr) {
			Debug("ShootProjectileApiHelper::Shoot(): ShootProjectile helper not found!");
			return false;
		}

		shoot(&Helper, Helper.StatusClearChance);
		return true;
	}
}
