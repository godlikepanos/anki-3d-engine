// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/List.h"

ANKI_TEST(Util, List)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Simple
	{
		List<Foo> a;
		Error err = ErrorCode::NONE;

		err = a.emplaceBack(alloc, 10);
		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);
		err = a.emplaceBack(alloc, 11);
		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);

		U sum = 0;

		err = a.iterateForward([&](const Foo& f) -> Error
		{
			sum += f.x;
			return ErrorCode::NONE;
		});

		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);
		ANKI_TEST_EXPECT_EQ(sum, 21);

		a.destroy(alloc);
	}

	// Sort
	{
		List<I> a;
		Error err = ErrorCode::NONE;

		err = a.emplaceBack(alloc, 10);
		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);
		err = a.emplaceBack(alloc, 9);
		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);
		err = a.emplaceBack(alloc, 11);
		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);
		err = a.emplaceBack(alloc, 2);
		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);

		a.sort();

		Array<I, 4> arr = {{2, 9, 10, 11}};
		U u = 0;

		err = a.iterateForward([&](const I& i) -> Error
		{
			if(arr[u++] == i)
			{
				return ErrorCode::NONE;
			}
			else
			{
				return ErrorCode::UNKNOWN;
			}
		});

		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);

		a.sort([](I& a, I& b) -> Bool
		{
			return a > b;
		});

		Array<I, 4> arr2 = {{11, 10, 9, 2}};
		u = 0;

		err = a.iterateForward([&](const I& i) -> Error
		{
			if(arr2[u++] == i)
			{
				return ErrorCode::NONE;
			}
			else
			{
				return ErrorCode::UNKNOWN;
			}
		});

		ANKI_TEST_EXPECT_EQ(err, ErrorCode::NONE);

		a.destroy(alloc);
	}
}

