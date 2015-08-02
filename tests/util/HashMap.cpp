// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/HashMap.h"
#include "anki/util/HighRezTimer.h"
#include <unordered_map>

using namespace anki;

class Hasher
{
public:
	U64 operator()(int x)
	{
		return x;
	}
};

class Compare
{
public:
	Bool operator()(int a, int b)
	{
		return a == b;
	}
};

ANKI_TEST(Util, HashMap)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	int arr[] = {20, 15, 5, 1, 10, 0, 18, 6, 7, 11, 13, 3};
	U arrSize = sizeof(arr) / sizeof(arr[0]);

	// Simple
	{
		HashMap<int, Hasher, Compare> map;
		map.pushBack(alloc, 20);
		map.destroy(alloc);
	}

	// Add more and iterate
	{
		HashMap<int, Hasher, Compare> map;

		for(U i = 0; i < arrSize; ++i)
		{
			map.pushBack(alloc, arr[i]);
		}

		U count = 0;
		auto it = map.getBegin();
		for(; it != map.getEnd(); ++it)
		{
			++count;
		}
		ANKI_TEST_EXPECT_EQ(count, arrSize);

		map.destroy(alloc);
	}

	// Erase
	{
		HashMap<int, Hasher, Compare> map;

		for(U i = 0; i < arrSize; ++i)
		{
			map.pushBack(alloc, arr[i]);
		}

		for(U i = arrSize - 1; i != 0; --i)
		{
			HashMap<int, Hasher, Compare>::Iterator it = map.find(arr[i]);
			ANKI_TEST_EXPECT_NEQ(it, map.getEnd());

			map.erase(alloc, it);
			map.pushBack(alloc, arr[i]);
		}

		map.destroy(alloc);
	}
}
