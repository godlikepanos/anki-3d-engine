// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Common.h>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace anki {

/// @addtogroup util_other
/// @{

using I8 = int8_t; ///< Integer 8bit
constexpr I8 kMaxI8 = std::numeric_limits<I8>::max();
constexpr I8 kMinI8 = std::numeric_limits<I8>::min();

using I16 = int16_t; ///< Integer 16bit
constexpr I16 kMaxI16 = std::numeric_limits<I16>::max();
constexpr I16 kMinI16 = std::numeric_limits<I16>::min();

using I32 = int32_t; ///< Integer 32bit
constexpr I32 kMaxI32 = std::numeric_limits<I32>::max();
constexpr I32 kMinI32 = std::numeric_limits<I32>::min();

using I64 = int64_t; ///< Integer 64bit
constexpr I64 kMaxI64 = std::numeric_limits<I64>::max();
constexpr I64 kMinI64 = std::numeric_limits<I64>::min();

using I = int_fast32_t; ///< Fast signed integer at least 32bit
constexpr I kMaxI = std::numeric_limits<I>::max();
constexpr I kMinI = std::numeric_limits<I>::min();

using U8 = uint8_t; ///< Unsigned integer 8bit
constexpr U8 kMaxU8 = std::numeric_limits<U8>::max();
constexpr U8 kMinU8 = std::numeric_limits<U8>::min();

using U16 = uint16_t; ///< Unsigned integer 16bit
constexpr U16 kMaxU16 = std::numeric_limits<U16>::max();
constexpr U16 kMinU16 = std::numeric_limits<U16>::min();

using U32 = uint32_t; ///< Unsigned integer 32bit
constexpr U32 kMaxU32 = std::numeric_limits<U32>::max();
constexpr U32 kMinU32 = std::numeric_limits<U32>::min();

using U64 = uint64_t; ///< Unsigned integer 64bit
constexpr U64 kMaxU64 = std::numeric_limits<U64>::max();
constexpr U64 kMinU64 = std::numeric_limits<U64>::min();

using U = uint_fast32_t; ///< Fast unsigned integer at least 32bit
constexpr U kMaxU = std::numeric_limits<U>::max();
constexpr U kMinU = std::numeric_limits<U>::min();

using PtrSize = size_t; ///< Like size_t
constexpr PtrSize kMaxPtrSize = std::numeric_limits<PtrSize>::max();
constexpr PtrSize kMinPtrSize = std::numeric_limits<PtrSize>::min();
static_assert(sizeof(PtrSize) == sizeof(void*), "Wrong size for size_t");

using F32 = float; ///< Floating point 32bit
constexpr F32 kMaxF32 = std::numeric_limits<F32>::max();
constexpr F32 kMinF32 = -std::numeric_limits<F32>::max();

using F64 = double; ///< Floating point 64bit
constexpr F64 kMaxF64 = std::numeric_limits<F64>::max();
constexpr F64 kMinF64 = -std::numeric_limits<F64>::max();

using Bool = bool; ///< 1 byte boolean type. The same as C++'s bool.
static_assert(sizeof(bool) == 1, "Wrong size for bool");

using Bool32 = I32;
using Char = char;
using WChar = wchar_t;

using Second = F64; ///< The base time unit is second.
constexpr Second kMaxSecond = kMaxF64;
constexpr Second kMinSecond = kMinF64;

using Timestamp = U64; ///< Timestamp type.
constexpr Timestamp kMaxTimestamp = kMaxU64;

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

ANKI_DO_LIMIT(I8, kMinI8, kMaxI8)
ANKI_DO_LIMIT(I16, kMinI16, kMaxI16)
ANKI_DO_LIMIT(I32, kMinI32, kMaxI32)
ANKI_DO_LIMIT(I64, kMinI64, kMaxI64)
ANKI_DO_LIMIT(U8, kMinU8, kMaxU8)
ANKI_DO_LIMIT(U16, kMinU16, kMaxU16)
ANKI_DO_LIMIT(U32, kMinU32, kMaxU32)
ANKI_DO_LIMIT(U64, kMinU64, kMaxU64)
ANKI_DO_LIMIT(F32, kMinF32, kMaxF32)
ANKI_DO_LIMIT(F64, kMinF64, kMaxF64)

#undef ANKI_DO_LIMIT

/// Representation of error and a wrapper on top of error codes.
class [[nodiscard]] Error
{
public:
	/// @name Error codes
	/// @{
	static constexpr I32 kNone = 0;
	static constexpr I32 kOutOfMemory = 1;
	static constexpr I32 kFunctionFailed = 2; ///< External operation failed
	static constexpr I32 kUserData = 3;

	// File errors
	static constexpr I32 kFileNotFound = 4;
	static constexpr I32 kFileAccess = 5; ///< Read/write access error

	static constexpr I32 kUnknown = 6;
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
		return ANKI_UNLIKELY(m_code != kNone);
	}

	/// @privatesection
	/// @{
	I32 _getCode() const
	{
		return m_code;
	}
	/// @}

private:
	I32 m_code = kNone;
};

/// Macro to check if a method/function returned an error. It will return on error.
#define ANKI_CHECK(x_) \
	do \
	{ \
		const Error retError = x_; \
		if(retError) \
		{ \
			return retError; \
		} \
	} while(0)

/// Macro to check if a method/function returned an error.
#define ANKI_CHECK_AND_IGNORE(x_) \
	do \
	{ \
		[[maybe_unused]] const Error retError = x_; \
	} while(0)

#if ANKI_EXTRA_CHECKS
#	define ANKI_DEBUG_CODE(x) x
#else
#	define ANKI_DEBUG_CODE(x)
#endif

/// @name AnKi type user literals.
/// @{
inline constexpr U8 operator"" _U8(unsigned long long arg) noexcept
{
	return U8(arg);
}

inline constexpr U16 operator"" _U16(unsigned long long arg) noexcept
{
	return U16(arg);
}

inline constexpr U32 operator"" _U32(unsigned long long arg) noexcept
{
	return U32(arg);
}

inline constexpr U64 operator"" _U64(unsigned long long arg) noexcept
{
	return U64(arg);
}
/// @}

/// @name Size user literals
/// @{
inline constexpr unsigned long long int operator""_B(unsigned long long int x)
{
	return x;
}

inline constexpr unsigned long long int operator""_KB(unsigned long long int x)
{
	return x * 1024;
}

inline constexpr unsigned long long int operator""_MB(unsigned long long int x)
{
	return x * (1024 * 1024);
}

inline constexpr unsigned long long int operator""_GB(unsigned long long int x)
{
	return x * (1024 * 1024 * 1024);
}
/// @}

/// @name Time user literals
/// @{
inline constexpr Second operator""_hour(long double x)
{
	return Second(x) * 60.0;
}

inline constexpr Second operator""_sec(long double x)
{
	return Second(x);
}

inline constexpr Second operator""_ms(long double x)
{
	return Second(x) / 1000.0;
}

inline constexpr Second operator""_ns(long double x)
{
	return Second(x) / 1000000000.0;
}
/// @}

/// @name Distance user literals
/// @{
inline constexpr F32 operator""_dm(long double x)
{
	return F32(x) / 10.0f;
}

inline constexpr F32 operator""_cm(long double x)
{
	return F32(x) / 100.0f;
}

inline constexpr F32 operator""_mm(long double x)
{
	return F32(x) / 1000.0f;
}
/// @}

/// Convenience macro that defines the type of a class.
#define ANKI_DEFINE_CLASS_SELF(selfType) \
	typedef auto _selfFn##selfType()->decltype(*this); \
	using _SelfRef##selfType = decltype(((_selfFn##selfType*)0)()); \
	using selfType = std::remove_reference<_SelfRef##selfType>::type;
/// @}

} // end namespace anki
