// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/SparseArray.h>
#include <anki/util/HighRezTimer.h>
#include <unordered_map>
#include <ctime>
#include <algorithm>
#include <malloc.h>

namespace anki
{
namespace
{

static I64 constructor0Count = 0;
static I64 constructor1Count = 0;
static I64 constructor2Count = 0;
static I64 constructor3Count = 0;
static I64 destructorCount = 0;
static I64 copyCount = 0;
static I64 moveCount = 0;

class SAFoo
{
public:
	int m_x;

	SAFoo()
		: m_x(0)
	{
		++constructor0Count;
	}

	SAFoo(int x)
		: m_x(x)
	{
		++constructor1Count;
	}

	SAFoo(const SAFoo& b)
		: m_x(b.m_x)
	{
		++constructor2Count;
	}

	SAFoo(SAFoo&& b)
		: m_x(b.m_x)
	{
		b.m_x = 0;
		++constructor3Count;
	}

	~SAFoo()
	{
		++destructorCount;
	}

	SAFoo& operator=(const SAFoo& b)
	{
		m_x = b.m_x;
		++copyCount;
		return *this;
	}

	SAFoo& operator=(SAFoo&& b)
	{
		m_x = b.m_x;
		b.m_x = 0;
		++moveCount;
		return *this;
	}

	static void checkCalls()
	{
		ANKI_TEST_EXPECT_EQ(constructor0Count, 0); // default
		ANKI_TEST_EXPECT_GT(constructor1Count, 0);
		ANKI_TEST_EXPECT_EQ(constructor2Count, 0); // copy
		ANKI_TEST_EXPECT_GT(constructor3Count, 0); // move
		ANKI_TEST_EXPECT_EQ(destructorCount, constructor1Count + constructor3Count);

		ANKI_TEST_EXPECT_EQ(copyCount, 0); // copy
		ANKI_TEST_EXPECT_GEQ(moveCount, 0); // move

		constructor0Count = constructor1Count = constructor2Count = constructor3Count = destructorCount = copyCount =
			moveCount = 0;
	}
};
} // namespace
} // namespace anki

ANKI_TEST(Util, SparseArray)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Set same key
	{
		SparseArray<PtrSize> arr;

		arr.emplace(alloc, 1000, 123);
		arr.emplace(alloc, 1000, 124);
		auto it = arr.find(1000);
		ANKI_TEST_EXPECT_EQ(*it, 124);
		arr.erase(alloc, it);
	}

	// Check destroy and grow
	{
		SparseArray<SAFoo> arr(64, 2);

		arr.emplace(alloc, 64 * 1, 123);
		arr.emplace(alloc, 64 * 2, 124);
		arr.emplace(alloc, 64 * 3, 125);

		ANKI_TEST_EXPECT_EQ(arr.find(64 * 1)->m_x, 123);
		ANKI_TEST_EXPECT_EQ(arr.find(64 * 2)->m_x, 124);
		ANKI_TEST_EXPECT_EQ(arr.find(64 * 3)->m_x, 125);

		arr.destroy(alloc);
		SAFoo::checkCalls();
	}

	// Do complex insertions
	{
		SparseArray<SAFoo, U32> arr(64, 3);

		arr.emplace(alloc, 64 * 0 - 1, 1);
		// Linear probing to 0
		arr.emplace(alloc, 64 * 1 - 1, 2);
		// Linear probing to 1
		arr.emplace(alloc, 64 * 2 - 1, 3);
		// Linear probing to 2
		arr.emplace(alloc, 1, 3);
		// Swap
		arr.emplace(alloc, 64 * 1, 3);

		ANKI_TEST_EXPECT_EQ(arr.getSize(), 5);

		arr.destroy(alloc);
		SAFoo::checkCalls();
	}

	// Fuzzy test
	{
		const U MAX = 10000;
		SparseArray<SAFoo, U32> arr;
		std::vector<int> numbers;

		srand(U32(time(nullptr)));

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
					ANKI_TEST_EXPECT_EQ(arr.find(num), arr.getEnd());
					arr.emplace(alloc, num, num);
					ANKI_TEST_EXPECT_EQ(arr.getSize(), i + 1);

					numbers.push_back(num);
					break;
				}
				else
				{
					// Found
					ANKI_TEST_EXPECT_NEQ(arr.find(num), arr.getEnd());
				}
			}

			arr.validate();
		}

		ANKI_TEST_EXPECT_EQ(arr.getSize(), MAX);

		// Remove randomly
		U count = MAX;
		while(count--)
		{
			U idx = rand() % (count + 1);
			int num = numbers[idx];
			numbers.erase(numbers.begin() + idx);

			auto it = arr.find(num);
			ANKI_TEST_EXPECT_NEQ(it, arr.getEnd());
			ANKI_TEST_EXPECT_EQ(it->m_x, num);
			arr.erase(alloc, it);

			arr.validate();
		}
	}

	// Fuzzy test #2: Do random insertions and removals
	{
		const U MAX = 10000;
		SparseArray<SAFoo, U64> arr;
		using StlMap =
			std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, HeapAllocator<std::pair<const int, int>>>;
		StlMap map(10, std::hash<int>(), std::equal_to<int>(), alloc);

		for(U i = 0; i < MAX; ++i)
		{
			const Bool insert = (rand() & 1) || arr.getSize() == 0;

			if(insert)
			{
				const I32 idx = rand();

				if(map.find(idx) != map.end())
				{
					continue;
				}

				arr.emplace(alloc, idx, idx + 1);
				map[idx] = idx + 1;

				arr.validate();
			}
			else
			{
				const U idx = U(rand()) % map.size();
				auto it = std::next(std::begin(map), idx);

				int key = it->first;

				auto it2 = arr.find(key);
				ANKI_TEST_EXPECT_NEQ(it2, arr.getEnd());
				ANKI_TEST_EXPECT_EQ(it->second, it2->m_x);

				map.erase(it);
				arr.erase(alloc, it2);

				ANKI_TEST_EXPECT_EQ(arr.find(key), arr.getEnd());

				arr.validate();
			}

			// Iterate and check
			{
				StlMap bMap = map;

				auto it = arr.getBegin();
				while(it != arr.getEnd())
				{
					I32 key = it->m_x - 1;

					auto it2 = bMap.find(key);
					ANKI_TEST_EXPECT_NEQ(it2, bMap.end());

					bMap.erase(it2);

					++it;
				}
			}
		}

		arr.destroy(alloc);

		// Check what the SparseArray have called
		SAFoo::checkCalls();
	}
}

static I64 akAllocSize = 0;
static I64 akMaxAllocSize = 0;
static ANKI_DONT_INLINE void* allocAlignedAk(void* userData, void* ptr, PtrSize size, PtrSize alignment)
{
	if(ptr == nullptr)
	{
#if ANKI_OS_LINUX
		akAllocSize += size;
		akMaxAllocSize = max(akMaxAllocSize, akAllocSize);
#endif
		return malloc(size);
	}
	else
	{
#if ANKI_OS_LINUX
		PtrSize s = malloc_usable_size(ptr);
		akAllocSize -= s;
#endif
		free(ptr);
		return nullptr;
	}
}

static I64 stlAllocSize = 0;
static I64 stlMaxAllocSize = 0;
static ANKI_DONT_INLINE void* allocAlignedStl(void* userData, void* ptr, PtrSize size, PtrSize alignment)
{
	if(ptr == nullptr)
	{
#if ANKI_OS_LINUX
		stlAllocSize += size;
		stlMaxAllocSize = max(stlMaxAllocSize, stlAllocSize);
#endif
		return malloc(size);
	}
	else
	{
#if ANKI_OS_LINUX
		PtrSize s = malloc_usable_size(ptr);
		stlAllocSize -= s;
#endif
		free(ptr);
		return nullptr;
	}
}

ANKI_TEST(Util, SparseArrayBench)
{
	HeapAllocator<U8> allocAk(allocAlignedAk, nullptr);
	HeapAllocator<U8> allocStl(allocAlignedStl, nullptr);
	HeapAllocator<U8> allocTml(allocAligned, nullptr);

	using StlMap =
		std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, HeapAllocator<std::pair<const int, int>>>;
	StlMap stdMap(10, std::hash<int>(), std::equal_to<int>(), allocStl);

	using AkMap = SparseArray<int, U32>;
	AkMap akMap(256, U32(log2(256.0f)), 0.90f);

	HighRezTimer timer;

	const U COUNT = 1024 * 1024 * 6;

	// Create a huge set
	std::vector<int> vals;

	{
		std::unordered_map<int, int> tmpMap;

		for(U i = 0; i < COUNT; ++i)
		{
			// Put unique keys
			int v;
			do
			{
				v = rand();
			} while(tmpMap.find(v) != tmpMap.end() && v != 0);
			tmpMap[v] = 1;

			vals.push_back(v);
		}
	}

	// Insertion
	{
		// AnkI
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			akMap.emplace(allocAk, vals[i], vals[i]);
		}
		timer.stop();
		Second akTime = timer.getElapsedTime();

		// STL
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			stdMap[vals[i]] = vals[i];
		}
		timer.stop();
		Second stlTime = timer.getElapsedTime();

		ANKI_TEST_LOGI("Inserting bench: STL %f AnKi %f | %f%%", stlTime, akTime, stlTime / akTime * 100.0);
	}

	// Search
	{
		// Search in random order
		std::random_shuffle(vals.begin(), vals.end());

		int count = 0;

		// Find values AnKi
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			auto it = akMap.find(vals[i]);
			count += *it;
		}
		timer.stop();
		Second akTime = timer.getElapsedTime();

		// Find values STL
		timer.start();
		for(U i = 0; i < COUNT; ++i)
		{
			count += stdMap[vals[i]];
		}
		timer.stop();
		Second stlTime = timer.getElapsedTime();

		// Print the "count" so that the compiler won't optimize it
		ANKI_TEST_LOGI("Find bench: STL %f AnKi %f | %f%% (r:%d)", stlTime, akTime, stlTime / akTime * 100.0, count);
	}

	// Mem usage
	const I64 stlMemUsage = stlMaxAllocSize + sizeof(stdMap);
	const I64 akMemUsage = akMaxAllocSize + sizeof(akMap);
	ANKI_TEST_LOGI("Max mem usage: STL %li AnKi %li | %f%% (At any given time what was the max mem usage)", stlMemUsage,
				   akMemUsage, F64(stlMemUsage) / F32(akMemUsage) * 100.0f);

	// Deletes
	{
		// Remove in random order
		std::random_shuffle(vals.begin(), vals.end());

		// Random delete AnKi
		Second akTime = 0.0;
		for(U i = 0; i < vals.size(); ++i)
		{
			auto it = akMap.find(vals[i]);

			timer.start();
			akMap.erase(allocAk, it);
			timer.stop();
			akTime += timer.getElapsedTime();
		}

		// Random delete STL
		Second stlTime = 0.0;
		for(U i = 0; i < vals.size(); ++i)
		{
			auto it = stdMap.find(vals[i]);

			timer.start();
			stdMap.erase(it);
			timer.stop();
			stlTime += timer.getElapsedTime();
		}

		ANKI_TEST_LOGI("Deleting bench: STL %f AnKi %f | %f%%\n", stlTime, akTime, stlTime / akTime * 100.0);
	}

	akMap.destroy(allocAk);
}
