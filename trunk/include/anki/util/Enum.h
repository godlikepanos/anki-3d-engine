// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_ENUM_H
#define ANKI_UTIL_ENUM_H

#include "anki/Config.h"

/// @privatesection
/// @{

// Re-implement std::underlying_type to avoid including the whole typetraits
// header
template<typename TEnum>
struct EnumUnderlyingType
{
	using Type = __underlying_type(TEnum);
};

#define _ANKI_ENUM_OPERATOR(enum_, qualifier_, operator_, selfOperator_) \
	qualifier_ enum_ operator operator_ (enum_ a, enum_ b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>( \
			static_cast<Int>(a) operator_ static_cast<Int>(b)); \
	} \
	qualifier_ enum_ operator operator_ ( \
		enum_ a, EnumUnderlyingType<enum_>::Type b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(static_cast<Int>(a) operator_ b); \
	} \
	qualifier_ enum_ operator operator_ ( \
		EnumUnderlyingType<enum_>::Type a, enum_ b) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(a operator_ static_cast<Int>(b)); \
	} \
	qualifier_ enum_& operator selfOperator_ (enum_& a, enum_ b) \
	{ \
		a = a operator_ b; \
		return a; \
	} \

#define _ANKI_ENUM_UNARAY_OPERATOR(enum_, qualifier_, operator_) \
	qualifier_ enum_ operator operator_ (enum_ a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		return static_cast<enum_>(operator_ static_cast<Int>(a)); \
	}

#define _ANKI_ENUM_INCREMENT_DECREMENT(enum_, qualifier_) \
	qualifier_ enum_& operator++(enum_& a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		a = static_cast<enum_>(static_cast<Int>(a) + 1); \
		return  a; \
	} \
	qualifier_ enum_& operator--(enum_& a) \
	{ \
		using Int = EnumUnderlyingType<enum_>::Type; \
		a = static_cast<enum_>(static_cast<Int>(a) - 1); \
		return  a; \
	}

/// @}

/// @addtogroup util_other
/// @{

/// Implement all those functions that will make a stronly typed enum behave 
/// like the old type of enums.
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
	_ANKI_ENUM_INCREMENT_DECREMENT(enum_, qualifier_)

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

/// @}

#endif


