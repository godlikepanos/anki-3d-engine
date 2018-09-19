// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Common.h>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace anki
{

/// @addtogroup util_other
/// @{

using I8 = int8_t; ///< Integer 8bit
const I8 MAX_I8 = std::numeric_limits<I8>::max();
const I8 MIN_I8 = std::numeric_limits<I8>::min();

using I16 = int16_t; ///< Integer 16bit
const I16 MAX_I16 = std::numeric_limits<I16>::max();
const I16 MIN_I16 = std::numeric_limits<I16>::min();

using I32 = int32_t; ///< Integer 32bit
const I32 MAX_I32 = std::numeric_limits<I32>::max();
const I32 MIN_I32 = std::numeric_limits<I32>::min();

using I64 = int64_t; ///< Integer 64bit
const I64 MAX_I64 = std::numeric_limits<I64>::max();
const I64 MIN_I64 = std::numeric_limits<I64>::min();

using I = int_fast32_t; ///< Fast signed integer at least 32bit
const I MAX_I = std::numeric_limits<I>::max();
const I MIN_I = std::numeric_limits<I>::min();

using U8 = uint8_t; ///< Unsigned integer 8bit
const U8 MAX_U8 = std::numeric_limits<U8>::max();
const U8 MIN_U8 = std::numeric_limits<U8>::min();

using U16 = uint16_t; ///< Unsigned integer 16bit
const U16 MAX_U16 = std::numeric_limits<U16>::max();
const U16 MIN_U16 = std::numeric_limits<U16>::min();

using U32 = uint32_t; ///< Unsigned integer 32bit
const U32 MAX_U32 = std::numeric_limits<U32>::max();
const U32 MIN_U32 = std::numeric_limits<U32>::min();

using U64 = uint64_t; ///< Unsigned integer 64bit
const U64 MAX_U64 = std::numeric_limits<U64>::max();
const U64 MIN_U64 = std::numeric_limits<U64>::min();

using U = uint_fast32_t; ///< Fast unsigned integer at least 32bit
const U MAX_U = std::numeric_limits<U>::max();
const U MIN_U = std::numeric_limits<U>::min();

using PtrSize = size_t; ///< Like size_t
const PtrSize MAX_PTR_SIZE = std::numeric_limits<PtrSize>::max();
const PtrSize MIN_PTR_SIZE = std::numeric_limits<PtrSize>::min();
static_assert(sizeof(PtrSize) == sizeof(void*), "Wrong size for size_t");

using F32 = float; ///< Floating point 32bit
const F32 MAX_F32 = std::numeric_limits<F32>::max();
const F32 MIN_F32 = -std::numeric_limits<F32>::max();

using F64 = double; ///< Floating point 64bit
const F64 MAX_F64 = std::numeric_limits<F64>::max();
const F64 MIN_F64 = -std::numeric_limits<F64>::max();

using Bool = int; ///< Fast boolean type
using Bool8 = I8; ///< Small 8bit boolean type
using Bool32 = I32; ///< A 32bit boolean

using Second = F64; ///< The base time unit is second.
const Second MAX_SECOND = MAX_F64;
const Second MIN_SECOND = MIN_F64;

using Timestamp = U64; ///< Timestamp type.
const Timestamp MAX_TIMESTAMP = MAX_U64;

/// Representation of error and a wrapper on top of error codes.
class Error
{
public:
	/// @name Error codes
	/// @{
	static constexpr I32 NONE = 0;
	static constexpr I32 OUT_OF_MEMORY = 1;
	static constexpr I32 FUNCTION_FAILED = 2; ///< External operation failed
	static constexpr I32 USER_DATA = 3;

	// File errors
	static constexpr I32 FILE_NOT_FOUND = 4;
	static constexpr I32 FILE_ACCESS = 5; ///< Read/write access error

	static constexpr I32 UNKNOWN = 6;
	/// @}

	/// Construct using an error code.
	Error(I32 code)
		: m_code(code)
	{
	}

	/// Copy.
	Error(const Error& b)
		: m_code(b.m_code)
	{
	}

	/// Copy.
	Error& operator=(const Error& b)
	{
		m_code = b.m_code;
		return *this;
	}

	/// Compare.
	Bool operator==(const Error& b) const
	{
		return m_code == b.m_code;
	}

	/// Compare.
	Bool operator==(I32 code) const
	{
		return m_code == code;
	}

	/// Compare.
	Bool operator!=(const Error& b) const
	{
		return m_code != b.m_code;
	}

	/// Compare.
	Bool operator!=(I32 code) const
	{
		return m_code != code;
	}

	/// Check if it is an error.
	operator Bool() const
	{
		return m_code != NONE;
	}

	/// @privatesection
	/// @{
	I32 _getCode() const
	{
		return m_code;
	}
	/// @}

private:
	I32 m_code = NONE;
};

/// Macro to check if a method/function returned an error.
#define ANKI_CHECK(x_) \
	do \
	{ \
		Error error = x_; \
		if(ANKI_UNLIKELY(error)) \
		{ \
			return error; \
		} \
	} while(0)

/// Macro the check if a memory allocation is OOM.
#define ANKI_CHECK_OOM(x_) \
	do \
	{ \
		if(ANKI_UNLIKELY(x_ == nullptr)) \
		{ \
			return Error::OUT_OF_MEMORY; \
		} \
	} while(0)

/// Macro to nuliffy a pointer on debug builds.
#if ANKI_EXTRA_CHECKS
#	define ANKI_DBG_NULLIFY = {}
#else
#	define ANKI_DBG_NULLIFY
#endif

/// @name Size user literals
/// @{
static constexpr unsigned long long int operator""_B(unsigned long long int x)
{
	return x;
}

static constexpr unsigned long long int operator""_KB(unsigned long long int x)
{
	return x * 1024;
}

static constexpr unsigned long long int operator""_MB(unsigned long long int x)
{
	return x * (1024 * 1024);
}

static constexpr unsigned long long int operator""_GB(unsigned long long int x)
{
	return x * (1024 * 1024 * 1024);
}
/// @}

/// @name Time user literals
/// @{
static constexpr long double operator""_ms(long double x)
{
	return x / 1000.0;
}

static constexpr long double operator""_ns(long double x)
{
	return x / 1000000000.0;
}
/// @}

/// Convenience macro that defines the type of a class.
#define ANKI_DEFINE_CLASS_SELF \
	typedef auto _selfFn()->decltype(*this); \
	using _SelfRef = decltype(((_selfFn*)0)()); \
	using Self = std::remove_reference<_SelfRef>::type;
/// @}

} // end namespace anki
