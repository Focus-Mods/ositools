#include <stdafx.h>

#include <GameDefinitions/Base/Base.h>
#include <GameDefinitions/Enumerations.h>
#include <Extender/Shared/ExtensionHelpers.h>

namespace dse
{

#define BEGIN_BITMASK_NS(NS, T, type) \
	Vector<FixedString> BitmaskInfoBase<NS::T>::Labels; \
	Map<FixedString, NS::T> BitmaskInfoBase<NS::T>::Values{0}; \
	NS::T BitmaskInfoBase<NS::T>::AllowedFlags{0};
#define BEGIN_ENUM_NS(NS, T, type) \
	Vector<FixedString> EnumInfoBase<NS::T>::Labels; \
	Map<FixedString, NS::T> EnumInfoBase<NS::T>::Values{0};
#define BEGIN_BITMASK(T, type) \
	Vector<FixedString> BitmaskInfoBase<T>::Labels; \
	Map<FixedString, T> BitmaskInfoBase<T>::Values{0}; \
	T BitmaskInfoBase<T>::AllowedFlags{0};
#define BEGIN_ENUM(T, type) \
	Vector<FixedString> EnumInfoBase<T>::Labels; \
	Map<FixedString, T> EnumInfoBase<T>::Values{0};
#define E(label)
#define EV(label, value)
#define END_ENUM_NS()
#define END_ENUM()
#include <GameDefinitions/Enumerations.inl>
#undef BEGIN_BITMASK_NS
#undef BEGIN_ENUM_NS
#undef BEGIN_BITMASK
#undef BEGIN_ENUM
#undef E
#undef EV
#undef END_ENUM_NS
#undef END_ENUM

	void InitializeEnumerations()
	{

#define BEGIN_BITMASK_NS(NS, T, type) { \
	using e = NS::T; \
	using ei = EnumInfo<e>; \
	ei::Init(61);
#define BEGIN_ENUM_NS(NS, T, type) { \
	using e = NS::T; \
	using ei = EnumInfo<e>; \
	ei::Init(61);
#define BEGIN_BITMASK(T, type) { \
	using e = T; \
	using ei = EnumInfo<e>; \
	ei::Init(61);
#define BEGIN_ENUM(T, type) { \
	using e = T; \
	using ei = EnumInfo<e>; \
	ei::Init(61);
#define E(label) ei::Add(e::label, #label);
#define EV(label, value) ei::Add(e::label, #label);
#define END_ENUM_NS() }
#define END_ENUM() }
#include <GameDefinitions/Enumerations.inl>
#undef BEGIN_BITMASK_NS
#undef BEGIN_ENUM_NS
#undef BEGIN_BITMASK
#undef BEGIN_ENUM
#undef E
#undef EV
#undef END_ENUM_NS
#undef END_ENUM

	}
}
