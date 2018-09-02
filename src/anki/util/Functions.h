// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

/// Check if a number os a power of 2
template<typename Int>
inline Bool isPowerOfTwo(Int x)
{
	return !(x == 0) && !(x & (x - 1));
}

/// Get the next power of two number. For example if x is 130 this will return 256.
template<typename Int>
inline Int nextPowerOfTwo(Int x)
{
	return pow(2, ceil(log(x) / log(2)));
}

/// Get the aligned number rounded up.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline Type getAlignedRoundUp(PtrSize alignment, Type value)
{
	ANKI_ASSERT(alignment > 0);
	PtrSize v = PtrSize(value);
	v = ((v + alignment - 1) / alignment) * alignment;
	return Type(v);
}

/// Align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline void alignRoundUp(PtrSize alignment, Type& value)
{
	value = getAlignedRoundUp(alignment, value);
}

/// Get the aligned number rounded down.
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline Type getAlignedRoundDown(PtrSize alignment, Type value)
{
	ANKI_ASSERT(alignment > 0);
	PtrSize v = PtrSize(value);
	v = (v / alignment) * alignment;
	return Type(v);
}

/// Align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline void alignRoundDown(PtrSize alignment, Type& value)
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

/// Check if types are the same.
template<class T, class Y>
struct TypesAreTheSame
{
	enum
	{
		m_value = 0
	};
};

template<class T>
struct TypesAreTheSame<T, T>
{
	enum
	{
		m_value = 1
	};
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
inline void unflatten3dArrayIndex(const U sizeA, const U sizeB, const U sizeC, const U flatIdx, U& a, U& b, U& c)
{
	ANKI_ASSERT(flatIdx < (sizeA * sizeB * sizeC));
	a = (flatIdx / (sizeB * sizeC)) % sizeA;
	b = (flatIdx / sizeC) % sizeB;
	c = flatIdx % sizeC;
}

/// Given a threaded problem split it into smaller ones.
inline void splitThreadedProblem(
	PtrSize threadId, PtrSize threadCount, PtrSize problemSize, PtrSize& start, PtrSize& end)
{
	ANKI_ASSERT(threadCount > 0 && threadId < threadCount);
	const PtrSize div = problemSize / threadCount;
	start = threadId * div;
	end = (threadId == threadCount - 1) ? problemSize : (threadId + 1u) * div;
	ANKI_ASSERT(!(threadId == threadCount - 1 && end != problemSize));
}

/// Equivelent to static_cast.
template<typename T, typename Y>
inline T scast(Y from)
{
	ANKI_ASSERT(from);
	return static_cast<T>(from);
}

/// Equivelent to reinterpret_cast.
template<typename T, typename Y>
inline T rcast(Y from)
{
	ANKI_ASSERT(from);
	return reinterpret_cast<T>(from);
}

#define _ANKI_CONCATENATE(a, b) a##b

/// Concatenate 2 preprocessor tokens.
#define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#define _ANKI_STRINGIZE(a) #a

/// Make a preprocessor token a string.
#define ANKI_STRINGIZE(a) _ANKI_STRINGIZE(a)
/// @}

} // end namespace anki
