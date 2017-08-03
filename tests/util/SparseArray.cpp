// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/SparseArray.h>
#include <anki/util/HighRezTimer.h>
#include <unordered_map>
#include <ctime>

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

			auto it = arr.getAt(num);
			ANKI_TEST_EXPECT_NEQ(it, arr.getEnd());
			ANKI_TEST_EXPECT_EQ(*it, num);
			arr.erase(alloc, it);
		}
	}

	// Fuzzy test #2: Do random insertions and removals
	{
		const U MAX = 10000;
		SparseArray<int, U32, 64, 4> arr;
		using StlMap =
			std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, HeapAllocator<std::pair<int, int>>>;
		StlMap map(10, std::hash<int>(), std::equal_to<int>(), alloc);

		for(U i = 0; i < MAX; ++i)
		{
			const Bool insert = (rand() & 1) || arr.getSize() == 0;

			if(insert)
			{
				const I idx = rand();

				if(map.find(idx) != map.end())
				{
					continue;
				}

				arr.setAt(alloc, idx, idx);
				map[idx] = idx;

				arr.validate();
			}
			else
			{
				const U idx = U(rand()) % map.size();
				auto it = std::next(std::begin(map), idx);

				auto it2 = arr.getAt(it->second);
				ANKI_TEST_EXPECT_NEQ(it2, arr.getEnd());

				map.erase(it);
				arr.erase(alloc, it2);

				arr.validate();
			}

			// Iterate and check
			{
				StlMap bMap = map;

				auto it = arr.getBegin();
				while(it != arr.getEnd())
				{
					I val = *it;

					auto it2 = bMap.find(val);
					ANKI_TEST_EXPECT_NEQ(it2, bMap.end());

					bMap.erase(it2);

					++it;
				}
			}
		}

		arr.destroy(alloc);
	}
}

static PtrSize akAllocSize = 0;
static ANKI_DONT_INLINE void* allocAlignedAk(void* userData, void* ptr, PtrSize size, PtrSize alignment)
{
	if(ptr == nullptr)
	{
		akAllocSize += size;
	}

	return allocAligned(userData, ptr, size, alignment);
}

static PtrSize stlAllocSize = 0;
static ANKI_DONT_INLINE void* allocAlignedStl(void* userData, void* ptr, PtrSize size, PtrSize alignment)
{
	if(ptr == nullptr)
	{
		stlAllocSize += size;
	}

	return allocAligned(userData, ptr, size, alignment);
}

ANKI_TEST(Util, SparseArrayBench)
{
	HeapAllocator<U8> allocAk(allocAlignedAk, nullptr);
	HeapAllocator<U8> allocStl(allocAlignedStl, nullptr);
	HeapAllocator<U8> allocTml(allocAligned, nullptr);

	using StlMap = std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, HeapAllocator<std::pair<int, int>>>;
	StlMap stdMap(10, std::hash<int>(), std::equal_to<int>(), allocStl);

	using AkMap = SparseArray<int, U32, 1024, 16>;
	AkMap akMap;

	HighRezTimer timer;

	const U COUNT = 1024 * 1024 * 5;

	// Create a huge set
	DynamicArrayAuto<int> vals(allocTml);

	{
		std::unordered_map<int, int> tmpMap;
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
	}

	// Insertion
	{
		// AnkI
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			akMap.setAt(allocAk, vals[i], vals[i]);
		}
		timer.stop();
		HighRezTimer::Scalar akTime = timer.getElapsedTime();

		// STL
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			stdMap[vals[i]] = vals[i];
		}
		timer.stop();
		HighRezTimer::Scalar stlTime = timer.getElapsedTime();

		ANKI_TEST_LOGI("Inserting bench: STL %f AnKi %f | %f%%", stlTime, akTime, stlTime / akTime * 100.0);
	}

	// Search
	{
		int count = 0;

		// Find values AnKi
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			auto it = akMap.getAt(vals[i]);
			count += *it;
		}
		timer.stop();
		HighRezTimer::Scalar akTime = timer.getElapsedTime();

		// Find values STL
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			count += stdMap[vals[i]];
		}
		timer.stop();
		HighRezTimer::Scalar stlTime = timer.getElapsedTime();

		ANKI_TEST_LOGI("Find bench: STL %f AnKi %f | %f%%", stlTime, akTime, stlTime / akTime * 100.0);
	}

	// Mem usage
	const PtrSize stlMemUsage = stlAllocSize + sizeof(stdMap);
	const PtrSize akMemUsage = akAllocSize + sizeof(akMap);
	ANKI_TEST_LOGI(
		"Mem usage: STL %llu AnKi %llu | %f%%", stlMemUsage, akMemUsage, F64(stlMemUsage) / akMemUsage * 100.0);

	// Deletes
	{
		const U DEL_COUNT = COUNT / 2;
		DynamicArrayAuto<AkMap::Iterator> delValsAk(allocTml);
		delValsAk.create(DEL_COUNT);

		DynamicArrayAuto<StlMap::iterator> delValsStl(allocTml);
		delValsStl.create(DEL_COUNT);

		std::unordered_map<int, int> tmpMap;
		for(U i = 0; i < DEL_COUNT; ++i)
		{
			// Put unique keys
			int v;
			do
			{
				v = vals[rand() % COUNT];
			} while(tmpMap.find(v) != tmpMap.end());
			tmpMap[v] = 1;

			delValsAk[i] = akMap.getAt(v);
			delValsStl[i] = stdMap.find(v);
		}

		// Random delete AnKi
		timer.start();
		for(U i = 0; i < DEL_COUNT; ++i)
		{
			akMap.erase(allocAk, delValsAk[i]);
		}
		timer.stop();
		HighRezTimer::Scalar akTime = timer.getElapsedTime();

		// Random delete STL
		timer.start();
		for(U i = 0; i < DEL_COUNT; ++i)
		{
			stdMap.erase(delValsStl[i]);
		}
		timer.stop();
		HighRezTimer::Scalar stlTime = timer.getElapsedTime();

		ANKI_TEST_LOGI("Deleting bench: STL %f AnKi %f | %f%%\n", stlTime, akTime, stlTime / akTime * 100.0);
	}

	akMap.destroy(allocAk);
}
