// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// Contains misc functions

#ifndef ANKI_UTIL_FUNCTIONS_H
#define ANKI_UTIL_FUNCTIONS_H

#include "anki/util/StdTypes.h"
#include "anki/util/String.h"
#include <cmath>

namespace anki {

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

/// Get the size in bytes of a vector
template<typename Vec>
inline PtrSize getVectorSizeInBytes(const Vec& v)
{
	return v.size() * sizeof(typename Vec::value_type);
}

/// Delete a pointer properly 
template<typename T>
inline void propperDelete(T*& x)
{
	typedef char TypeMustBeComplete[sizeof(T) ? 1 : -1];
  	(void) sizeof(TypeMustBeComplete);
	delete x;
	x = nullptr;
}

/// Delete a pointer properly
template<typename T>
inline void propperDeleteArray(T*& x)
{
	typedef char TypeMustBeComplete[sizeof(T) ? 1 : -1];
	(void) sizeof(TypeMustBeComplete);
	delete[] x;
	x = nullptr;
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
#	error "Unimplemented"
#endif
}

/// Get the underlying type of a strongly typed enum
template<typename TEnum>
constexpr typename std::underlying_type<TEnum>::type enumValue(TEnum val)
{
	return static_cast<typename std::underlying_type<TEnum>::type>(val);
}

/// @}

} // end namespace anki

#endif
