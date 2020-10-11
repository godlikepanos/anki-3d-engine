// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
constexpr I8 MAX_I8 = std::numeric_limits<I8>::max();
constexpr I8 MIN_I8 = std::numeric_limits<I8>::min();

using I16 = int16_t; ///< Integer 16bit
constexpr I16 MAX_I16 = std::numeric_limits<I16>::max();
constexpr I16 MIN_I16 = std::numeric_limits<I16>::min();

using I32 = int32_t; ///< Integer 32bit
constexpr I32 MAX_I32 = std::numeric_limits<I32>::max();
constexpr I32 MIN_I32 = std::numeric_limits<I32>::min();

using I64 = int64_t; ///< Integer 64bit
constexpr I64 MAX_I64 = std::numeric_limits<I64>::max();
constexpr I64 MIN_I64 = std::numeric_limits<I64>::min();

using I = int_fast32_t; ///< Fast signed integer at least 32bit
constexpr I MAX_I = std::numeric_limits<I>::max();
constexpr I MIN_I = std::numeric_limits<I>::min();

using U8 = uint8_t; ///< Unsigned integer 8bit
constexpr U8 MAX_U8 = std::numeric_limits<U8>::max();
constexpr U8 MIN_U8 = std::numeric_limits<U8>::min();

using U16 = uint16_t; ///< Unsigned integer 16bit
constexpr U16 MAX_U16 = std::numeric_limits<U16>::max();
constexpr U16 MIN_U16 = std::numeric_limits<U16>::min();

using U32 = uint32_t; ///< Unsigned integer 32bit
constexpr U32 MAX_U32 = std::numeric_limits<U32>::max();
constexpr U32 MIN_U32 = std::numeric_limits<U32>::min();

using U64 = uint64_t; ///< Unsigned integer 64bit
constexpr U64 MAX_U64 = std::numeric_limits<U64>::max();
constexpr U64 MIN_U64 = std::numeric_limits<U64>::min();

using U = uint_fast32_t; ///< Fast unsigned integer at least 32bit
constexpr U MAX_U = std::numeric_limits<U>::max();
constexpr U MIN_U = std::numeric_limits<U>::min();

using PtrSize = size_t; ///< Like size_t
constexpr PtrSize MAX_PTR_SIZE = std::numeric_limits<PtrSize>::max();
constexpr PtrSize MIN_PTR_SIZE = std::numeric_limits<PtrSize>::min();
static_assert(sizeof(PtrSize) == sizeof(void*), "Wrong size for size_t");

using F32 = float; ///< Floating point 32bit
constexpr F32 MAX_F32 = std::numeric_limits<F32>::max();
constexpr F32 MIN_F32 = -std::numeric_limits<F32>::max();

using F64 = double; ///< Floating point 64bit
constexpr F64 MAX_F64 = std::numeric_limits<F64>::max();
constexpr F64 MIN_F64 = -std::numeric_limits<F64>::max();

using Bool = bool; ///< 1 byte boolean type. The same as C++'s bool.
static_assert(sizeof(bool) == 1, "Wrong size for bool");

using Bool32 = I32;

using Second = F64; ///< The base time unit is second.
constexpr Second MAX_SECOND = MAX_F64;
constexpr Second MIN_SECOND = MIN_F64;

using Timestamp = U64; ///< Timestamp type.
constexpr Timestamp MAX_TIMESTAMP = MAX_U64;

// Numeric limits
template<typename T>
constexpr T getMinNumericLimit();

template<typename T>
constexpr T getMaxNumericLimit();

#define ANKI_DO_LIMIT(type, min, max) \
	template<> \
	constexpr type getMinNumericLimit() \
	{ \
		return min; \
	} \
	template<> \
	constexpr type getMaxNumericLimit() \
	{ \
		return max; \
	}

ANKI_DO_LIMIT(I8, MIN_I8, MAX_I8)
ANKI_DO_LIMIT(I16, MIN_I16, MAX_I16)
ANKI_DO_LIMIT(I32, MIN_I32, MAX_I32)
ANKI_DO_LIMIT(I64, MIN_I64, MAX_I64)
ANKI_DO_LIMIT(U8, MIN_U8, MAX_U8)
ANKI_DO_LIMIT(U16, MIN_U16, MAX_U16)
ANKI_DO_LIMIT(U32, MIN_U32, MAX_U32)
ANKI_DO_LIMIT(U64, MIN_U64, MAX_U64)
ANKI_DO_LIMIT(F32, MIN_F32, MAX_F32)
ANKI_DO_LIMIT(F64, MIN_F64, MAX_F64)

#undef ANKI_DO_LIMIT

/// @name AnKi type literals.
/// @{
inline constexpr U8 operator"" _U8(unsigned long long arg) noexcept
{
	return static_cast<U8>(arg);
}

inline constexpr U16 operator"" _U16(unsigned long long arg) noexcept
{
	return static_cast<U16>(arg);
}

inline constexpr U32 operator"" _U32(unsigned long long arg) noexcept
{
	return static_cast<U32>(arg);
}

inline constexpr U64 operator"" _U64(unsigned long long arg) noexcept
{
	return static_cast<U64>(arg);
}
/// @}

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
	explicit operator Bool() const
	{
		return ANKI_UNLIKELY(m_code != NONE);
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
		if(error) \
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

#if ANKI_EXTRA_CHECKS
#	define ANKI_DEBUG_CODE(x) x
#else
#	define ANKI_DEBUG_CODE(x)
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

#if ANKI_COMPILER_GCC_COMPATIBLE
/// Redefine the sizeof because the default returns size_t and that will require casting when used with U32 for example.
#	define _ANKI_SIZEOF(type) ((anki::U32)(sizeof(type)))
#	define sizeof(type) _ANKI_SIZEOF(type)

/// Redefine the alignof because the default returns size_t and that will require casting when used with U32 for
/// example.
#	define _ANKI_ALIGNOF(type) ((anki::U32)(alignof(type)))
#	define alignof(type) _ANKI_ALIGNOF(type)
#endif
/// @}

} // end namespace anki
