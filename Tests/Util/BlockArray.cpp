// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Framework/TestFoo.h>
#include <AnKi/Util/BlockArray.h>
#include <vector>

ANKI_TEST(Util, BlockArray)
{
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);

	// Simple
	TestFoo::reset();
	{
		BlockArray<TestFoo> arr;
		auto it = arr.emplace(123);
		ANKI_TEST_EXPECT_EQ(it->m_x, 123);

		auto it2 = arr.emplace(124);
		ANKI_TEST_EXPECT_EQ(it2->m_x, 124);

		auto it3 = arr.emplace(666);
		ANKI_TEST_EXPECT_EQ(it3->m_x, 666);

		arr.erase(arr.getBegin() + 1);
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

	// Fuzzy
	TestFoo::reset();
	{
		constexpr U32 kOperations = 1000;
		std::vector<std::pair<int, int>> vec;
		BlockArray<TestFoo> arr;

		for(U32 op = 0; op < kOperations; ++op)
		{
			const int opType = rand() % 2u;

			if(opType == 0 && vec.size())
			{
				// Remove something
				const int randPos = rand() % vec.size();

				ANKI_TEST_EXPECT_EQ(arr[vec[randPos].first].m_x, vec[randPos].second);
				arr.erase(arr.getBegin() + vec[randPos].first);

				vec.erase(vec.begin() + randPos);
			}
			else
			{
				const int randVal = rand();
				auto it = arr.emplace(randVal);
				vec.push_back({it.getArrayIndex(), randVal});
			}
		}

		ANKI_TEST_EXPECT_EQ(vec.size(), arr.getSize());
		for(auto it : vec)
		{
			ANKI_TEST_EXPECT_EQ(arr[it.first].m_x, it.second);
		}
	}

	DefaultMemoryPool::freeSingleton();
}
