#pragma once

#include <Lua/Shared/LuaHelpers.h>
#include <Lua/Shared/LuaLifetime.h>
#include <Lua/Shared/Proxies/LuaUserdata.h>
#include <Lua/Shared/Proxies/LuaPropertyMap.h>

BEGIN_NS(lua)

LifetimeHolder LifetimeFromState(lua_State* L);
LifetimeHolder GlobalLifetimeFromState(lua_State* L);

class ObjectProxyImplBase
{
public:
	inline virtual ~ObjectProxyImplBase() {};
	virtual FixedString const& GetTypeName() const = 0;
	virtual void* GetRaw() = 0;
	virtual bool GetProperty(lua_State* L, FixedString const& prop) = 0;
	virtual bool SetProperty(lua_State* L, FixedString const& prop, int index) = 0;
	virtual int Next(lua_State* L, FixedString const& key) = 0;
	virtual bool IsA(FixedString const& typeName) = 0;
};

template <class T>
struct ObjectProxyHelpers
{
	static bool GetProperty(lua_State* L, T* object, LifetimeHolder const& lifetime, FixedString const& prop)
	{
		auto const& map = StaticLuaPropertyMap<T>::PropertyMap;
		auto fetched = map.GetProperty(L, lifetime, object, prop);
		if (!fetched) {
			luaL_error(L, "Object of type '%s' has no property named '%s'", GetTypeInfo<T>().TypeName.GetString(), prop.GetString());
			return false;
		}

		return true;
	}

	static bool SetProperty(lua_State* L, T* object, LifetimeHolder const& lifetime, FixedString const& prop, int index)
	{
		auto const& map = StaticLuaPropertyMap<T>::PropertyMap;
		auto ok = map.SetProperty(L, lifetime, object, prop, index);
		if (!ok) {
			luaL_error(L, "Object of type '%s' has no property named '%s'", GetTypeInfo<T>().TypeName.GetString(), prop.GetString());
			return false;
		}

		return true;
	}

	static int Next(lua_State* L, T* object, LifetimeHolder const& lifetime, FixedString const& key)
	{
		auto const& map = StaticLuaPropertyMap<T>::PropertyMap;
		if (!key) {
			if (!map.Properties.empty()) {
				StackCheck _(L, 2);
				auto it = map.Properties.begin();
				push(L, it->first);
				if (!map.GetProperty(L, lifetime, object, it->second)) {
					push(L, nullptr);
				}

				return 2;
			}
		} else {
			auto it = map.Properties.find(key);
			if (it != map.Properties.end()) {
				++it;
				if (it != map.Properties.end()) {
					StackCheck _(L, 2);
					push(L, it->first);
					if (!map.GetProperty(L, lifetime, object, it->second)) {
						push(L, nullptr);
					}

					return 2;
				}
			}
		}

		return 0;
	}

	static bool IsA(FixedString const& typeName)
	{
		auto const& map = StaticLuaPropertyMap<T>::PropertyMap;
		if (map.Name == typeName) {
			return true;
		}

		for (auto const& parent : map.Parents) {
			if (parent == typeName) {
				return true;
			}
		}

		return false;
	}
};

template <class T>
class ObjectProxyRefImpl : public ObjectProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "ObjectProxyImpl template parameter should not be a pointer type!");
	static_assert(!std::is_const_v<T>, "ObjectProxyImpl template parameter should not be CV-qualified!");
	static_assert(!std::is_volatile_v<T>, "ObjectProxyImpl template parameter should not be CV-qualified!");

	ObjectProxyRefImpl(LifetimeHolder const& lifetime, T * obj)
		: object_(obj), lifetime_(lifetime)
	{
		assert(obj != nullptr);
	}
		
	~ObjectProxyRefImpl() override
	{}

	T* Get() const
	{
		return object_;
	}

	void* GetRaw() override
	{
		return object_;
	}

	FixedString const& GetTypeName() const override
	{
		return StaticLuaPropertyMap<T>::PropertyMap.Name;
	}

	bool GetProperty(lua_State* L, FixedString const& prop) override
	{
		return ObjectProxyHelpers<T>::GetProperty(L, object_, lifetime_, prop);
	}

	bool SetProperty(lua_State* L, FixedString const& prop, int index) override
	{
		return ObjectProxyHelpers<T>::SetProperty(L, object_, lifetime_, prop, index);
	}

	int Next(lua_State* L, FixedString const& key) override
	{
		return ObjectProxyHelpers<T>::Next(L, object_, lifetime_, key);
	}

	bool IsA(FixedString const& typeName) override
	{
		return ObjectProxyHelpers<T>::IsA(typeName);
	}

private:
	T* object_;
	LifetimeHolder lifetime_;
};

template <class T>
class ObjectProxyHandleBasedRefImpl : public ObjectProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "ObjectProxyImpl template parameter should not be a pointer type!");
	static_assert(!std::is_const_v<T>, "ObjectProxyImpl template parameter should not be CV-qualified!");
	static_assert(!std::is_volatile_v<T>, "ObjectProxyImpl template parameter should not be CV-qualified!");

	ObjectProxyHandleBasedRefImpl(LifetimeHolder const& containerLifetime, ComponentHandle handle, LifetimeHolder const& lifetime)
		: handle_(handle), containerLifetime_(containerLifetime), lifetime_(lifetime)
	{}

	~ObjectProxyHandleBasedRefImpl() override
	{}

	T* Get() const;

	void* GetRaw() override
	{
		return Get();
	}

	FixedString const& GetTypeName() const override
	{
		return StaticLuaPropertyMap<T>::PropertyMap.Name;
	}

	bool GetProperty(lua_State* L, FixedString const& prop) override
	{
		auto object = Get();
		if (!object) return false;
		return ObjectProxyHelpers<T>::GetProperty(L, object, LifetimeFromState(L), prop);
	}

	bool SetProperty(lua_State* L, FixedString const& prop, int index) override
	{
		auto object = Get();
		if (!object) return false;
		return ObjectProxyHelpers<T>::SetProperty(L, object, LifetimeFromState(L), prop, index);
	}

	int Next(lua_State* L, FixedString const& key) override
	{
		auto object = Get();
		if (!object) return 0;
		return ObjectProxyHelpers<T>::Next(L, object, LifetimeFromState(L), key);
	}

	bool IsA(FixedString const& typeName) override
	{
		return ObjectProxyHelpers<T>::IsA(typeName);
	}

private:
	ComponentHandle handle_;
	LifetimeHolder containerLifetime_;
	LifetimeReference lifetime_;
};

// Object proxy that owns the contained object and deletes the object on GC
template <class T>
class ObjectProxyOwnerImpl : public ObjectProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "ObjectProxyImpl template parameter should not be a pointer type!");

	ObjectProxyOwnerImpl(LifetimePool& pool, Lifetime* lifetime, T* obj)
		: lifetime_(pool, lifetime), 
		object_(obj, &GameDelete<T>)
	{
		assert(obj != nullptr);
	}

	~ObjectProxyOwnerImpl() override
	{
		lifetime_.GetLifetime()->Kill();
	}

	T* Get() const
	{
		return object_.get();
	}

	void* GetRaw() override
	{
		return object_.get();
	}

	FixedString const& GetTypeName() const override
	{
		return StaticLuaPropertyMap<T>::PropertyMap.Name;
	}

	bool GetProperty(lua_State* L, FixedString const& prop) override
	{
		return ObjectProxyHelpers<T>::GetProperty(L, object_.get(), lifetime_.Get(), prop);
	}

	bool SetProperty(lua_State* L, FixedString const& prop, int index) override
	{
		return ObjectProxyHelpers<T>::SetProperty(L, object_.get(), lifetime_.Get(), prop, index);
	}

	int Next(lua_State* L, FixedString const& key) override
	{
		return ObjectProxyHelpers<T>::Next(L, object_.get(), lifetime_.Get(), key);
	}

	bool IsA(FixedString const& typeName) override
	{
		return ObjectProxyHelpers<T>::IsA(typeName);
	}

private:
	GameUniquePtr<T> object_;
	LifetimeReference lifetime_;
};


// Object proxy that contains the object (i.e. the object is allocated next to the proxy impl in the Lua heap)
template <class T>
class ObjectProxyContainerImpl : public ObjectProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "ObjectProxyImpl template parameter should not be a pointer type!");

	template <class... Args>
	ObjectProxyContainerImpl(LifetimePool& pool, Lifetime* lifetime, Args... args)
		: lifetime_(pool, lifetime), 
		object_(args...)
	{}

	~ObjectProxyContainerImpl() override
	{
		lifetime_.GetLifetime()->Kill();
	}

	T* Get() const
	{
		return &object_;
	}

	void* GetRaw() override
	{
		return &object_;
	}

	FixedString const& GetTypeName() const override
	{
		return StaticLuaPropertyMap<T>::PropertyMap.Name;
	}

	bool GetProperty(lua_State* L, FixedString const& prop) override
	{
		return ObjectProxyHelpers<T>::GetProperty(L, &object_, lifetime_.Get(), prop);
	}

	bool SetProperty(lua_State* L, FixedString const& prop, int index) override
	{
		return ObjectProxyHelpers<T>::SetProperty(L, &object_, lifetime_.Get(), prop, index);
	}

	int Next(lua_State* L, FixedString const& key) override
	{
		return ObjectProxyHelpers<T>::Next(L, &object_, lifetime_.Get(), key);
	}

	bool IsA(FixedString const& typeName) override
	{
		return ObjectProxyHelpers<T>::IsA(typeName);
	}

private:
	LifetimeReference lifetime_;
	T object_;
};


class ObjectProxy2 : private Userdata<ObjectProxy2>, public Indexable, public NewIndexable,
	public Iterable, public Stringifiable, public EqualityComparable
{
public:
	static char const * const MetatableName;

	template <class TImpl, class T, class... Args>
	inline static TImpl* MakeImpl(lua_State* L, T* object, LifetimeHolder const& lifetime, Args... args)
	{
		auto self = NewWithExtraData(L, sizeof(TImpl), lifetime);
		return new (self->GetImpl()) TImpl(lifetime, object, args...);
	}

	template <class T>
	inline static ObjectProxyRefImpl<T>* MakeRef(lua_State* L, T* object, LifetimeHolder const& lifetime)
	{
		auto self = NewWithExtraData(L, sizeof(ObjectProxyRefImpl<T>), lifetime);
		return new (self->GetImpl()) ObjectProxyRefImpl<T>(lifetime, object);
	}

	template <class T>
	inline static ObjectProxyOwnerImpl<T>* MakeOwner(lua_State* L, LifetimePool& pool, T* obj)
	{
		auto lifetime = pool.Allocate();
		auto self = NewWithExtraData(L, sizeof(ObjectProxyOwnerImpl<T>), LifetimeHolder(pool, lifetime));
		return new (self->GetImpl()) ObjectProxyOwnerImpl<T>(pool, lifetime, obj);
	}

	template <class T, class... Args>
	inline static ObjectProxyContainerImpl<T>* MakeContainer(lua_State* L, LifetimePool& pool, Args... args)
	{
		auto lifetime = pool.Allocate();
		auto self = NewWithExtraData(L, sizeof(ObjectProxyContainerImpl<T>), LifetimeHolder(pool, lifetime));
		return new (self->GetImpl()) ObjectProxyContainerImpl<T>(pool, lifetime, args...);
	}

	template <class T>
	inline static ObjectProxyHandleBasedRefImpl<T>* MakeHandle(lua_State* L, ComponentHandle handle, LifetimeHolder const& lifetime)
	{
		auto self = NewWithExtraData(L, sizeof(ObjectProxyHandleBasedRefImpl<T>), GlobalLifetimeFromState(L));
		return new (self->GetImpl()) ObjectProxyHandleBasedRefImpl<T>(GlobalLifetimeFromState(L), handle, lifetime);
	}

	static void* CheckedGetRaw(lua_State* L, int index, FixedString const& typeName);
	static void* TryGetRaw(lua_State* L, int index, FixedString const& typeName);

	template <class T>
	inline static T* CheckedGet(lua_State* L, int index)
	{
		auto const& typeName = StaticLuaPropertyMap<T>::PropertyMap.Name;
		auto obj = CheckedGetRaw(L, index, typeName);
		return reinterpret_cast<T *>(obj);
	}

	template <class T>
	inline static T* TryGet(lua_State* L, int index)
	{
		auto const& typeName = StaticLuaPropertyMap<T>::PropertyMap.Name;
		auto obj = TryGetRaw(L, index, typeName);
		return reinterpret_cast<T *>(obj);
	}

	inline ObjectProxyImplBase* GetImpl()
	{
		return reinterpret_cast<ObjectProxyImplBase*>(this + 1);
	}

	inline bool IsAlive() const
	{
		return lifetime_.IsAlive();
	}

	inline void* GetRaw()
	{
		if (!lifetime_.IsAlive()) {
			return nullptr;
		}

		return GetImpl()->GetRaw();
	}

	template <class T>
	T* Get()
	{
		if (!lifetime_.IsAlive()) {
			return nullptr;
		}

		if (GetImpl()->IsA(StaticLuaPropertyMap<T>::PropertyMap.Name)) {
			return reinterpret_cast<T*>(GetImpl()->GetRaw());
		} else {
			return nullptr;
		}
	}

private:
	LifetimeReference lifetime_;

	ObjectProxy2(LifetimeHolder const& lifetime)
		: lifetime_(lifetime)
	{}

	~ObjectProxy2()
	{
		GetImpl()->~ObjectProxyImplBase();
	}

protected:
	friend Userdata<ObjectProxy2>;

	int Index(lua_State* L);
	int NewIndex(lua_State* L);
	int Next(lua_State* L);
	int ToString(lua_State* L);
	bool IsEqual(lua_State* L, ObjectProxy2* other);
};

template <class T>
inline void push_proxy(lua_State* L, LifetimeHolder const& lifetime, T const& v)
{
	if constexpr (std::is_pointer_v<T> 
		&& (std::is_base_of_v<HasObjectProxy, std::remove_pointer_t<T>>
			|| HasObjectProxyTag<std::remove_pointer_t<T>>::HasProxy)) {
		if (v) {
			ObjectProxy2::MakeRef<std::remove_pointer_t<T>>(L, v, lifetime);
		} else {
			lua_pushnil(L);
		}
	} else {
		push(L, v);
	}
}

END_NS()
