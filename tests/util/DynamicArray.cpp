// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/DynamicArray.h>
#include <vector>

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
		srand(time(nullptr));
		HeapAllocator<U8> alloc(allocAligned, nullptr);
		DynamicArrayAuto<DynamicArrayFoo> arr(alloc);

		std::vector<DynamicArrayFoo> vec;

		const U ITERATIONS = 1000000;

		for(U i = 0; i < ITERATIONS; ++i)
		{
			const Bool grow = arr.getSize() > 0 && (rand() & 1);
			PtrSize newSize;
			U32 value = rand();
			if(grow)
			{
				newSize = vec.size() * randRange(1.0, 4.0);
			}
			else
			{
				newSize = vec.size() * randRange(0.0, 0.9);
			}

			vec.resize(newSize, value);
			arr.resize(newSize, value);

			// Validate
			ANKI_TEST_EXPECT_EQ(arr.getSize(), vec.size());
			for(U i = 0; i < arr.getSize(); ++i)
			{
				ANKI_TEST_EXPECT_EQ(arr[i].m_x, vec[i].m_x);
			}

			arr.validate();
		}

		arr = DynamicArrayAuto<DynamicArrayFoo>(alloc);
		vec = std::vector<DynamicArrayFoo>();
		ANKI_TEST_EXPECT_GT(destructorCount, 0);
		ANKI_TEST_EXPECT_EQ(
			constructor0Count + constructor1Count + constructor2Count + constructor3Count, destructorCount);
	}
}