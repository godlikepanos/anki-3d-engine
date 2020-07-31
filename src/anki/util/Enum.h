// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>

/// @privatesection
/// @{

// Re-implement std::underlying_type to avoid including the whole typetraits header
template<typename TEnum>
struct EnumUnderlyingType
{
	using Type = __underlying_type(TEnum);
};

/// Convert an enum to it's integer type.
template<typename TEnum>
constexpr inline typename EnumUnderlyingType<TEnum>::Type enumToType(TEnum e)
{
	return static_cast<typename EnumUnderlyingType<TEnum>::Type>(e);
}

#define _ANKI_ENUM_OPERATOR(enum_, qualifier_, operator_, assignOperator_) \
	constexpr qualifier_ enum_ operator operator_(const enum_ a, const enum_ b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(static_cast<Int>(a) operator_ static_cast<Int>(b)); \
	} \
	constexpr qualifier_ enum_ operator operator_(const enum_ a, const EnumUnderlyingType<enum_>::Type b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(static_cast<Int>(a) operator_ b); \
	} \
	constexpr qualifier_ enum_ operator operator_(const EnumUnderlyingType<enum_>::Type a, const enum_ b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(a operator_ static_cast<Int>(b)); \
	} \
	qualifier_ enum_& operator assignOperator_(enum_& a, const enum_ b) \
	{ \
		a = a operator_ b; \
		return a; \
	} \
	qualifier_ enum_& operator assignOperator_(enum_& a, const EnumUnderlyingType<enum_>::Type b) \
	{ \
		a = a operator_ b; \
		return a; \
	} \
	qualifier_ EnumUnderlyingType<enum_>::Type& operator assignOperator_(EnumUnderlyingType<enum_>::Type& a, \
																		 const enum_ b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		a = static_cast<Int>(a operator_ static_cast<Int>(b)); \
		return a; \
	}

#define _ANKI_ENUM_UNARAY_OPERATOR(enum_, qualifier_, operator_) \
	constexpr qualifier_ enum_ operator operator_(const enum_ a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(operator_ static_cast<Int>(a)); \
	}

#define _ANKI_ENUM_INCREMENT_DECREMENT(enum_, qualifier_) \
	qualifier_ enum_& operator++(enum_& a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		a = static_cast<enum_>(static_cast<Int>(a) + 1); \
		return a; \
	} \
	qualifier_ enum_& operator--(enum_& a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		a = static_cast<enum_>(static_cast<Int>(a) - 1); \
		return a; \
	} \
	qualifier_ enum_ operator++(enum_& a, int) \
	{ \
		enum_ old = a; \
		++a; \
		return old; \
	} \
	qualifier_ enum_ operator--(enum_& a, int) \
	{ \
		enum_ old = a; \
		--a; \
		return old; \
	}

#define _ANKI_ENUM_NEGATIVE_OPERATOR(enum_, qualifier_) \
	qualifier_ Bool operator!(const enum_& a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<Int>(a) == 0; \
	} \
/// @}

/// @addtogroup util_other
/// @{

/// Implement all those functions that will make a stronly typed enum behave like the old type of enums.
#define ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enum_, qualifier_) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, |, |=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, &, &=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, ^, ^=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, +, +=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, -, -=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, *, *=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, /, /=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, <<, <<=) \
	_ANKI_ENUM_OPERATOR(enum_, qualifier_, >>, >>=) \
	_ANKI_ENUM_UNARAY_OPERATOR(enum_, qualifier_, ~) \
	_ANKI_ENUM_INCREMENT_DECREMENT(enum_, qualifier_) \
	_ANKI_ENUM_NEGATIVE_OPERATOR(enum_, qualifier_)

/// Convert enum to the underlying type.
template<typename TEnum>
inline typename EnumUnderlyingType<TEnum>::Type enumToValue(TEnum e)
{
	return static_cast<typename EnumUnderlyingType<TEnum>::Type>(e);
}

/// Convert enum to the underlying type.
template<typename TEnum>
inline TEnum valueToEnum(typename EnumUnderlyingType<TEnum>::Type v)
{
	return static_cast<TEnum>(v);
}

/// @memberof EnumIterable
template<typename TEnum>
class EnumIterableIterator
{
public:
	using Type = typename EnumUnderlyingType<TEnum>::Type;

	EnumIterableIterator(TEnum val)
		: m_val(static_cast<Type>(val))
	{
	}

	TEnum operator*() const
	{
		return static_cast<TEnum>(m_val);
	}

	void operator++()
	{
		++m_val;
	}

	bool operator!=(EnumIterableIterator b) const
	{
		return m_val != b.m_val;
	}

private:
	Type m_val;
};

/// Allow an enum to be used in a for range loop.
/// @code
/// for(SomeEnum type : EnumIterable<SomeEnum>())
/// {
/// 	...
/// }
/// @endcode
template<typename TEnum>
class EnumIterable
{
public:
	using Iterator = EnumIterableIterator<TEnum>;

	static Iterator begin()
	{
		return Iterator(TEnum::FIRST);
	}

	static Iterator end()
	{
		return Iterator(TEnum::COUNT);
	}
};
/// @}
