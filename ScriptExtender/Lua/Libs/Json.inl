#include <Lua/Shared/LuaSerializers.h>

#include <fstream>
#include <unordered_set>
#include <json/json.h>
#include <lstate.h>

BEGIN_NS(lua::utils)

void JsonParse(lua_State * L, Json::Value const & val);

void JsonParseArray(lua_State * L, Json::Value const & val)
{
	lua_newtable(L);
	int idx = 1;
	for (auto it = val.begin(), end = val.end(); it != end; ++it) {
		push(L, idx++);
		JsonParse(L, *it);
		lua_settable(L, -3);
	}
}

void JsonParseObject(lua_State * L, Json::Value const & val)
{
	lua_newtable(L);
	for (auto it = val.begin(), end = val.end(); it != end; ++it) {
		JsonParse(L, it.key());
		JsonParse(L, *it);
		lua_settable(L, -3);
	}
}

void JsonParse(lua_State * L, Json::Value const & val)
{
	switch (val.type()) {
	case Json::nullValue:
		lua_pushnil(L);
		break;

	case Json::intValue:
		push(L, val.asInt64());
		break;

	case Json::uintValue:
		push(L, (int64_t)val.asUInt64());
		break;

	case Json::realValue:
		push(L, val.asDouble());
		break;

	case Json::stringValue:
		push(L, val.asCString());
		break;

	case Json::booleanValue:
		push(L, val.asBool());
		break;

	case Json::arrayValue:
		JsonParseArray(L, val);
		break;

	case Json::objectValue:
		JsonParseObject(L, val);
		break;

	default:
		luaL_error(L, "Attempted to parse unknown Json value");
	}
}


int JsonParse(lua_State * L)
{
	StackCheck _(L, 1);
	size_t length;
	auto json = luaL_checklstring(L, 1, &length);

	Json::CharReaderBuilder factory;
	std::unique_ptr<Json::CharReader> reader(factory.newCharReader());

	Json::Value root;
	std::string errs;
	if (!reader->parse(json, json + length, &root, &errs)) {
		return luaL_error(L, "Unable to parse JSON: %s", errs.c_str());
	}

	JsonParse(L, root);
	return 1;
}

struct StringifyContext
{
	bool StringifyInternalTypes{ false };
	bool IterateUserdata{ false };
	bool Beautify{ true };
	bool AvoidRecursion{ false };
	uint32_t MaxDepth{ 64 };
	std::unordered_set<void*> SeenUserdata;
};

TValue* GetStackElem(lua_State* L, int idx)
{
	CallInfo* ci = L->ci;
	if (idx > 0) {
		return ci->func + idx;
	} else if (idx > LUA_REGISTRYINDEX) {
		return L->top + idx;
	} else {
		return nullptr;
	}
}

void* GetTablePointer(lua_State* L, int index)
{
	luaL_checktype(L, index, LUA_TTABLE);
	auto val = GetStackElem(L, index);
	if (val) {
		return hvalue(val);
	} else {
		return nullptr;
	}
}

void* GetUserdataPointer(lua_State* L, int index)
{
	auto proxy = Userdata<ObjectProxy2>::AsUserData(L, index);
	if (proxy) {
		return proxy->GetRaw();
	}

	return lua_touserdata(L, index);
}

void* GetPointerValue(lua_State* L, int index)
{
	switch (lua_type(L, index)) {
	case LUA_TTABLE:
		return GetTablePointer(L, index);

	case LUA_TUSERDATA:
		return GetUserdataPointer(L, index);

	default:
		return nullptr;
	}
}

Json::Value JsonStringify(lua_State * L, int index, unsigned depth, StringifyContext& ctx);

Json::Value JsonStringifyUserdata(lua_State * L, int index, unsigned depth, StringifyContext& ctx)
{
	StackCheck _(L, 0);

	if (ctx.AvoidRecursion) {
		auto ptr = GetPointerValue(L, index);
		if (ptr) {
			auto seenIt = ctx.SeenUserdata.find(ptr);
			if (seenIt != ctx.SeenUserdata.end()) {
				return Json::Value("*RECURSION*");
			} else {
				ctx.SeenUserdata.insert(ptr);
			}
		}
	}

	index = lua_absindex(L, index);
	if (!lua_getmetatable(L, index)) {
		return Json::Value::null;
	}

	push(L, "__pairs");
	lua_gettable(L, -2);
	lua_remove(L, -2);
	// No __pairs function, can't iterate this object
	if (lua_type(L, -1) == LUA_TNIL) {
		lua_pop(L, 1);
		return Json::Value::null;
	}

	Json::Value arr(Json::objectValue);

	// Call __pairs(obj)
	auto nextIndex = lua_absindex(L, -1);
	lua_pushvalue(L, index);
	lua_call(L, 1, 3); // returns __next, obj, nil

	// Push next, obj, k
	lua_pushvalue(L, nextIndex);
	lua_pushvalue(L, nextIndex + 1);
	lua_pushvalue(L, nextIndex + 2);
	// Call __next(obj, k)
	lua_call(L, 2, 2); // returns k, val

#if !defined(NDEBUG)
	auto proxy = Userdata<ObjectProxy2>::AsUserData(L, index);
	auto objPtr = proxy ? proxy->GetRaw() : nullptr;
	auto typeName = proxy ? proxy->GetImpl()->GetTypeName() : FixedString{};
#endif

	while (lua_type(L, -2) != LUA_TNIL) {
#if !defined(NDEBUG)
		STDString key;
		if (lua_type(L, -2) == LUA_TSTRING) {
			key = lua_tostring(L, -2);
		} else if (lua_type(L, -2) == LUA_TNUMBER) {
			lua_pushvalue(L, -2);
			key = lua_tostring(L, -1);
			lua_pop(L, 1);
		}
#endif

		Json::Value val(JsonStringify(L, -1, depth + 1, ctx));

		if (lua_type(L, -2) == LUA_TSTRING) {
			auto key = lua_tostring(L, -2);
			arr[key] = val;
		} else if (lua_type(L, -2) == LUA_TNUMBER) {
			lua_pushvalue(L, -2);
			auto key = lua_tostring(L, -1);
			arr[key] = val;
			lua_pop(L, 1);
		} else if (lua_type(L, -2) == LUA_TUSERDATA && ctx.StringifyInternalTypes) {
			int top = lua_gettop(L);
			lua_getglobal(L, "tostring");  /* function to be called */
			lua_pushvalue(L, -3);   /* value to print */
			lua_call(L, 1, 1);
			const char* key = lua_tostring(L, -1);  /* get result */
			if (key) {
				arr[key] = val;
			}
			int top2 = lua_gettop(L);
			lua_pop(L, 1);  /* pop result */
		} else {
			throw std::runtime_error("Can only stringify string or number table keys");
		}

		// Push next, obj, k
		lua_pushvalue(L, nextIndex);
		lua_pushvalue(L, nextIndex + 1);
		lua_pushvalue(L, nextIndex + 3);
		lua_remove(L, -4);
		lua_remove(L, -4);
		// Call __next(obj, k)
		lua_call(L, 2, 2); // returns k, val
	}

	lua_pop(L, 2);

	// Pop __next, obj, nil
	lua_pop(L, 3);
	return arr;
}

Json::Value JsonStringifyObject(lua_State * L, int index, unsigned depth, StringifyContext& ctx)
{
	Json::Value arr(Json::objectValue);
	lua_pushnil(L);

	if (index < 0) index--;

	while (lua_next(L, index) != 0) {
		Json::Value val(JsonStringify(L, -1, depth + 1, ctx));

		if (lua_type(L, -2) == LUA_TSTRING) {
			auto key = lua_tostring(L, -2);
			arr[key] = val;
		} else if (lua_type(L, -2) == LUA_TNUMBER) {
			lua_pushvalue(L, -2);
			auto key = lua_tostring(L, -1);
			arr[key] = val;
			lua_pop(L, 1);
		} else {
			throw std::runtime_error("Can only stringify string or number table keys");
		}

		lua_pop(L, 1);
	}

	return arr;
}

Json::Value JsonStringifyArray(lua_State * L, int index, unsigned depth, StringifyContext& ctx)
{
	Json::Value arr(Json::arrayValue);
	lua_pushnil(L);

	if (index < 0) index--;

	while (lua_next(L, index) != 0) {
		arr.append(JsonStringify(L, -1, depth + 1, ctx));
		lua_pop(L, 1);
	}

	return arr;
}

bool JsonCanStringifyAsArray(lua_State * L, int index)
{
	Json::Value arr(Json::objectValue);
	lua_pushnil(L);

	if (index < 0) index--;

	int next = 1;
	bool isArray = true;
	while (lua_next(L, index) != 0) {
#if LUA_VERSION_NUM > 501
		if (lua_isinteger(L, -2)) {
			auto key = lua_tointeger(L, -2);
			if (key != next++) {
				isArray = false;
			}
		} else {
			isArray = false;
		}
#else
		if (lua_isnumber(L, -2)) {
			auto key = lua_tonumber(L, -2);
			if (abs(key - next++) < 0.0001) {
				isArray = false;
			}
		} else {
			isArray = false;
		}
#endif

		lua_pop(L, 1);
	}

	return isArray;
}

Json::Value JsonStringifyTable(lua_State * L, int index, unsigned depth, StringifyContext& ctx)
{
	if (ctx.AvoidRecursion) {
		auto ptr = GetPointerValue(L, index);
		if (ptr) {
			auto seenIt = ctx.SeenUserdata.find(ptr);
			if (seenIt != ctx.SeenUserdata.end()) {
				return Json::Value("*RECURSION*");
			} else {
				ctx.SeenUserdata.insert(ptr);
			}
		}
	}

	if (JsonCanStringifyAsArray(L, index)) {
		return JsonStringifyArray(L, index, depth, ctx);
	} else {
		return JsonStringifyObject(L, index, depth, ctx);
	}
}


Json::Value JsonStringify(lua_State * L, int index, unsigned depth, StringifyContext& ctx)
{
	if (depth > ctx.MaxDepth) {
		throw std::runtime_error("Recursion depth exceeded while stringifying JSON");
	}

	switch (lua_type(L, index)) {
	case LUA_TNIL:
		return Json::Value(Json::nullValue);

	case LUA_TBOOLEAN:
		return Json::Value(lua_toboolean(L, index) == 1);

	case LUA_TNUMBER:
#if LUA_VERSION_NUM > 501
		if (lua_isinteger(L, index)) {
			return Json::Value(lua_tointeger(L, index));
		} else {
			return Json::Value(lua_tonumber(L, index));
		}
#else
		return Json::Value(lua_tonumber(L, index));
#endif

	case LUA_TSTRING:
		return Json::Value(lua_tostring(L, index));

	case LUA_TTABLE:
		return JsonStringifyTable(L, index, depth, ctx);

	case LUA_TUSERDATA:
	{
		if (ctx.IterateUserdata) {
			auto obj = JsonStringifyUserdata(L, index, depth, ctx);
			if (!obj.isNull()) {
				return obj;
			}
			// Fallthrough
		} else {
			// Fallthrough
		}
	}

	case LUA_TLIGHTUSERDATA:
	case LUA_TFUNCTION:
	case LUA_TTHREAD:
		if (ctx.StringifyInternalTypes) {
			auto val = Json::Value(luaL_tolstring(L, index, NULL));
			lua_pop(L, 1);
			return val;
		} else {
			throw std::runtime_error("Attempted to stringify a lightuserdata, userdata, function or thread value");
		}

	default:
		throw std::runtime_error("Attempted to stringify an unknown type");
	}
}


int JsonStringify(lua_State * L)
{
	StackCheck _(L, 1);
	int nargs = lua_gettop(L);
	if (nargs < 1) {
		return luaL_error(L, "JsonStringify expects at least one parameter.");
	}

	if (nargs > 4) {
		return luaL_error(L, "JsonStringify expects at most three parameters.");
	}

	StringifyContext ctx;

	if (nargs >= 2) {
		// New stringify API - Json.Stringify(obj, paramTable)
		if (lua_type(L, 2) == LUA_TTABLE) {
			ctx.Beautify = try_gettable<bool>(L, "Beautify", 2, true);
			ctx.StringifyInternalTypes = try_gettable<bool>(L, "StringifyInternalTypes", 2, false);
			ctx.IterateUserdata = try_gettable<bool>(L, "IterateUserdata", 2, false);
			ctx.AvoidRecursion = try_gettable<bool>(L, "AvoidRecursion", 2, false);
			ctx.MaxDepth = try_gettable<uint32_t>(L, "MaxDepth", 2, 64);

			if (ctx.MaxDepth > 64) {
				ctx.MaxDepth = 64;
			}
		} else {
			// Old stringify API - Json.Stringify(obj, beautify, stringifyInternalTypes, iterateUserdata)
			ctx.Beautify = lua_toboolean(L, 2) == 1;

			if (nargs >= 3) {
				ctx.StringifyInternalTypes = lua_toboolean(L, 3) == 1;
			}

			if (nargs >= 4) {
				ctx.IterateUserdata = lua_toboolean(L, 4) == 1;
			}
		}
	}

	Json::Value root;
	try {
		root = JsonStringify(L, 1, 0, ctx);
	} catch (std::runtime_error & e) {
		return luaL_error(L, "%s", e.what());
	}

	Json::StreamWriterBuilder builder;
	if (ctx.Beautify) {
		builder["indentation"] = "\t";
	}
	std::stringstream ss;
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &ss);

	push(L, ss.str());
	return 1;
}

void RegisterJsonLib(lua_State* L)
{
	static const luaL_Reg jsonLib[] = {
		{"Parse", JsonParse},
		{"Stringify", JsonStringify},
		{0,0}
	};

	RegisterLib(L, "Json", jsonLib);
}

END_NS()
