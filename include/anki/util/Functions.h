// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
	while(((x % 2) == 0) && x > 1)
	{
		x /= 2;
	}
	return (x == 1);
}

/// Get the next power of two number. For example if x is 130 this will return
/// 256
template<typename Int>
inline Int nextPowerOfTwo(Int x)
{
	return pow(2, ceil(log(x) / log(2)));
}

/// Get align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline Type getAlignedRoundUp(PtrSize alignment, Type value)
{
	ANKI_ASSERT(isPowerOfTwo(alignment));
	PtrSize v = (PtrSize)value;
	v = (v + alignment - 1) & ~(alignment - 1);
	return (Type)v;
}

/// Align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline void alignRoundUp(PtrSize alignment, Type& value)
{
	value = getAlignedRoundUp(alignment, value);
}

/// Get align number
/// @param alignment The bytes of alignment
/// @param value The value to align
template<typename Type>
inline Type getAlignedRoundDown(PtrSize alignment, Type value)
{
	ANKI_ASSERT(isPowerOfTwo(alignment));
	PtrSize v = (PtrSize)value;
	v &= ~(alignment - 1);
	return (Type)v;
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
	ANKI_ASSERT(ptr);
	uintptr_t i = reinterpret_cast<uintptr_t>(ptr);
	PtrSize size = i;
	return size;
}

/// Convert a number to a pointer.
template<typename TPtr>
inline TPtr numberToPtr(PtrSize num)
{
	ANKI_ASSERT(num);
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

/// Perform a static cast of a pointer
template<typename To, typename From>
To staticCastPtr(From from)
{
#if ANKI_DEBUG == 1
	To ptr = dynamic_cast<To>(from);
	ANKI_ASSERT(ptr != nullptr);
	return ptr;
#else
	return static_cast<To>(from);
#endif
}

/// Count bits
inline U32 countBits(U32 number)
{
#if defined(__GNUC__)
	return __builtin_popcount(number);
#else
#error "Unimplemented"
#endif
}

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
/// @}

} // end namespace anki
