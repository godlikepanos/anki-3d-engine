// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

// ANKI_ENABLE_METHOD & ANKI_ENABLE_ARG trickery copied from Tick library
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

template<typename T, int N>
struct DummyType
{
};

#if defined(_MSC_VER)
#	define ANKI_REQUIRES_BOOL(line, ...) RequiresUnwrap<decltype(RequiresBool<(__VA_ARGS__)>{}), line>::VALUE

#	define ANKI_ENABLE_INTERNAL(line, ...) \
		typename PrivateEnum<line>::Type ANKI_CONCATENATE( \
			privateEnum, line) = PrivateEnum<line>::Type::NA, \
						 bool ANKI_CONCATENATE(privateBool, line) = true, \
						 typename = typename std::enable_if_t<( \
							 ANKI_CONCATENATE(privateBool, line) && ANKI_REQUIRES_BOOL(line, __VA_ARGS__))>
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

/// Pick a random number from min to max
extern I32 randRange(I32 min, I32 max);

/// Pick a random number from min to max
extern U32 randRange(U32 min, U32 max);

/// Pick a random number from min to max
extern F32 randRange(F32 min, F32 max);

/// Pick a random number from min to max
extern F64 randRange(F64 min, F64 max);

extern F32 randFloat(F32 max);

/// Get min of two values.
template<typename T>
inline T min(T a, T b)
{
	return (a < b) ? a : b;
}

/// Get max of two values.
template<typename T>
inline T max(T a, T b)
{
	return (a > b) ? a : b;
}

template<typename T>
inline T clamp(T v, T minv, T maxv)
{
	ANKI_ASSERT(minv < maxv);
	return min<T>(max<T>(minv, v), maxv);
}

/// Check if a number is a power of 2
template<typename Int, ANKI_ENABLE(std::is_integral<Int>::value)>
inline Bool isPowerOfTwo(Int x)
{
	return !(x == 0) && !(x & (x - 1));
}

/// Get the next power of two number. For example if x is 130 this will return 256.
template<typename Int, ANKI_ENABLE(std::is_integral<Int>::value)>
inline Int nextPowerOfTwo(Int x)
{
	return pow(2, ceil(log(x) / log(2)));
}

/// Get the aligned number rounded up.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename TInt, ANKI_ENABLE(std::is_integral<TInt>::value)>
inline TInt getAlignedRoundUp(PtrSize alignment, TInt value)
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
inline TFloat getAlignedRoundUp(TFloat alignment, TFloat value)
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
inline TInt getAlignedRoundDown(PtrSize alignment, TInt value)
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
inline TFloat getAlignedRoundDown(TFloat alignment, TFloat value)
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
inline Bool isAligned(PtrSize alignment, Type value)
{
	return ((PtrSize)value % alignment) == 0;
}

template<typename T>
inline void swapValues(T& a, T& b)
{
	T tmp = b;
	b = a;
	a = tmp;
}

/// Convert any pointer to a number.
template<typename TPtr>
inline PtrSize ptrToNumber(TPtr ptr)
{
	uintptr_t i = reinterpret_cast<uintptr_t>(ptr);
	PtrSize size = i;
	return size;
}

/// Convert a number to a pointer.
template<typename TPtr>
inline TPtr numberToPtr(PtrSize num)
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
	typedef T Type;
};

template<typename T>
void memorySet(T* dest, T value, PtrSize count)
{
	ANKI_ASSERT(dest);
	while(count--)
	{
		dest[count] = value;
	}
}

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
inline void unflatten3dArrayIndex(
	const T sizeA, const T sizeB, const T sizeC, const TI flatIdx, TOut& a, TOut& b, TOut& c)
{
	ANKI_ASSERT(flatIdx < (sizeA * sizeB * sizeC));
	a = (flatIdx / (sizeB * sizeC)) % sizeA;
	b = (flatIdx / sizeC) % sizeB;
	c = flatIdx % sizeC;
}

/// Given a threaded problem split it into smaller ones. This function accepts the number of threads and the size of
/// the threaded problem. Then given a thread index it chooses a range that the thread can operate into. That range is
/// supposed to be as evenly split as possible across threads.
template<typename TThreadId, typename TThreadCount, typename TProblemSize, typename TResult>
inline void splitThreadedProblem(
	TThreadId threadId, TThreadCount threadCount, TProblemSize problemSize, TResult& start, TResult& end)
{
	ANKI_ASSERT(threadCount > 0 && threadId < threadCount);
	const auto div = problemSize / threadCount;
	start = threadId * div;
	end = (threadId == threadCount - 1) ? problemSize : (threadId + 1u) * div;
	ANKI_ASSERT(!(threadId == threadCount - 1 && end != problemSize));
}
/// @}

} // end namespace anki