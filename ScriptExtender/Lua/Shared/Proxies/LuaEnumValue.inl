#include <Lua/Shared/Proxies/LuaEnumValue.h>
#include <Extender/ScriptExtender.h>

BEGIN_NS(lua)

EnumInfoStore<EnumUnderlyingType>* EnumValueMetatable::GetEnumInfo(CppValueMetadata const& self)
{
	return EnumRegistry::Get().EnumsById[self.PropertyMapTag];
}

FixedString EnumValueMetatable::GetLabel(CppValueMetadata const& self)
{
	return GetEnumInfo(self)->Find(static_cast<EnumUnderlyingType>(self.Value));
}

EnumUnderlyingType EnumValueMetatable::GetValue(CppValueMetadata const& self)
{
	return static_cast<EnumUnderlyingType>(self.Value);
}

int EnumValueMetatable::Index(lua_State* L, CppValueMetadata& self)
{
	auto index = get<FixedString>(L, 2);
	if (index == GFS.strLabel) {
		push(L, GetLabel(self));
	} else if (index == GFS.strValue) {
		push(L, GetValue(self));
	} else if (index == GFS.strEnumName) {
		push(L, GetEnumInfo(self)->EnumName);
	} else {
		luaL_error(L, "Enum values have no property named '%s'", index.GetStringOrDefault());
		push(L, nullptr);
	}

	return 1;
}

int EnumValueMetatable::ToString(lua_State* L, CppValueMetadata& self)
{
	StackCheck _(L, 1);
	push(L, GetLabel(self));
	return 1;
}

bool EnumValueMetatable::IsEqual(lua_State* L, CppValueMetadata& self, int otherIndex)
{
	auto ei = GetEnumInfo(self);
	auto other = try_get_enum_value(L, otherIndex, *ei);
	return other && *other == self.Value;
}

char const* EnumValueMetatable::GetTypeName(lua_State* L, CppValueMetadata& self)
{
	auto ei = GetEnumInfo(self);
	return ei->EnumName.GetStringOrDefault();
}

EnumUnderlyingType get_enum_value(lua_State* L, int index, EnumInfoStore<EnumUnderlyingType> const& store)
{
	switch (lua_type(L, index)) {
	case LUA_TSTRING:
	{
		auto val = do_get(L, index, Overload<FixedString>{});
		auto valueIndex = store.Find(val);
		if (valueIndex) {
			return *valueIndex;
		} else {
			luaL_error(L, "Param %d: not a valid '%s' enum label: %s", index, store.EnumName.GetStringOrDefault(), val.GetStringOrDefault());
		}
		break;
	}

	case LUA_TNUMBER:
	{
		auto val = (EnumUnderlyingType)lua_tointeger(L, index);
		if (store.Find(val)) {
			return val;
		} else {
			luaL_error(L, "Param %d: not a valid '%s' enum index: %d", index, store.EnumName.GetStringOrDefault(), val);
		}
		break;
	}

	case LUA_TLIGHTCPPOBJECT:
	{
		CppValueMetadata meta;
		lua_get_cppvalue(L, index, meta);
		if (meta.MetatableTag == EnumValueMetatable::MetaTag && meta.PropertyMapTag == (unsigned)store.RegistryIndex) {
			return static_cast<EnumUnderlyingType>(meta.Value);
		} else {
			luaL_error(L, "Param %d: expected a '%s' enum value, got type %d", index, store.EnumName.GetStringOrDefault(), meta.MetatableTag);
		}
		break;
	}

	default:
		luaL_error(L, "Param %d: expected integer, string or enum label of type '%s' value, got %s", index,
			store.EnumName.GetStringOrDefault(), lua_typename(L, lua_type(L, index)));
		break;
	}

	return 0;
}

std::optional<EnumUnderlyingType> try_get_enum_value(lua_State* L, int index, EnumInfoStore<EnumUnderlyingType> const& store)
{
	switch (lua_type(L, index)) {
	case LUA_TSTRING:
	{
		auto val = do_get(L, index, Overload<FixedString>{});
		auto valueIndex = store.Find(val);
		if (valueIndex) {
			return *valueIndex;
		}
		break;
	}

	case LUA_TNUMBER:
	{
		auto val = (EnumUnderlyingType)lua_tointeger(L, index);
		if (store.Find(val)) {
			return val;
		}
		break;
	}

	case LUA_TLIGHTCPPOBJECT:
	{
		CppValueMetadata meta;
		lua_get_cppvalue(L, index, meta);
		if (meta.MetatableTag == EnumValueMetatable::MetaTag && meta.PropertyMapTag == (unsigned)store.RegistryIndex) {
			return static_cast<EnumUnderlyingType>(meta.Value);
		}
		break;
	}
	}

	return {};
}

void push_enum_value(lua_State* L, EnumUnderlyingType value, EnumInfoStore<EnumUnderlyingType> const& store)
{
	EnumValueMetatable::Make(L, value, store.RegistryIndex);
}

END_NS()
