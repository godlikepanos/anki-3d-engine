// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include <anki/util/SparseArray.h>

ANKI_TEST(Util, SparseArray)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Set same key
	{
		SparseArray<PtrSize> arr;

		arr.setAt(alloc, 1000, 123);
		auto it = arr.setAt(alloc, 1000, 124);
		ANKI_TEST_EXPECT_EQ(*arr.getAt(1000), 124);
		arr.erase(alloc, it);
	}

	// Check destroy
	{
		SparseArray<PtrSize> arr;

		arr.setAt(alloc, 10000, 123);
		arr.setAt(alloc, 20000, 124);
		arr.setAt(alloc, 30000, 125);

		ANKI_TEST_EXPECT_EQ(*arr.getAt(10000), 123);
		ANKI_TEST_EXPECT_EQ(*arr.getAt(20000), 124);
		ANKI_TEST_EXPECT_EQ(*arr.getAt(30000), 125);

		arr.destroy(alloc);
	}

	// Do complex insertions
	{
		SparseArray<PtrSize, U32, 32, 3> arr;

		arr.setAt(alloc, 32, 1);
		// Linear probing
		arr.setAt(alloc, 32 * 2, 2);
		arr.setAt(alloc, 32 * 3, 3);
		// Append to a tree
		arr.setAt(alloc, 32 * 4, 4);
		// Linear probing
		arr.setAt(alloc, 32 + 1, 5);
		// Evict node
		arr.setAt(alloc, 32 * 2 + 1, 5);

		ANKI_TEST_EXPECT_EQ(arr.getSize(), 6);

		arr.destroy(alloc);
	}

	// Fuzzy test
	{
		const U MAX = 1000;
		SparseArray<int, U32, 32, 3> arr;
		std::vector<int> numbers;

		srand(time(nullptr));

		// Insert random
		for(U i = 0; i < MAX; ++i)
		{
			U num;
			while(1)
			{
				num = rand();
				if(std::find(numbers.begin(), numbers.end(), int(num)) == numbers.end())
				{
					// Not found
					ANKI_TEST_EXPECT_EQ(arr.getAt(num), arr.getEnd());
					arr.setAt(alloc, num, num);
					numbers.push_back(num);
					break;
				}
				else
				{
					// Found
					ANKI_TEST_EXPECT_NEQ(arr.getAt(num), arr.getEnd());
				}
			}
		}

		ANKI_TEST_EXPECT_EQ(arr.getSize(), MAX);

		// Remove randomly
		U count = MAX;
		while(count--)
		{
			U idx = rand() % (count + 1);
			int num = numbers[idx];
			numbers.erase(numbers.begin() + idx);

			auto it = arr.getAt(idx);
			ANKI_TEST_EXPECT_NEQ(it, arr.getEnd());
			ANKI_TEST_EXPECT_EQ(*it, num);
			arr.erase(alloc, it);
		}
	}
}
