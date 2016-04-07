// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/HashMap.h"
#include "anki/util/DynamicArray.h"
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
	int vals[] = {20, 15, 5, 1, 10, 0, 18, 6, 7, 11, 13, 3};
	U valsSize = sizeof(vals) / sizeof(vals[0]);

	// Simple
	{
		HashMap<int, int, Hasher, Compare> map;
		map.pushBack(alloc, 20, 1);
		map.pushBack(alloc, 21, 1);
		map.destroy(alloc);
	}

	// Add more and iterate
	{
		HashMap<int, int, Hasher, Compare> map;

		for(U i = 0; i < valsSize; ++i)
		{
			map.pushBack(alloc, vals[i], vals[i] * 10);
		}

		U count = 0;
		auto it = map.getBegin();
		for(; it != map.getEnd(); ++it)
		{
			++count;
		}
		ANKI_TEST_EXPECT_EQ(count, valsSize);

		map.destroy(alloc);
	}

	// Erase
	{
		HashMap<int, int, Hasher, Compare> map;

		for(U i = 0; i < valsSize; ++i)
		{
			map.pushBack(alloc, vals[i], vals[i] * 10);
		}

		for(U i = valsSize - 1; i != 0; --i)
		{
			HashMap<int, int, Hasher, Compare>::Iterator it = map.find(vals[i]);
			ANKI_TEST_EXPECT_NEQ(it, map.getEnd());

			map.erase(alloc, it);
			map.pushBack(alloc, vals[i], vals[i] * 10);
		}

		map.destroy(alloc);
	}

	// Find
	{
		HashMap<int, int, Hasher, Compare> map;

		for(U i = 0; i < valsSize; ++i)
		{
			map.pushBack(alloc, vals[i], vals[i] * 10);
		}

		for(U i = valsSize - 1; i != 0; --i)
		{
			HashMap<int, int, Hasher, Compare>::Iterator it = map.find(vals[i]);
			ANKI_TEST_EXPECT_NEQ(it, map.getEnd());
			ANKI_TEST_EXPECT_EQ(*it, vals[i] * 10);
		}

		map.destroy(alloc);
	}

	// Bench it
	{
		HashMap<int, int, Hasher, Compare> akMap;
		std::unordered_map<int,
			int,
			std::hash<int>,
			std::equal_to<int>,
			HeapAllocator<std::pair<int, int>>>
			stdMap(10, std::hash<int>(), std::equal_to<int>(), alloc);

		std::unordered_map<int, int> tmpMap;

		HighRezTimer timer;
		I64 count = 0;

		// Create a huge set
		const U COUNT = 1024 * 100;
		DynamicArrayAuto<int> vals(alloc);
		vals.create(COUNT);

		for(U i = 0; i < COUNT; ++i)
		{
			// Put unique keys
			int v;
			do
			{
				v = rand();
			} while(tmpMap.find(v) != tmpMap.end());
			tmpMap[v] = 1;

			vals[i] = v;
		}

		// Put the vals AnKi
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			akMap.pushBack(alloc, vals[i], vals[i]);
		}
		timer.stop();
		HighRezTimer::Scalar akTime = timer.getElapsedTime();

		// Put the vals STL
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			stdMap[vals[i]] = vals[i];
		}
		timer.stop();
		HighRezTimer::Scalar stlTime = timer.getElapsedTime();

		printf("Inserting bench: STL %f AnKi %f | %f%%\n",
			stlTime,
			akTime,
			stlTime / akTime * 100.0);

		// Find values AnKi
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			auto it = akMap.find(vals[i]);
			count += *it;
		}
		timer.stop();
		akTime = timer.getElapsedTime();

		// Find values STL
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			count += stdMap[vals[i]];
		}
		timer.stop();
		stlTime = timer.getElapsedTime();

		printf("Find bench: STL %f AnKi %f | %f%%\n",
			stlTime,
			akTime,
			stlTime / akTime * 100.0);

		akMap.destroy(alloc);
	}
}

class Hashable : public IntrusiveHashMapEnabled<Hashable>
{
public:
	Hashable(int x)
		: m_x(x)
	{
	}

	int m_x;
};

ANKI_TEST(Util, IntrusiveHashMap)
{
	Hashable a(1);
	Hashable b(2);
	Hashable c(10);

	IntrusiveHashMap<int, Hashable, Hasher, Compare> map;

	// Add vals
	map.pushBack(1, &a);
	map.pushBack(2, &b);
	map.pushBack(10, &c);

	// Find
	ANKI_TEST_EXPECT_EQ(map.find(1)->m_x, 1);
	ANKI_TEST_EXPECT_EQ(map.find(10)->m_x, 10);

	// Erase
	map.erase(map.find(10));
	ANKI_TEST_EXPECT_EQ(map.find(10), map.getEnd());

	// Put back
	map.pushBack(10, &c);
	ANKI_TEST_EXPECT_NEQ(map.find(10), map.getEnd());
}
