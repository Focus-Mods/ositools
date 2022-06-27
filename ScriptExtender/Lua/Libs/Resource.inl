#pragma once
#include <Lua/Shared/LuaSerializers.h>
#include <Lua/Shared/LuaMethodHelpers.h>
#include <Extender/ScriptExtender.h>

BEGIN_NS(lua::resource)

Resource* Get(ResourceType type, FixedString templateId)
{
	auto bank = GetStaticSymbols().GetResourceBank();
	return bank->Container.Banks[(uint32_t)type]->Resources.TryGet(templateId);
}

void RegisterResourceLib()
{
	DECLARE_MODULE(Resource, Both)
	BEGIN_MODULE()
	MODULE_FUNCTION(Get)
	END_MODULE()
}

END_NS()