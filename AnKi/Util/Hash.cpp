// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Hash.h>
#include <AnKi/Util/Assert.h>

namespace anki {

constexpr U64 kHashM = 0xc6a4a7935bd1e995;
constexpr U64 kHashR = 47;

U64 appendHash(const void* buffer, PtrSize bufferSize, U64 h)
{
	const U64* data = static_cast<const U64*>(buffer);
	const U64* const end = data + (bufferSize / sizeof(U64));

	while(data != end)
	{
		U64 k = *data++;

		k *= kHashM;
		k ^= k >> kHashR;
		k *= kHashM;

		h ^= k;
		h *= kHashM;
	}

	const U8* data2 = reinterpret_cast<const U8*>(data);

	switch(bufferSize & (sizeof(U64) - 1))
	{
	case 7:
		h ^= U64(data2[6]) << 48;
	case 6:
		h ^= U64(data2[5]) << 40;
	case 5:
		h ^= U64(data2[4]) << 32;
	case 4:
		h ^= U64(data2[3]) << 24;
	case 3:
		h ^= U64(data2[2]) << 16;
	case 2:
		h ^= U64(data2[1]) << 8;
	case 1:
		h ^= U64(data2[0]);
		h *= kHashM;
	};

	h ^= h >> kHashR;
	h *= kHashM;
	h ^= h >> kHashR;

	ANKI_ASSERT(h != 0);
	return h;
}

U64 computeHash(const void* buffer, PtrSize bufferSize, U64 seed)
{
	const U64 h = seed ^ (bufferSize * kHashM);
	return appendHash(buffer, bufferSize, h);
}

} // end namespace anki
