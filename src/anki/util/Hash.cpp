// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Hash.h>
#include <anki/util/Assert.h>

namespace anki
{

const U64 HASH_M = 0xc6a4a7935bd1e995;
const U64 HASH_R = 47;

U64 appendHash(const void* buffer, U32 bufferSize, U64 h)
{
	const U64* data = static_cast<const U64*>(buffer);
	const U64* end = data + (bufferSize / sizeof(U64));

	while(data != end)
	{
		U64 k = *data++;

		k *= HASH_M;
		k ^= k >> HASH_R;
		k *= HASH_M;

		h ^= k;
		h *= HASH_M;
	}

	const U8* data2 = reinterpret_cast<const U8*>(data);

	switch(bufferSize & (sizeof(U64) - 1))
	{
	case 7:
		h ^= static_cast<U64>(data2[6]) << 48;
	case 6:
		h ^= static_cast<U64>(data2[5]) << 40;
	case 5:
		h ^= static_cast<U64>(data2[4]) << 32;
	case 4:
		h ^= static_cast<U64>(data2[3]) << 24;
	case 3:
		h ^= static_cast<U64>(data2[2]) << 16;
	case 2:
		h ^= static_cast<U64>(data2[1]) << 8;
	case 1:
		h ^= static_cast<U64>(data2[0]);
		h *= HASH_M;
	};

	h ^= h >> HASH_R;
	h *= HASH_M;
	h ^= h >> HASH_R;

	ANKI_ASSERT(h != 0);
	return h;
}

U64 computeHash(const void* buffer, U32 bufferSize, U64 seed)
{
	U64 h = seed ^ (bufferSize * HASH_M);
	return appendHash(buffer, bufferSize, h);
}

} // end namespace anki
