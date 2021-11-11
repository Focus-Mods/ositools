#pragma once

#include <span>
#include <Lua/Shared/LuaHelpers.h>
#include <Lua/Shared/LuaSerializers.h>
#include <Lua/Shared/LuaLifetime.h>
#include <Lua/Shared/Proxies/LuaUserdata.h>

BEGIN_SE()

BY_VAL_ARRAY(uint8_t);
BY_VAL_ARRAY(int16_t);
BY_VAL_ARRAY(uint16_t);
BY_VAL_ARRAY(int32_t);
BY_VAL_ARRAY(uint32_t);
BY_VAL_ARRAY(int64_t);
BY_VAL_ARRAY(uint64_t);
BY_VAL_ARRAY(float);
BY_VAL_ARRAY(double);
BY_VAL_ARRAY(bool);
BY_VAL_ARRAY(ComponentHandle);
BY_VAL_ARRAY(EntityHandle);
BY_VAL_ARRAY(FixedString);
BY_VAL_ARRAY(STDString);
BY_VAL_ARRAY(STDWString);
BY_VAL_ARRAY(Path);
BY_VAL_ARRAY(NetId);
BY_VAL_ARRAY(UserId);
BY_VAL_ARRAY(glm::ivec2);
BY_VAL_ARRAY(glm::vec2);
BY_VAL_ARRAY(glm::vec3);
BY_VAL_ARRAY(glm::vec4);
BY_VAL_ARRAY(glm::mat3);

END_SE()

BEGIN_NS(lua)

LifetimeHolder GetCurrentLifetime();

class ArrayProxyImplBase
{
public:
	inline virtual ~ArrayProxyImplBase() {};
	virtual char const* GetTypeName() const = 0;
	virtual void* GetRaw() = 0;
	virtual bool GetElement(lua_State* L, unsigned arrayIndex) = 0;
	virtual bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) = 0;
	virtual int Next(lua_State* L, int key) = 0;
	virtual unsigned Length() = 0;
};


template <class T>
class ArrayProxyByRefImpl : public ArrayProxyImplBase
{
public:
	ArrayProxyByRefImpl(LifetimeHolder const& lifetime, Array<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~ArrayProxyByRefImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= (int)object_->Size()) {
			MakeObjectRef(L, lifetime_, &(*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		// Appending/swapping elements to by-ref arrays not supported for now
		return false;
	}

	unsigned Length() override
	{
		return object_->Size();
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->Size()) {
			push(L, ++key);
			MakeObjectRef(L, lifetime_, &(*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	Array<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class ArrayProxyByValImpl : public ArrayProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "ArrayProxyByValImpl template parameter should not be a pointer type!");

	ArrayProxyByValImpl(LifetimeHolder const& lifetime, Array<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~ArrayProxyByValImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->Size()) {
			LuaWrite(L, (*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->Size()) {
			lua_pushvalue(L, luaIndex);
			LuaRead(L, (*object_)[arrayIndex - 1]);
			lua_pop(L, 1);
			return true;
		} else if (arrayIndex == object_->Size() + 1) {
			T val;
			lua_pushvalue(L, luaIndex);
			LuaRead(L, val);
			lua_pop(L, 1);

			object_->Add(val);
			return true;
		} else {
			return false;
		}
	}

	unsigned Length() override
	{
		return object_->Size();
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->Size()) {
			push(L, ++key);
			LuaWrite(L, (*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	Array<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class VectorProxyByRefImpl : public ArrayProxyImplBase
{
public:
	VectorProxyByRefImpl(LifetimeHolder const& lifetime, Vector<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~VectorProxyByRefImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= (int)object_->size()) {
			MakeObjectRef(L, lifetime_, &(*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		// Appending/swapping elements to by-ref arrays not supported for now
		return false;
	}

	unsigned Length() override
	{
		return (unsigned)object_->size();
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->size()) {
			push(L, ++key);
			MakeObjectRef(L, lifetime_, &(*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	Vector<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class VectorProxyByValImpl : public ArrayProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "VectorProxyByValImpl template parameter should not be a pointer type!");

	VectorProxyByValImpl(LifetimeHolder const& lifetime, Vector<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~VectorProxyByValImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->size()) {
			LuaWrite(L, (*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->size()) {
			lua_pushvalue(L, luaIndex);
			LuaRead(L, (*object_)[arrayIndex - 1]);
			lua_pop(L, 1);
			return true;
		} else if (arrayIndex == object_->size() + 1) {
			T val;
			lua_pushvalue(L, luaIndex);
			LuaRead(L, val);
			lua_pop(L, 1);

			object_->push_back(val);
			return true;
		} else {
			return false;
		}
	}

	unsigned Length() override
	{
		return (unsigned)object_->size();
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->size()) {
			push(L, ++key);
			LuaWrite(L, (*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	Vector<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class SpanProxyByRefImpl : public ArrayProxyImplBase
{
public:
	SpanProxyByRefImpl(LifetimeHolder const& lifetime, std::span<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~SpanProxyByRefImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= (int)object_->size()) {
			MakeObjectRef(L, lifetime_, &(*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		// Appending/swapping elements to by-ref arrays not supported for now
		return false;
	}

	unsigned Length() override
	{
		return (unsigned)object_->size();
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->size()) {
			push(L, ++key);
			MakeObjectRef(L, lifetime_, &(*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	std::span<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class SpanProxyByValImpl : public ArrayProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "SpanProxyByValImpl template parameter should not be a pointer type!");

	SpanProxyByValImpl(LifetimeHolder const& lifetime, std::span<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~SpanProxyByValImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->size()) {
			LuaWrite(L, (*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->size()) {
			lua_pushvalue(L, luaIndex);
			LuaRead(L, (*object_)[arrayIndex - 1]);
			lua_pop(L, 1);
			return true;
		} else {
			// Cannot append elements to a gsl::span
			return false;
		}
	}

	unsigned Length() override
	{
		return (unsigned)object_->size();
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->size()) {
			push(L, ++key);
			LuaWrite(L, (*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	std::span<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class ObjectSetProxyByRefImpl : public ArrayProxyImplBase
{
public:
	ObjectSetProxyByRefImpl(LifetimeHolder const& lifetime, ObjectSet<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~ObjectSetProxyByRefImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->Size) {
			MakeObjectRef(L, lifetime_, &(*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		// Appending/swapping elements to by-ref arrays not supported for now
		return false;
	}

	unsigned Length() override
	{
		return object_->Size;
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->Size) {
			push(L, ++key);
			MakeObjectRef(L, lifetime_, &(*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	ObjectSet<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T>
class ObjectSetProxyByValImpl : public ArrayProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "ObjectSetProxyByValImpl template parameter should not be a pointer type!");

	ObjectSetProxyByValImpl(LifetimeHolder const& lifetime, ObjectSet<T> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~ObjectSetProxyByValImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->Size) {
			LuaWrite(L, (*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= object_->Size) {
			lua_pushvalue(L, luaIndex);
			LuaRead(L, (*object_)[arrayIndex - 1]);
			lua_pop(L, 1);
			return true;
		} else if (arrayIndex == object_->Size + 1) {
			T val;
			lua_pushvalue(L, luaIndex);
			LuaRead(L, val);
			lua_pop(L, 1);

			object_->Add(val);
			return true;
		} else {
			return false;
		}
	}

	unsigned Length() override
	{
		return object_->Size;
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < (int)object_->Size) {
			push(L, ++key);
			LuaWrite(L, (*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	ObjectSet<T>* object_;
	LifetimeHolder lifetime_;
};


template <class T, int Size>
class StdArrayProxyByRefImpl : public ArrayProxyImplBase
{
public:
	StdArrayProxyByRefImpl(LifetimeHolder const& lifetime, std::array<T, Size> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~StdArrayProxyByRefImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= Size) {
			MakeObjectRef(L, lifetime_, &(*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		// Appending/swapping elements to by-ref arrays not supported for now
		return false;
	}

	unsigned Length() override
	{
		return Size;
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < Size) {
			push(L, ++key);
			MakeObjectRef(L, lifetime_, &(*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	std::array<T, Size>* object_;
	LifetimeHolder lifetime_;
};


template <class T, int Size>
class StdArrayProxyByValImpl : public ArrayProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "StdArrayProxyByValImpl template parameter should not be a pointer type!");

	StdArrayProxyByValImpl(LifetimeHolder const& lifetime, std::array<T, Size> * obj)
		: object_(obj), lifetime_(lifetime)
	{}
		
	~StdArrayProxyByValImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	char const* GetTypeName() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElement(lua_State* L, unsigned arrayIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= Size) {
			LuaWrite(L, (*object_)[arrayIndex - 1]);
			return true;
		} else {
			return false;
		}
	}

	bool SetElement(lua_State* L, unsigned arrayIndex, int luaIndex) override
	{
		if (arrayIndex > 0 && arrayIndex <= Size) {
			lua_pushvalue(L, luaIndex);
			LuaRead(L, (*object_)[arrayIndex - 1]);
			lua_pop(L, 1);
			return true;
		} else {
			return false;
		}
	}

	unsigned Length() override
	{
		return Size;
	}

	int Next(lua_State* L, int key) override
	{
		if (key >= 0 && key < Size) {
			push(L, ++key);
			LuaWrite(L, (*object_)[key - 1]);
			return 2;
		} else {
			return 0;
		}
	}

private:
	std::array<T, Size>* object_;
	LifetimeHolder lifetime_;
};


class ArrayProxy : private Userdata<ArrayProxy>, public Indexable, public NewIndexable,
	public Lengthable, public Iterable, public Stringifiable, public GarbageCollected
{
public:
	static char const * const MetatableName;

	template <class TImpl, class... Args>
	inline static TImpl* MakeImplByRef(lua_State* L, LifetimeHolder const& lifetime, Args... args)
	{
		auto self = NewWithExtraData(L, sizeof(TImpl), lifetime);
		return new (self->GetImpl()) TImpl(lifetime, args...);
	}

	template <class T>
	inline static ArrayProxyByRefImpl<T>* MakeByRef(lua_State* L, Array<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<ArrayProxyByRefImpl<T>>(L, lifetime, object);
	}

	template <class T>
	inline static VectorProxyByRefImpl<T>* MakeByRef(lua_State* L, Vector<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<VectorProxyByRefImpl<T>>(L, lifetime, object);
	}

	template <class T>
	inline static SpanProxyByRefImpl<T>* MakeByRef(lua_State* L, std::span<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<SpanProxyByRefImpl<T>>(L, lifetime, object);
	}

	template <class T>
	inline static ObjectSetProxyByRefImpl<T>* MakeByRef(lua_State* L, ObjectSet<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<ObjectSetProxyByRefImpl<T>>(L, lifetime, object);
	}

	template <class T, int Size>
	inline static StdArrayProxyByRefImpl<T, Size>* MakeByRef(lua_State* L, std::array<T, Size>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<StdArrayProxyByRefImpl<T, Size>>(L, lifetime, object);
	}

	template <class T>
	inline static ArrayProxyByValImpl<T>* MakeByVal(lua_State* L, Array<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<ArrayProxyByValImpl<T>>(L, lifetime, object);
	}

	template <class T>
	inline static VectorProxyByValImpl<T>* MakeByVal(lua_State* L, Vector<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<VectorProxyByValImpl<T>>(L, lifetime, object);
	}

	template <class T>
	inline static SpanProxyByValImpl<T>* MakeByVal(lua_State* L, std::span<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<SpanProxyByValImpl<T>>(L, lifetime, object);
	}

	template <class T>
	inline static ObjectSetProxyByValImpl<T>* MakeByVal(lua_State* L, ObjectSet<T>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<ObjectSetProxyByValImpl<T>>(L, lifetime, object);
	}

	template <class T, int Size>
	inline static StdArrayProxyByValImpl<T, Size>* MakeByVal(lua_State* L, std::array<T, Size>* object, LifetimeHolder const& lifetime)
	{
		return MakeImplByRef<StdArrayProxyByValImpl<T, Size>>(L, lifetime, object);
	}

	inline ArrayProxyImplBase* GetImpl()
	{
		return reinterpret_cast<ArrayProxyImplBase*>(this + 1);
	}

	inline bool IsAlive() const
	{
		return lifetime_.IsAlive();
	}

	template <class T>
	T* Get()
	{
		if (!lifetime_.IsAlive()) {
			return nullptr;
		}

		if (strcmp(GetTypeInfo<T>(), GetImpl()->GetTypeName()) == 0) {
			return reinterpret_cast<T*>(GetImpl()->GetRaw());
		} else {
			return nullptr;
		}
	}

private:
	LifetimeReference lifetime_;

	ArrayProxy(LifetimeHolder const& lifetime)
		: lifetime_(lifetime)
	{}

	~ArrayProxy()
	{
		GetImpl()->~ArrayProxyImplBase();
	}

protected:
	friend Userdata<ArrayProxy>;

	int Index(lua_State* L);
	int NewIndex(lua_State* L);
	int Length(lua_State* L);
	int Next(lua_State* L);
	int ToString(lua_State* L);
	int GC(lua_State* L);
};

template <class T>
inline void push_array_ref_proxy(lua_State* L, LifetimeHolder const& lifetime, T* v)
{
	ArrayProxy::MakeByRef<T>(L, v, lifetime);
}

template <class T>
inline T* checked_get_array_proxy(lua_State* L, int index)
{
	auto proxy = Userdata<ArrayProxy>::CheckUserData(L, index);
	auto const& typeName = GetTypeInfo<T>();
	if (strcmp(proxy->GetImpl()->GetTypeName(), typeName) == 0) {
		auto obj = proxy->Get<T>();
		if (obj == nullptr) {
			luaL_error(L, "Argument %d: got Array<%s> whose lifetime has expired", index, typeName);
			return nullptr;
		} else {
			return obj;
		}
	} else {
		luaL_error(L, "Argument %d: expected Array<%s>, got Array<%s>", index, typeName, proxy->GetImpl()->GetTypeName());
		return nullptr;
	}
}

END_NS()
