// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/DynamicArray.h>
#include <vector>
#include <ctime>

namespace anki {

static I64 g_constructor0Count = 0;
static I64 g_constructor1Count = 0;
static I64 g_constructor2Count = 0;
static I64 g_constructor3Count = 0;
static I64 g_destructorCount = 0;
static I64 g_copyCount = 0;
static I64 g_moveCount = 0;

class DynamicArrayFoo
{
public:
	static constexpr I32 kWrongNumber = -1234;

	int m_x;

	DynamicArrayFoo()
		: m_x(0)
	{
		++g_constructor0Count;
	}

	DynamicArrayFoo(int x)
		: m_x(x)
	{
		++g_constructor1Count;
		if(m_x == kWrongNumber)
		{
			++m_x;
		}
	}

	DynamicArrayFoo(const DynamicArrayFoo& b)
		: m_x(b.m_x)
	{
		ANKI_TEST_EXPECT_NEQ(b.m_x, kWrongNumber);
		++g_constructor2Count;
	}

	DynamicArrayFoo(DynamicArrayFoo&& b)
		: m_x(b.m_x)
	{
		ANKI_TEST_EXPECT_NEQ(b.m_x, kWrongNumber);
		b.m_x = 0;
		++g_constructor3Count;
	}

	~DynamicArrayFoo()
	{
		ANKI_TEST_EXPECT_NEQ(m_x, kWrongNumber);
		m_x = kWrongNumber;
		++g_destructorCount;
	}

	DynamicArrayFoo& operator=(const DynamicArrayFoo& b)
	{
		ANKI_TEST_EXPECT_NEQ(m_x, kWrongNumber);
		ANKI_TEST_EXPECT_NEQ(b.m_x, kWrongNumber);
		m_x = b.m_x;
		++g_copyCount;
		return *this;
	}

	DynamicArrayFoo& operator=(DynamicArrayFoo&& b)
	{
		ANKI_TEST_EXPECT_NEQ(m_x, kWrongNumber);
		ANKI_TEST_EXPECT_NEQ(b.m_x, kWrongNumber);
		m_x = b.m_x;
		b.m_x = 0;
		++g_moveCount;
		return *this;
	}
};

} // end namespace anki

ANKI_TEST(Util, DynamicArray)
{
	{
		HeapMemoryPool pool(allocAligned, nullptr);
		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);

		arr.resize(0);
		arr.resize(2, 1);
		arr.resize(3, 2);
		arr.resize(4, 3);

		ANKI_TEST_EXPECT_EQ(arr.getSize(), 4);
		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 1);
		ANKI_TEST_EXPECT_EQ(arr[1].m_x, 1);
		ANKI_TEST_EXPECT_EQ(arr[2].m_x, 2);
		ANKI_TEST_EXPECT_EQ(arr[3].m_x, 3);

		arr.emplaceBack(4);
		ANKI_TEST_EXPECT_EQ(arr.getSize(), 5);
		ANKI_TEST_EXPECT_EQ(arr[4].m_x, 4);

		arr.resize(1);
		arr.resize(0);
	}

	// Fuzzy
	{
		srand(U32(time(nullptr)));
		HeapMemoryPool pool(allocAligned, nullptr);
		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);

		std::vector<DynamicArrayFoo> vec;

		const U ITERATIONS = 1000000;

		for(U i = 0; i < ITERATIONS; ++i)
		{
			const Bool grow = arr.getSize() > 0 && (rand() & 1);
			U32 newSize;
			U32 value = rand();
			if(grow)
			{
				newSize = U32(vec.size()) * getRandomRange(1, 4);
			}
			else
			{
				newSize = U32(F32(vec.size()) * getRandomRange(0.0, 0.9));
			}

			vec.resize(newSize, value);
			arr.resize(newSize, value);

			// Validate
			ANKI_TEST_EXPECT_EQ(arr.getSize(), vec.size());
			for(U32 j = 0; j < arr.getSize(); ++j)
			{
				ANKI_TEST_EXPECT_EQ(arr[j].m_x, vec[j].m_x);
			}

			arr.validate();
		}

		arr = DynamicArrayRaii<DynamicArrayFoo>(&pool);
		vec = std::vector<DynamicArrayFoo>();
		ANKI_TEST_EXPECT_GT(g_destructorCount, 0);
		ANKI_TEST_EXPECT_EQ(g_constructor0Count + g_constructor1Count + g_constructor2Count + g_constructor3Count,
							g_destructorCount);
	}
}

ANKI_TEST(Util, DynamicArrayEmplaceAt)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Empty & add to the end
	{
		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);

		arr.emplaceAt(arr.getEnd(), 12);
		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 12);
	}

	// 1 element & add to he end
	{
		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);
		arr.emplaceBack(12);

		arr.emplaceAt(arr.getEnd(), 34);

		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 12);
		ANKI_TEST_EXPECT_EQ(arr[1].m_x, 34);
	}

	// 1 element & add to 0
	{
		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);
		arr.emplaceBack(12);

		arr.emplaceAt(arr.getBegin(), 34);

		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 34);
		ANKI_TEST_EXPECT_EQ(arr[1].m_x, 12);
	}

	// A bit more complex
	{
		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);

		for(I32 i = 0; i < 10; ++i)
		{
			arr.emplaceBack(i);
		}

		arr.emplaceAt(arr.getBegin() + 4, 666);

		for(I32 i = 0; i < 10 + 1; ++i)
		{
			if(i < 4)
			{
				ANKI_TEST_EXPECT_EQ(arr[i].m_x, i);
			}
			else if(i == 4)
			{
				ANKI_TEST_EXPECT_EQ(arr[i].m_x, 666);
			}
			else
			{
				ANKI_TEST_EXPECT_EQ(arr[i].m_x, i - 1);
			}
		}
	}

	// Fuzzy
	{
		srand(U32(time(nullptr)));

		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);
		std::vector<DynamicArrayFoo> vec;

		const I ITERATIONS = 10000;
		for(I i = 0; i < ITERATIONS; ++i)
		{
			I32 randNum = rand();
			I32 op = rand() % 3;

			switch(op)
			{
			case 0:
				// Push back
				arr.emplaceBack(randNum);
				vec.emplace_back(randNum);
				break;
			case 1:
				// Insert somewhere
				if(!arr.isEmpty())
				{
					I pos = rand() % arr.getSize();
					arr.emplaceAt(arr.getBegin() + pos, randNum);
					vec.emplace(vec.begin() + pos, randNum);
				}
				break;
			default:
				// Insert at the end
				arr.emplaceAt(arr.getEnd(), randNum);
				vec.emplace(vec.end(), randNum);
				break;
			}
		}

		// Check
		ANKI_TEST_EXPECT_EQ(arr.getSize(), vec.size());
		for(U32 i = 0; i < arr.getSize(); ++i)
		{
			ANKI_TEST_EXPECT_EQ(arr[i].m_x, vec[i].m_x);
		}

		arr.destroy();
		vec.resize(0);

		ANKI_TEST_EXPECT_EQ(g_constructor0Count + g_constructor1Count + g_constructor2Count + g_constructor3Count,
							g_destructorCount);
	}
}

ANKI_TEST(Util, DynamicArrayErase)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Fuzzy
	{
		srand(U32(time(nullptr)));

		DynamicArrayRaii<DynamicArrayFoo> arr(&pool);
		std::vector<DynamicArrayFoo> vec;

		const I ITERATIONS = 10000;
		for(I i = 0; i < ITERATIONS; ++i)
		{
			if(getRandom() % 2)
			{
				const I32 r = rand();
				arr.emplaceBack(r);
				vec.push_back(r);
			}
			else if(arr.getSize() > 0)
			{
				PtrSize eraseFrom = getRandom() % arr.getSize();
				PtrSize eraseTo = getRandom() % (arr.getSize() + 1);
				if(eraseTo < eraseFrom)
				{
					swapValues(eraseTo, eraseFrom);
				}

				if(eraseTo != eraseFrom)
				{
					vec.erase(vec.begin() + eraseFrom, vec.begin() + eraseTo);
					arr.erase(arr.getBegin() + eraseFrom, arr.getBegin() + eraseTo);
				}
			}
		}

		// Check
		ANKI_TEST_EXPECT_EQ(arr.getSize(), vec.size());
		for(U32 i = 0; i < arr.getSize(); ++i)
		{
			ANKI_TEST_EXPECT_EQ(arr[i].m_x, vec[i].m_x);
		}

		arr.destroy();
		vec.resize(0);

		ANKI_TEST_EXPECT_EQ(g_constructor0Count + g_constructor1Count + g_constructor2Count + g_constructor3Count,
							g_destructorCount);
	}
}
