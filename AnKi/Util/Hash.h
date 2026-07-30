// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>

namespace anki {

// Computes a hash of a buffer. This function implements the MurmurHash2 algorithm by Austin Appleby
[[nodiscard]] ANKI_PURE U64 computeHash(const void* buffer, PtrSize bufferSize, U64 seed = 123);

// Computes a hash of a buffer. This function implements the MurmurHash2 algorithm by Austin Appleby
[[nodiscard]] ANKI_PURE U64 appendHash(const void* buffer, PtrSize bufferSize, U64 prevHash);

// See computeHash
template<typename T>
[[nodiscard]] ANKI_PURE U64 computeObjectHash(const T& obj, U64 seed = 123)
{
	return computeHash(&obj, sizeof(obj), seed);
}

// See appendHash
template<typename T>
[[nodiscard]] ANKI_PURE U64 appendObjectHash(const T& obj, U64 prevHash)
{
	return appendHash(&obj, sizeof(obj), prevHash);
}

constexpr U64 kFnv1aOffsetBasis = 0xCBF29CE484222325;
constexpr U64 kFnv1aPrime = 0x100000001B3;

// A constexpr hashing function implementing FNV-1a. Unlike computeHash() it takes an element count and not a byte size, and the two don't produce
// the same hash for the same bytes
template<typename T>
[[nodiscard]] constexpr U64 computeArrayHashConstexpr(const T* arr, PtrSize elementCount,
													  U64 seed = kFnv1aOffsetBasis) requires(std::is_integral<T>::value)
{
	U64 hash = seed;
	for(PtrSize i = 0; i < elementCount; ++i)
	{
		for(U32 b = 0; b < sizeof(T); ++b)
		{
			hash ^= U64(U8(U64(arr[i]) >> (b * 8)));
			hash *= kFnv1aPrime;
		}
	}

	return hash;
}

} // end namespace anki
