// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// Contains misc functions

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Assert.h>
#include <cmath>
#include <utility>
#include <new>
#include <cstring>
#include <algorithm>
#include <functional>

namespace anki
{

/// @addtogroup util_other
/// @{

#define _ANKI_CONCATENATE(a, b) a##b

/// Concatenate 2 preprocessor tokens.
#define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#define _ANKI_STRINGIZE(a) #a

/// Make a preprocessor token a string.
#define ANKI_STRINGIZE(a) _ANKI_STRINGIZE(a)

/// Format to print bits
#define ANKI_PRIb8 "c%c%c%c%c%c%c%c"
#define ANKI_PRIb16 ANKI_PRIb8 "%c%c%c%c%c%c%c%c"
#define ANKI_PRIb32 ANKI_PRIb16 "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define ANKI_PRIb64 ANKI_PRIb32 "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"

#define _ANKI_FORMAT_HELPER(byte, bit) (U64(byte) & (U64(1) << U64(bit))) ? '1' : '0'

#define ANKI_FORMAT_U8(byte) \
	_ANKI_FORMAT_HELPER((byte), 7), _ANKI_FORMAT_HELPER((byte), 6), _ANKI_FORMAT_HELPER((byte), 5), \
		_ANKI_FORMAT_HELPER((byte), 4), _ANKI_FORMAT_HELPER((byte), 3), _ANKI_FORMAT_HELPER((byte), 2), \
		_ANKI_FORMAT_HELPER((byte), 1), _ANKI_FORMAT_HELPER((byte), 0)

#define ANKI_FORMAT_U16(u16) ANKI_FORMAT_U8(u16 >> 8), ANKI_FORMAT_U8(u16)
#define ANKI_FORMAT_U32(u32) ANKI_FORMAT_U16(u32 >> 16), ANKI_FORMAT_U16(u32)
#define ANKI_FORMAT_U64(u64) ANKI_FORMAT_U32(u64 >> 32), ANKI_FORMAT_U32(u64)

// ANKI_ENABLE_METHOD & ANKI_ENABLE_ARG trickery copied from Tick library
template<typename T, int N>
struct DummyType
{
};

#if defined(_MSC_VER)
template<bool B>
struct RequiresBool
{
	static constexpr bool VALUE = B;
};

template<typename T, int N>
struct RequiresUnwrap : T
{
};

template<int N>
struct PrivateEnum
{
	enum class Type
	{
		NA
	};
};

#	define ANKI_REQUIRES_BOOL(line, ...) RequiresUnwrap<decltype(RequiresBool<(__VA_ARGS__)>{}), line>::VALUE

#	define ANKI_ENABLE_INTERNAL(line, ...) \
		typename PrivateEnum<line>::Type ANKI_CONCATENATE( \
			privateEnum, line) = PrivateEnum<line>::Type::NA, \
						 bool ANKI_CONCATENATE(privateBool, line) = true, \
						 typename = typename std::enable_if_t<(ANKI_CONCATENATE(privateBool, line) \
															   && ANKI_REQUIRES_BOOL(line, __VA_ARGS__))>
#else

#	define ANKI_ENABLE_INTERNAL(line, ...) \
		bool privateBool##line = true, typename std::enable_if_t<(privateBool##line && __VA_ARGS__), int> = 0

#endif

/// Use it to enable a method based on a constant expression.
/// @code
/// template<int N> class Foo {
/// 	ANKI_ENABLE_METHOD(N == 10)
/// 	void foo() {}
///	};
/// @endcode
#define ANKI_ENABLE_METHOD(...) template<ANKI_ENABLE_INTERNAL(__LINE__, __VA_ARGS__)>

/// Use it to enable a method based on a constant expression.
/// @code
/// class Foo {
/// 	void foo(ANKI_ENABLE_ARG(Boo, expr) b) {}
///	};
/// @endcode
#define ANKI_ENABLE_ARG(type_, expression) \
	typename std::conditional<(expression), type_, DummyType<type_, __LINE__>>::type

/// Use it to enable a method based on a constant expression.
/// @code
/// template<typename T, ANKI_ENABLE(std::is_whatever<T>::value)>
/// void foo(T x) {}
/// @endcode
#define ANKI_ENABLE(...) ANKI_ENABLE_INTERNAL(__LINE__, __VA_ARGS__)

/// OS specific debug breakpoint
#if ANKI_OS_WINDOWS
#	define ANKI_DEBUG_BREAK() __debugbreak()
#else
#	define ANKI_DEBUG_BREAK() abort()
#endif

/// Get a pseudo random number.
U64 getRandom();

/// Pick a random number from min to max
template<typename T, ANKI_ENABLE(std::is_floating_point<T>::value)>
T getRandomRange(T min, T max)
{
	ANKI_ASSERT(min <= max);
	const F64 r = F64(getRandom()) / F64(MAX_U64);
	return T(min + r * (max - min));
}

template<typename T, ANKI_ENABLE(std::is_integral<T>::value)>
T getRandomRange(T min, T max)
{
	ANKI_ASSERT(min <= max);
	const U64 r = getRandom();
	return T(r % U64(max - min + 1)) + min;
}

/// Get min of two values.
template<typename T>
inline constexpr T min(T a, T b)
{
	return (a < b) ? a : b;
}

/// Get max of two values.
template<typename T>
inline constexpr T max(T a, T b)
{
	return (a > b) ? a : b;
}

/// Check if a number is a power of 2
template<typename Int, ANKI_ENABLE(std::is_integral<Int>::value)>
inline constexpr Bool isPowerOfTwo(Int x)
{
	return !(x == 0) && !(x & (x - 1));
}

/// Get the next power of two number. For example if x is 130 this will return 256.
template<typename Int, ANKI_ENABLE(std::is_integral<Int>::value)>
inline constexpr Int nextPowerOfTwo(Int x)
{
	const F64 d = F64(x);
	const F64 res = pow(2.0, ceil(log(d) / log(2.0)));
	return Int(res);
}

/// Get the aligned number rounded up.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TInt, ANKI_ENABLE(std::is_integral<TInt>::value)>
inline constexpr TInt getAlignedRoundUp(PtrSize alignment, TInt value)
{
	ANKI_ASSERT(alignment > 0);
	PtrSize v = PtrSize(value);
	v = ((v + alignment - 1) / alignment) * alignment;
	return TInt(v);
}

/// Get the aligned number rounded up.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TFloat, ANKI_ENABLE(std::is_floating_point<TFloat>::value)>
inline constexpr TFloat getAlignedRoundUp(TFloat alignment, TFloat value)
{
	ANKI_ASSERT(alignment > TFloat(0.0));
	return ceil(value / alignment) * alignment;
}

/// Align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TAlignment, typename TValue>
inline void alignRoundUp(TAlignment alignment, TValue& value)
{
	value = getAlignedRoundUp(alignment, value);
}

/// Get the aligned number rounded down.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TInt, ANKI_ENABLE(std::is_integral<TInt>::value)>
inline constexpr TInt getAlignedRoundDown(PtrSize alignment, TInt value)
{
	ANKI_ASSERT(alignment > 0);
	PtrSize v = PtrSize(value);
	v = (v / alignment) * alignment;
	return TInt(v);
}

/// Get the aligned number rounded down.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TFloat, ANKI_ENABLE(std::is_floating_point<TFloat>::value)>
inline constexpr TFloat getAlignedRoundDown(TFloat alignment, TFloat value)
{
	ANKI_ASSERT(alignment > TFloat(0.0));
	return floor(value / alignment) * alignment;
}

/// Align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TAlignment, typename TValue>
inline void alignRoundDown(TAlignment alignment, TValue& value)
{
	value = getAlignedRoundDown(alignment, value);
}

/// Check if a number is aligned
template<typename Type>
inline constexpr Bool isAligned(PtrSize alignment, Type value)
{
	return (PtrSize(value) % alignment) == 0;
}

template<typename T>
inline void swapValues(T& a, T& b)
{
	const T tmp = b;
	b = a;
	a = tmp;
}

/// Convert any pointer to a number.
template<typename TPtr>
inline PtrSize ptrToNumber(TPtr ptr)
{
	const uintptr_t i = reinterpret_cast<uintptr_t>(ptr);
	const PtrSize size = i;
	return size;
}

/// Convert a number to a pointer.
template<typename TPtr>
inline constexpr TPtr numberToPtr(PtrSize num)
{
	uintptr_t i = static_cast<uintptr_t>(num);
	TPtr ptr = reinterpret_cast<TPtr>(i);
	return ptr;
}

/// A simple template trick to remove the pointer from one type
///
/// Example:
/// @code
/// double a = 1234.456;
/// RemovePointer<decltype(&a)>::Type b = a;
/// @endcode
/// The b is of type double
template<typename T>
struct RemovePointer;

template<typename T>
struct RemovePointer<T*>
{
	using Type = T;
};

/// Zero memory of an object
template<typename T>
void zeroMemory(T& x)
{
	memset(&x, 0, sizeof(T));
}

/// Find a value in a shorted container.
template<class TForwardIterator, class T, class TCompare = std::less<>>
TForwardIterator binarySearch(TForwardIterator first, TForwardIterator last, const T& value, TCompare comp = {})
{
	first = std::lower_bound(first, last, value, comp);
	return (first != last && !comp(value, *first)) ? first : last;
}

/// Individual classes should specialize that function if they are packed. If a class is packed it can be used as
/// whole in hashing.
template<typename T>
constexpr Bool isPacked()
{
	return false;
}

/// Unflatten 3D array index.
/// Imagine an array [sizeA][sizeB][sizeC] and a flat index in that array. Then this function will compute the unflatten
/// indices.
template<typename T, typename TI, typename TOut>
inline void unflatten3dArrayIndex(const T sizeA, const T sizeB, const T sizeC, const TI flatIdx, TOut& a, TOut& b,
								  TOut& c)
{
	ANKI_ASSERT(flatIdx < (sizeA * sizeB * sizeC));
	a = (flatIdx / (sizeB * sizeC)) % sizeA;
	b = (flatIdx / sizeC) % sizeB;
	c = flatIdx % sizeC;
}

/// Given a threaded problem split it into smaller ones. This function accepts the number of threads and the size of
/// the threaded problem. Then given a thread index it chooses a range that the thread can operate into. That range is
/// supposed to be as evenly split as possible across threads.
inline void splitThreadedProblem(U32 threadId, U32 threadCount, U32 problemSize, U32& start, U32& end)
{
	ANKI_ASSERT(threadCount > 0 && threadId < threadCount);
	const U32 div = problemSize / threadCount;
	start = threadId * div;
	end = (threadId == threadCount - 1) ? problemSize : (threadId + 1u) * div;
	ANKI_ASSERT(!(threadId == threadCount - 1 && end != problemSize));
}
/// @}

} // end namespace anki
