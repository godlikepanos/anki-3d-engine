// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// Contains misc functions

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Forward.h>
#include <AnKi/Util/Assert.h>
#include <cmath>
#include <utility>
#include <new>
#include <cstring>
#include <algorithm>
#include <functional>
#include <bit>

namespace anki {

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
	_ANKI_FORMAT_HELPER((byte), 7), _ANKI_FORMAT_HELPER((byte), 6), _ANKI_FORMAT_HELPER((byte), 5), _ANKI_FORMAT_HELPER((byte), 4), \
		_ANKI_FORMAT_HELPER((byte), 3), _ANKI_FORMAT_HELPER((byte), 2), _ANKI_FORMAT_HELPER((byte), 1), _ANKI_FORMAT_HELPER((byte), 0)

#define ANKI_FORMAT_U16(u16) ANKI_FORMAT_U8(u16 >> 8), ANKI_FORMAT_U8(u16)
#define ANKI_FORMAT_U32(u32) ANKI_FORMAT_U16(u32 >> 16), ANKI_FORMAT_U16(u32)
#define ANKI_FORMAT_U64(u64) ANKI_FORMAT_U32(u64 >> 32), ANKI_FORMAT_U32(u64)

/// OS specific debug breakpoint
#if ANKI_OS_WINDOWS
#	define ANKI_DEBUG_BREAK() __debugbreak()
#else
#	define ANKI_DEBUG_BREAK() abort()
#endif

// ANKI_FOREACH
// Stolen from here https://stackoverflow.com/questions/5957679/is-there-a-way-to-use-c-preprocessor-stringification-on-variadic-macro-argumen
#define _ANKI_NUM_ARGS(X100, X99, X98, X97, X96, X95, X94, X93, X92, X91, X90, X89, X88, X87, X86, X85, X84, X83, X82, X81, X80, X79, X78, X77, X76, \
					   X75, X74, X73, X72, X71, X70, X69, X68, X67, X66, X65, X64, X63, X62, X61, X60, X59, X58, X57, X56, X55, X54, X53, X52, X51, \
					   X50, X49, X48, X47, X46, X45, X44, X43, X42, X41, X40, X39, X38, X37, X36, X35, X34, X33, X32, X31, X30, X29, X28, X27, X26, \
					   X25, X24, X23, X22, X21, X20, X19, X18, X17, X16, X15, X14, X13, X12, X11, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, n, ...) \
	n
#define _ANKI_NUM_ARGS2(...) \
	_ANKI_NUM_ARGS(__VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, \
				   71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
				   39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, \
				   6, 5, 4, 3, 2, 1)
#define _ANKI_EXPAND(x) x
#define _ANKI_FIRSTARG(x, ...) (x)
#define _ANKI_RESTARGS(x, ...) (__VA_ARGS__)
#define ANKI_FOREACH(macro, list) _ANKI_FOREACH_(_ANKI_NUM_ARGS2 list, macro, list)
#define _ANKI_FOREACH_(n, m, list) _ANKI_FOREACH__(n, m, list)
#define _ANKI_FOREACH__(n, m, list) _ANKI_FOREACH_##n(m, list)
#define _ANKI_FOREACH_1(m, list) m list
#define _ANKI_FOREACH_2(m, list) _ANKI_EXPAND(m _ANKI_FIRSTARG list), _ANKI_FOREACH_1(m, _ANKI_RESTARGS list)
#define _ANKI_FOREACH_3(m, list) _ANKI_EXPAND(m _ANKI_FIRSTARG list), _ANKI_FOREACH_2(m, _ANKI_RESTARGS list)
#define _ANKI_FOREACH_4(m, list) _ANKI_EXPAND(m _ANKI_FIRSTARG list), _ANKI_FOREACH_3(m, _ANKI_RESTARGS list)
#define _ANKI_FOREACH_5(m, list) _ANKI_EXPAND(m _ANKI_FIRSTARG list), _ANKI_FOREACH_4(m, _ANKI_RESTARGS list)
#define _ANKI_FOREACH_6(m, list) _ANKI_EXPAND(m _ANKI_FIRSTARG list), _ANKI_FOREACH_5(m, _ANKI_RESTARGS list)

/// Get a pseudo random number.
U64 getRandom();

/// Pick a random number from min to max
template<typename T>
T getRandomRange(T min, T max) requires(std::is_floating_point<T>::value)
{
	ANKI_ASSERT(min <= max);
	const F64 r = F64(getRandom()) / F64(kMaxU64);
	return T(min + r * (max - min));
}

template<typename T>
T getRandomRange(T min, T max) requires(std::is_integral<T>::value)
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

template<typename T>
inline constexpr T clamp(T v, T minv, T maxv)
{
	ANKI_ASSERT(minv <= maxv);
	return min<T>(max<T>(minv, v), maxv);
}

/// Check if a number is a power of 2
template<typename Int>
inline constexpr Bool isPowerOfTwo(Int x) requires(std::is_integral<Int>::value)
{
	return !(x == 0) && !(x & (x - 1));
}

/// Get the next power of two number. For example if x is 130 this will return 256.
template<typename Int>
inline constexpr Int nextPowerOfTwo(Int x) requires(std::is_integral<Int>::value)
{
	const F64 d = F64(x);
	const F64 res = pow(2.0, ceil(log(d) / log(2.0)));
	return Int(res);
}

/// Get the previous power of two number. For example if x is 130 this will return 128.
template<typename Int>
inline constexpr Int previousPowerOfTwo(Int x) requires(std::is_integral<Int>::value)
{
	const U64 out = (x != 0) ? (1_U64 << ((sizeof(U64) * 8 - 1) - std::countl_zero<U64>(x))) : 0;
	return Int(out);
}

/// Get the aligned number rounded up.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TInt>
inline constexpr TInt getAlignedRoundUp(PtrSize alignment, TInt value) requires(std::is_integral<TInt>::value)
{
	ANKI_ASSERT(alignment > 0);
	PtrSize v = PtrSize(value);
	v = ((v + alignment - 1) / alignment) * alignment;
	return TInt(v);
}

/// Get the aligned number rounded up.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TFloat>
inline constexpr TFloat getAlignedRoundUp(TFloat alignment, TFloat value) requires(std::is_floating_point<TFloat>::value)
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
template<typename TInt>
inline constexpr TInt getAlignedRoundDown(PtrSize alignment, TInt value) requires(std::is_integral<TInt>::value)
{
	ANKI_ASSERT(alignment > 0);
	PtrSize v = PtrSize(value);
	v = (v / alignment) * alignment;
	return TInt(v);
}

/// Get the aligned number rounded down.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TFloat>
inline constexpr TFloat getAlignedRoundDown(TFloat alignment, TFloat value) requires(std::is_floating_point<TFloat>::value)
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

/// Given two alignments compute a new alignment that satisfies both
template<typename T>
T computeCompoundAlignment(const T alignment1, const T alignment2)
{
	ANKI_ASSERT(alignment1 && alignment2);

	// Compute greatest common divisor
	T greatestCommonDivisor = alignment1;
	T alignment2_ = alignment2;
	while(alignment2_ != 0)
	{
		const auto temp = alignment2_;
		alignment2_ = greatestCommonDivisor % alignment2_;
		greatestCommonDivisor = temp;
	}

	// Calculate the least common multiple (LCM) of the two alignments
	const auto lcmAlignment = alignment1 * (alignment2 / greatestCommonDivisor);

	return lcmAlignment;
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
/// using Ptr = double*;
/// RemovePointer<Ptr>::Type b = 666.0;
/// @endcode
/// The b is of type double
template<typename T>
struct RemovePointer;

template<typename T>
struct RemovePointer<T*>
{
	using Type = T;
};

template<typename T>
struct RemovePointer<const T*>
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
inline void unflatten3dArrayIndex(const T sizeA, const T sizeB, const T sizeC, const TI flatIdx, TOut& a, TOut& b, TOut& c)
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
	const U32 range = problemSize / threadCount;
	const U32 remain = problemSize % threadCount;

	start = threadId * range + min(remain, threadId);
	end = start + range + (threadId < remain);

	ANKI_ASSERT(start <= problemSize && end <= end);
	ANKI_ASSERT(!(threadId == threadCount - 1 && end != problemSize));
}

/// Just copy the memory of a float to a uint.
inline U64 floatBitsToUint(F64 f)
{
	U64 out;
	memcpy(&out, &f, sizeof(out));
	return out;
}

/// Just copy the memory of a float to a uint.
inline U32 floatBitsToUint(F32 f)
{
	U32 out;
	memcpy(&out, &f, sizeof(out));
	return out;
}

/// Call one of the costructors of an object.
template<typename T, typename... TArgs>
void callConstructor(T& p, TArgs&&... args)
{
	::new(&p) T(std::forward<TArgs>(args)...);
}

/// Call the destructor of an object.
template<typename T>
void callDestructor(T& p)
{
	static_assert(sizeof(T) > 0, "Incomplete type");
	p.~T();
}

#define ANKI_FRIEND_CALL_CONSTRUCTOR_AND_DESTRUCTOR \
	template<typename T, typename... TArgs> \
	friend void callConstructor(T& p, TArgs&&... args); \
	template<typename T> \
	friend void callDestructor(T& p);

/// Allocate a new object and call it's constructor
template<typename T, typename TMemPool, typename... TArgs>
[[nodiscard]] T* newInstance(TMemPool& pool, TArgs&&... args)
{
	T* ptr = static_cast<T*>(pool.allocate(sizeof(T), alignof(T)));
	if(ptr) [[likely]]
	{
		callConstructor(*ptr, std::forward<TArgs>(args)...);
	}

	return ptr;
}

/// Allocate a new array of objects and call their constructor
template<typename T, typename TMemPool>
[[nodiscard]] T* newArray(TMemPool& pool, PtrSize n)
{
	T* ptr = static_cast<T*>(pool.allocate(n * sizeof(T), alignof(T)));
	if(ptr) [[likely]]
	{
		for(PtrSize i = 0; i < n; i++)
		{
			callConstructor(ptr[i]);
		}
	}

	return ptr;
}

/// Allocate a new array of objects and call their constructor
template<typename T, typename TMemPool>
[[nodiscard]] T* newArray(TMemPool& pool, PtrSize n, const T& copy)
{
	T* ptr = static_cast<T*>(pool.allocate(n * sizeof(T), alignof(T)));
	if(ptr) [[likely]]
	{
		for(PtrSize i = 0; i < n; i++)
		{
			callConstructor(ptr[i], copy);
		}
	}

	return ptr;
}

/// Allocate a new array of objects and call their constructor.
/// @note The output is a parameter (instead of a return value) to work with template deduction.
template<typename T, typename TMemPool, typename TSize>
void newArray(TMemPool& pool, PtrSize n, WeakArray<T, TSize>& out)
{
	T* arr = newArray<T>(pool, n);
	ANKI_ASSERT(n < getMaxNumericLimit<TSize>());
	out.setArray(arr, TSize(n));
}

/// Call the destructor and deallocate an object.
template<typename T, typename TMemPool>
void deleteInstance(TMemPool& pool, T* ptr)
{
	if(ptr != nullptr) [[likely]]
	{
		callDestructor(*ptr);
		pool.free(ptr);
	}
}

/// Call the destructor and deallocate an array of objects.
template<typename T, typename TMemPool>
void deleteArray(TMemPool& pool, T* arr, PtrSize n)
{
	if(arr != nullptr) [[likely]]
	{
		for(PtrSize i = 0; i < n; i++)
		{
			callDestructor(arr[i]);
		}

		pool.free(arr);
	}
	else
	{
		ANKI_ASSERT(n == 0);
	}
}

/// Call the destructor and deallocate an array of objects.
template<typename T, typename TMemPool, typename TSize>
void deleteArray(TMemPool& pool, WeakArray<T, TSize>& arr)
{
	deleteArray(pool, arr.getBegin(), arr.getSize());
	arr.setArray(nullptr, 0);
}
/// @}

} // end namespace anki
