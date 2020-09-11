// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Assert.h>

namespace anki
{

/// @privatesection
/// @{

/// This is a template where the Type will be I64 if the T is any unsigned integer and U64 otherwise
template<typename T, bool = std::is_unsigned<T>::value>
class EnumSafeIntegerType
{
public:
	using Type = I64;
};

template<typename T>
class EnumSafeIntegerType<T, true>
{
public:
	using Type = U64;
};

/// This macro will do an operation between 2 values. It will be used in constexpr functions. There is also an assertion
/// which makes sure that the result will fit in an enum. Despite the fact that the assertion contains non-constexpr
/// elements it will work on constexpr expressions. The compiler will compile-time ignore the non-constexpr part if the
/// assert if the assertion expression is true.
#define _ANKI_ENUM_OPERATION_BODY(enumType, regularOperator, a, b) \
	using EnumInt = std::underlying_type<enumType>::type; \
	using SafeInt = EnumSafeIntegerType<EnumInt>::Type; \
	const SafeInt c = SafeInt(a) regularOperator SafeInt(b); \
	ANKI_ASSERT(c <= SafeInt(std::numeric_limits<EnumInt>::max())); \
	return enumType(c)

#define _ANKI_ENUM_OPERATOR(enumType, qualifier, regularOperator, assignmentOperator) \
	constexpr qualifier enumType operator regularOperator(const enumType a, const enumType b) \
	{ \
		_ANKI_ENUM_OPERATION_BODY(enumType, regularOperator, a, b); \
	} \
	constexpr qualifier enumType operator regularOperator(const enumType a, \
														  const std::underlying_type<enumType>::type b) \
	{ \
		_ANKI_ENUM_OPERATION_BODY(enumType, regularOperator, a, b); \
	} \
	constexpr qualifier enumType operator regularOperator(const std::underlying_type<enumType>::type a, \
														  const enumType b) \
	{ \
		_ANKI_ENUM_OPERATION_BODY(enumType, regularOperator, a, b); \
	} \
	qualifier enumType& operator assignmentOperator(enumType& a, const enumType b) \
	{ \
		a = a regularOperator b; \
		return a; \
	} \
	qualifier enumType& operator assignmentOperator(enumType& a, const std::underlying_type<enumType>::type b) \
	{ \
		a = a regularOperator b; \
		return a; \
	} \
	qualifier std::underlying_type<enumType>::type& operator assignmentOperator( \
		std::underlying_type<enumType>::type& a, const enumType b) \
	{ \
		using EnumInt = std::underlying_type<enumType>::type; \
		a = EnumInt(a regularOperator b); \
		return a; \
	}

#define _ANKI_ENUM_UNARAY_OPERATOR(enumType, qualifier, regularOperator) \
	constexpr qualifier enumType operator regularOperator(const enumType a) \
	{ \
		using EnumInt = std::underlying_type<enumType>::type; \
		return enumType(regularOperator EnumInt(a)); \
	}

#define _ANKI_ENUM_INCREMENT_DECREMENT(enumType, qualifier) \
	qualifier enumType& operator++(enumType& a) \
	{ \
		a = a + 1; \
		return a; \
	} \
	qualifier enumType& operator--(enumType& a) \
	{ \
		a = a - 1; \
		return a; \
	} \
	qualifier enumType operator++(enumType& a, int) \
	{ \
		const enumType old = a; \
		++a; \
		return old; \
	} \
	qualifier enumType operator--(enumType& a, int) \
	{ \
		const enumType old = a; \
		--a; \
		return old; \
	}

#define _ANKI_ENUM_NEGATIVE_OPERATOR(enumType, qualifier) \
	qualifier Bool operator!(const enumType a) \
	{ \
		using EnumInt = std::underlying_type<enumType>::type; \
		return EnumInt(a) == 0; \
	}

#define _ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType, qualifier) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, |, |=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, &, &=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, ^, ^=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, +, +=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, -, -=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, *, *=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, /, /=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, <<, <<=) \
	_ANKI_ENUM_OPERATOR(enumType, qualifier, >>, >>=) \
	_ANKI_ENUM_UNARAY_OPERATOR(enumType, qualifier, ~) \
	_ANKI_ENUM_INCREMENT_DECREMENT(enumType, qualifier) \
	_ANKI_ENUM_NEGATIVE_OPERATOR(enumType, qualifier)
/// @}

/// @addtogroup util_other
/// @{

/// Implement all those functions that will make a stronly typed enum behave like the old type of enums.
#define ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType) _ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType, inline)

/// Same as ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS but for enums that are defined in a class.
#define ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS_FRIEND(enumType) _ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType, friend)

/// @memberof EnumIterable
template<typename TEnum>
class EnumIterableIterator
{
public:
	using Type = typename std::underlying_type<TEnum>::type;

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

} // end namespace anki
