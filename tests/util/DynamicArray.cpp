// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/DynamicArray.h>
#include <vector>
#include <ctime>

namespace anki
{

static I64 constructor0Count = 0;
static I64 constructor1Count = 0;
static I64 constructor2Count = 0;
static I64 constructor3Count = 0;
static I64 destructorCount = 0;
static I64 copyCount = 0;
static I64 moveCount = 0;

class DynamicArrayFoo
{
public:
	int m_x;

	DynamicArrayFoo()
		: m_x(0)
	{
		++constructor0Count;
	}

	DynamicArrayFoo(int x)
		: m_x(x)
	{
		++constructor1Count;
	}

	DynamicArrayFoo(const DynamicArrayFoo& b)
		: m_x(b.m_x)
	{
		++constructor2Count;
	}

	DynamicArrayFoo(DynamicArrayFoo&& b)
		: m_x(b.m_x)
	{
		b.m_x = 0;
		++constructor3Count;
	}

	~DynamicArrayFoo()
	{
		++destructorCount;
	}

	DynamicArrayFoo& operator=(const DynamicArrayFoo& b)
	{
		m_x = b.m_x;
		++copyCount;
		return *this;
	}

	DynamicArrayFoo& operator=(DynamicArrayFoo&& b)
	{
		m_x = b.m_x;
		b.m_x = 0;
		++moveCount;
		return *this;
	}
};

} // end namespace anki

ANKI_TEST(Util, DynamicArray)
{
	{
		HeapAllocator<U8> alloc(allocAligned, nullptr);
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);

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
		HeapAllocator<U8> alloc(allocAligned, nullptr);
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);

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

		arr = DynamicArrayAuto<DynamicArrayFoo>(alloc);
		vec = std::vector<DynamicArrayFoo>();
		ANKI_TEST_EXPECT_GT(destructorCount, 0);
		ANKI_TEST_EXPECT_EQ(constructor0Count + constructor1Count + constructor2Count + constructor3Count,
							destructorCount);
	}
}

ANKI_TEST(Util, DynamicArrayEmplaceAt)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Empty & add to the end
	{
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);

		arr.emplaceAt(arr.getEnd(), 12);
		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 12);
	}

	// 1 element & add to he end
	{
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);
		arr.emplaceBack(12);

		arr.emplaceAt(arr.getEnd(), 34);

		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 12);
		ANKI_TEST_EXPECT_EQ(arr[1].m_x, 34);
	}

	// 1 element & add to 0
	{
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);
		arr.emplaceBack(12);

		arr.emplaceAt(arr.getBegin(), 34);

		ANKI_TEST_EXPECT_EQ(arr[0].m_x, 34);
		ANKI_TEST_EXPECT_EQ(arr[1].m_x, 12);
	}

	// A bit more complex
	{
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);

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

		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);
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

		ANKI_TEST_EXPECT_EQ(constructor0Count + constructor1Count + constructor2Count + constructor3Count,
							destructorCount);
	}
}
