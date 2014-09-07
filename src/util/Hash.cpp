// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Hash.h"

namespace anki {

//==============================================================================
U64 computeHash(const void* buffer, U32 bufferSize, U64 seed)
{
	const U64 m = 0xc6a4a7935bd1e995;
	const U64 r = 47;

	U64 h = seed ^ (bufferSize * m);

	const U64* data = reinterpret_cast<const U64*>(buffer);
	const U64* end = data + (bufferSize / sizeof(U64));

	while(data != end)
	{
		U64 k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m;
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
		h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

} // end namespace anki

