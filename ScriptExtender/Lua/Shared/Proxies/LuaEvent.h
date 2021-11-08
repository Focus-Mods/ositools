#pragma once

#include <Lua/Shared/LuaHelpers.h>
#include <Lua/Shared/Proxies/LuaUserdata.h>
#include <Lua/Shared/LuaLifetime.h>

BEGIN_NS(lua)

class EventObjectParamsImplBase
{
public:
	inline virtual ~EventObjectParamsImplBase() {};
	virtual char const* GetTypeName() const = 0;
	virtual bool GetProperty(lua_State* L, LifetimeHolder const& lifetime, FixedString const& prop) = 0;
	virtual bool SetProperty(lua_State* L, LifetimeHolder const& lifetime, FixedString const& prop, int index) = 0;
	virtual int Next(lua_State* L, LifetimeHolder const& lifetime, FixedString const& key) = 0;
};

enum class WriteableEvent {};
enum class ReadOnlyEvent {};

template <class TParams>
class EventObjectParamsImpl : public EventObjectParamsImplBase
{
public:
	EventObjectParamsImpl(TParams& eventParams)
		: params_(eventParams)
	{}

	~EventObjectParamsImpl() override
	{}

	inline TParams const& GetParams() const
	{
		return params_;
	}

	char const* GetTypeName() const override
	{
		return TypeInfo<TParams>::TypeName;
	}

	bool GetProperty(lua_State* L, LifetimeHolder const& lifetime, FixedString const& prop) override
	{
		return ObjectProxyHelpers<TParams>::GetProperty(L, &params_, lifetime, prop);
	}

	bool SetProperty(lua_State* L, LifetimeHolder const& lifetime, FixedString const& prop, int index) override
	{
		return ObjectProxyHelpers<TParams>::SetProperty(L, &params_, lifetime, prop, index);
	}

	int Next(lua_State* L, LifetimeHolder const& lifetime, FixedString const& key) override
	{
		return ObjectProxyHelpers<TParams>::Next(L, &params_, lifetime, key);
	}

private:
	TParams& params_;
};


class EventObject : private Userdata<EventObject>, public Indexable, public NewIndexable,
	public Iterable, public Stringifiable, public GarbageCollected
{
public:
	static char const* const MetatableName;

	template <class TParams>
	inline static EventObjectParamsImpl<TParams>* Make(lua_State* L, LifetimePool& pool, char const* eventName,
		TParams& eventParams, bool canPreventAction, WriteableEvent)
	{
		auto lifetime = pool.Allocate();
		auto self = NewWithExtraData(L, sizeof(EventObjectParamsImpl<TParams>), LifetimeHolder(pool, lifetime), eventName, canPreventAction, true);
		return new (self->GetImpl()) EventObjectParamsImpl<TParams>(eventParams);
	}

	template <class TParams>
	inline static EventObjectParamsImpl<TParams>* Make(lua_State* L, LifetimePool& pool, char const* eventName, 
		TParams const& eventParams, bool canPreventAction, ReadOnlyEvent)
	{
		auto lifetime = pool.Allocate();
		auto self = NewWithExtraData(L, sizeof(EventObjectParamsImpl<TParams>), LifetimeHolder(pool, lifetime), eventName, canPreventAction, false);
		return new (self->GetImpl()) EventObjectParamsImpl<TParams>(const_cast<TParams &>(eventParams));
	}

	inline EventObjectParamsImplBase* GetImpl()
	{
		// The event object is stored in the extra space we allocated
		// when constructing the Lua object using Userdata::NewWithExtraData()
		return reinterpret_cast<EventObjectParamsImplBase*>(this + 1);
	}

	inline bool IsAlive() const
	{
		return lifetime_.IsAlive();
	}

	template <class TParams>
	TParams* Get()
	{
		if (!lifetime_.IsAlive()) {
			return nullptr;
		}

		if (GetImpl()->GetTypeName() == TypeInfo<TParams>::TypeName) {
			return &reinterpret_cast<EventObjectParamsImpl<TParams>*>(GetImpl())->GetParams();
		} else {
			return nullptr;
		}
	}

private:
	LifetimeReference lifetime_;
	char const* eventName_;
	bool canPreventAction_;
	bool writeable_;

	EventObject(LifetimeHolder const& lifetime, char const* eventName, bool canPreventAction, bool writeable)
		: lifetime_(lifetime), eventName_(eventName), canPreventAction_(canPreventAction), writeable_(writeable)
	{}

	~EventObject()
	{
		GetImpl()->~EventObjectParamsImplBase();
	}

protected:
	friend Userdata<EventObject>;

	int Index(lua_State* L);
	int NewIndex(lua_State* L);
	int Next(lua_State* L);
	int ToString(lua_State* L);
	int GC(lua_State* L);
};


struct EmptyEventParams {};

END_NS()
