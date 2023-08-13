// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Framework/TestFoo.h>
#include <AnKi/Util/BlockArray.h>
#include <AnKi/Util/HighRezTimer.h>
#include <vector>

ANKI_TEST(Util, BlockArray)
{
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);

	// Simple
	TestFoo::reset();
	{
		BlockArray<TestFoo> arr;

		for(TestFoo& f : arr) // Iterate empty
		{
			f.m_x = 1;
		}

		auto it = arr.emplace(123);
		ANKI_TEST_EXPECT_EQ(it->m_x, 123);

		auto it2 = arr.emplace(124);
		ANKI_TEST_EXPECT_EQ(it2->m_x, 124);

		auto it3 = arr.emplace(666);
		ANKI_TEST_EXPECT_EQ(it3->m_x, 666);

		arr.erase(it2);
		ANKI_TEST_EXPECT_EQ(arr.getSize(), 2);

		int sum = 0;
		for(auto& it : arr)
		{
			sum += it.m_x;
		}
		ANKI_TEST_EXPECT_EQ(sum, 123 + 666);
	}
	ANKI_TEST_EXPECT_EQ(TestFoo::m_constructorCount, TestFoo::m_destructorCount);
	ANKI_TEST_EXPECT_EQ(TestFoo::m_copyCount, 0);

	// Copy
	TestFoo::reset();
	{
		constexpr U32 kInsertionCount = 100;
		BlockArray<TestFoo> arr;

		// Add some
		for(U32 i = 0; i < kInsertionCount; ++i)
		{
			arr.emplace(rand());
		}

		// Remove some
		for(U32 i = 0; i < kInsertionCount / 2; ++i)
		{
			const U32 idx = U32(rand()) % kInsertionCount;
			if(arr.indexExists(idx))
			{
				arr.erase(arr.indexToIterator(idx));
			}
			arr.validate();
		}

		// Copy
		BlockArray<TestFoo> arr2 = arr;
		arr2.validate();

		// Test
		I64 sum = 0;
		for(auto it : arr)
		{
			sum += it.m_x;
		}

		I64 sum2 = 0;
		for(auto it : arr2)
		{
			sum2 += it.m_x;
		}

		ANKI_TEST_EXPECT_EQ(sum, sum2);
		ANKI_TEST_EXPECT_EQ(arr.getSize(), arr2.getSize());
	}
	ANKI_TEST_EXPECT_EQ(TestFoo::m_constructorCount, TestFoo::m_destructorCount);
	ANKI_TEST_EXPECT_EQ(TestFoo::m_copyCount, 0);

	// Fuzzy
	TestFoo::reset();
	{
		constexpr U32 kOperations = 10000;
		std::vector<std::pair<int, int>> vec;
		BlockArray<TestFoo> arr;

		for(U32 op = 0; op < kOperations; ++op)
		{
			const int opType = rand() % 3u;

			if(opType == 0 && vec.size())
			{
				// Remove something
				const int randPos = rand() % int(vec.size());

				ANKI_TEST_EXPECT_EQ(arr[vec[randPos].first].m_x, vec[randPos].second);
				arr.erase(arr.indexToIterator(vec[randPos].first));

				vec.erase(vec.begin() + randPos);
			}
			else if(op == 1)
			{
				// Add something
				const int randVal = rand();
				auto idx = arr.emplace(randVal);
				vec.push_back({idx.getArrayIndex(), randVal});
			}
			else if(vec.size())
			{
				// Change the value

				const int randPos = rand() % int(vec.size());

				ANKI_TEST_EXPECT_EQ(arr[vec[randPos].first].m_x, vec[randPos].second);
				arr[vec[randPos].first].m_x = 1234;
				vec[randPos].second = 1234;
			}

			arr.validate();
		}

		ANKI_TEST_EXPECT_EQ(vec.size(), arr.getSize());
		for(auto it : vec)
		{
			ANKI_TEST_EXPECT_EQ(arr[it.first].m_x, it.second);
		}
	}
	ANKI_TEST_EXPECT_EQ(TestFoo::m_constructorCount, TestFoo::m_destructorCount);
	ANKI_TEST_EXPECT_EQ(TestFoo::m_copyCount, 0);

	// Performance
	if(ANKI_OPTIMIZE)
	{
		// Insertion
		std::vector<U64> vec;
		BlockArray<U64> arr;
		constexpr U kInsertionOpCount = 5'000'000;

		U64 begin = HighRezTimer::getCurrentTimeUs();
		for(U i = 0; i < kInsertionOpCount; ++i)
		{
			arr.emplace(i);
		}
		const U64 arrInsertionTime = HighRezTimer::getCurrentTimeUs() - begin;

		begin = HighRezTimer::getCurrentTimeUs();
		for(U i = 0; i < kInsertionOpCount; ++i)
		{
			vec.emplace_back(i);
		}
		const U64 vecInsertionTime = HighRezTimer::getCurrentTimeUs() - begin;

		ANKI_TEST_LOGI("Insertion time: BlockArray %luus, std::vector %luus, a/b %lu", arrInsertionTime, vecInsertionTime,
					   arrInsertionTime / vecInsertionTime);

		// Iterate
		begin = HighRezTimer::getCurrentTimeUs();
		U64 sum = 0;
		for(auto& it : arr)
		{
			sum += it;
		}
		const U64 arrIterateTime = HighRezTimer::getCurrentTimeUs() - begin;

		begin = HighRezTimer::getCurrentTimeUs();
		U64 sum2 = 0;
		for(auto& it : vec)
		{
			sum2 += it;
		}
		const U64 vecIterateTime = HighRezTimer::getCurrentTimeUs() - begin;

		ANKI_TEST_EXPECT_EQ(sum, sum2);
		ANKI_TEST_LOGI("Iteration time: BlockArray %luus, std::vector %luus, a/b %lu", arrIterateTime, vecIterateTime,
					   arrIterateTime / vecIterateTime);
	}

	DefaultMemoryPool::freeSingleton();
}
