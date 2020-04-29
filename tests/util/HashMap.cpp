// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/HashMap.h"
#include "anki/util/DynamicArray.h"
#include "anki/util/HighRezTimer.h"
#include <unordered_map>
#include <algorithm>

using namespace anki;

class Hasher
{
public:
	U64 operator()(int x)
	{
		return x;
	}
};

ANKI_TEST(Util, HashMap)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	int vals[] = {20, 15, 5, 1, 10, 0, 18, 6, 7, 11, 13, 3};
	U valsSize = sizeof(vals) / sizeof(vals[0]);

	// Simple
	{
		HashMap<int, int, Hasher> map;
		map.emplace(alloc, 20, 1);
		map.emplace(alloc, 21, 1);
		map.destroy(alloc);
	}

	// Add more and iterate
	{
		HashMap<int, int, Hasher> map;

		for(U i = 0; i < valsSize; ++i)
		{
			map.emplace(alloc, vals[i], vals[i] * 10);
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
		HashMap<int, int, Hasher> map;

		for(U i = 0; i < valsSize; ++i)
		{
			map.emplace(alloc, vals[i], vals[i] * 10);
		}

		for(U i = valsSize - 1; i != 0; --i)
		{
			HashMap<int, int, Hasher>::Iterator it = map.find(vals[i]);
			ANKI_TEST_EXPECT_NEQ(it, map.getEnd());

			map.erase(alloc, it);
			map.emplace(alloc, vals[i], vals[i] * 10);
		}

		map.destroy(alloc);
	}

	// Find
	{
		HashMap<int, int, Hasher> map;

		for(U i = 0; i < valsSize; ++i)
		{
			map.emplace(alloc, vals[i], vals[i] * 10);
		}

		for(U i = valsSize - 1; i != 0; --i)
		{
			HashMap<int, int, Hasher>::Iterator it = map.find(vals[i]);
			ANKI_TEST_EXPECT_NEQ(it, map.getEnd());
			ANKI_TEST_EXPECT_EQ(*it, vals[i] * 10);
		}

		map.destroy(alloc);
	}

	// Fuzzy test
	{
		const U MAX = 1000;
		HashMap<int, int, Hasher> akMap;
		std::vector<int> numbers;

		// Insert random
		for(U i = 0; i < MAX; ++i)
		{
			I32 num;
			while(1)
			{
				num = rand();
				if(std::find(numbers.begin(), numbers.end(), num) == numbers.end())
				{
					// Not found
					ANKI_TEST_EXPECT_EQ(akMap.find(num), akMap.getEnd());
					akMap.emplace(alloc, num, num);
					numbers.push_back(num);
					break;
				}
				else
				{
					// Found
					ANKI_TEST_EXPECT_NEQ(akMap.find(num), akMap.getEnd());
				}
			}
		}

		// Remove randomly
		U count = MAX;
		while(count--)
		{
			U idx = rand() % (count + 1);
			int num = numbers[idx];
			numbers.erase(numbers.begin() + idx);

			auto it = akMap.find(num);
			akMap.erase(alloc, it);
		}

		akMap.destroy(alloc);
	}

	// Bench it
	{
		using AkMap = HashMap<int, int, Hasher>;
		AkMap akMap(128, 32, 0.9f);
		using StlMap =
			std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, HeapAllocator<std::pair<const int, int>>>;
		StlMap stdMap(10, std::hash<int>(), std::equal_to<int>(), alloc);

		std::unordered_map<int, int> tmpMap;

		HighRezTimer timer;

		// Create a huge set
		const U32 COUNT = 1024 * 1024 * 10;
		DynamicArrayAuto<int> vals(alloc);
		vals.create(COUNT);

		for(U32 i = 0; i < COUNT; ++i)
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

		// Insertion
		{
			// Put the vals AnKi
			timer.start();
			for(U32 i = 0; i < COUNT; ++i)
			{
				akMap.emplace(alloc, vals[i], vals[i]);
			}
			timer.stop();
			Second akTime = timer.getElapsedTime();

			// Put the vals STL
			timer.start();
			for(U32 i = 0; i < COUNT; ++i)
			{
				stdMap[vals[i]] = vals[i];
			}
			timer.stop();
			Second stlTime = timer.getElapsedTime();

			ANKI_TEST_LOGI("Inserting bench: STL %f AnKi %f | %f%%", stlTime, akTime, stlTime / akTime * 100.0);
		}

		// Search
		{
			I64 count = 0; // To avoid compiler opts

			// Find values AnKi
			timer.start();
			for(U32 i = 0; i < COUNT; ++i)
			{
				auto it = akMap.find(vals[i]);
				count += *it;
			}
			timer.stop();
			Second akTime = timer.getElapsedTime();

			// Find values STL
			timer.start();
			for(U32 i = 0; i < COUNT; ++i)
			{
				count += stdMap[vals[i]];
			}
			timer.stop();
			Second stlTime = timer.getElapsedTime();

			ANKI_TEST_LOGI("Find bench: STL %f AnKi %f | %f%% (%ld)", stlTime, akTime, stlTime / akTime * 100.0, count);
		}

		// Delete
		{
			// Remove in random order
			std::random_shuffle(vals.begin(), vals.end());

			// Random delete AnKi
			Second akTime = 0.0;
			for(U32 i = 0; i < vals.getSize(); ++i)
			{
				auto it = akMap.find(vals[i]);

				timer.start();
				akMap.erase(alloc, it);
				timer.stop();
				akTime += timer.getElapsedTime();
			}

			// Random delete STL
			Second stlTime = 0.0;
			for(U32 i = 0; i < vals.getSize(); ++i)
			{
				auto it = stdMap.find(vals[i]);

				timer.start();
				stdMap.erase(it);
				timer.stop();
				stlTime += timer.getElapsedTime();
			}

			ANKI_TEST_LOGI("Deleting bench: STL %f AnKi %f | %f%%", stlTime, akTime, stlTime / akTime * 100.0);
		}

		akMap.destroy(alloc);
	}
}
