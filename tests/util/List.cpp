// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/List.h"
#include "anki/util/HighRezTimer.h"
#include <list>

ANKI_TEST(Util, List)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Simple
	{
		List<Foo> a;
		Error err = Error::NONE;

		a.emplaceBack(alloc, 10);
		a.emplaceBack(alloc, 11);

		U sum = 0;

		err = a.iterateForward([&](const Foo& f) -> Error {
			sum += f.x;
			return Error::NONE;
		});

		ANKI_TEST_EXPECT_EQ(err, Error::NONE);
		ANKI_TEST_EXPECT_EQ(sum, 21);

		a.destroy(alloc);
	}

	// Sort
	{
		List<I> a;
		Error err = Error::NONE;

		a.emplaceBack(alloc, 10);
		a.emplaceBack(alloc, 9);
		a.emplaceBack(alloc, 11);
		a.emplaceBack(alloc, 2);

		a.sort();

		Array<I, 4> arr = {{2, 9, 10, 11}};
		U u = 0;

		err = a.iterateForward([&](const I& i) -> Error {
			if(arr[u++] == i)
			{
				return Error::NONE;
			}
			else
			{
				return Error::UNKNOWN;
			}
		});

		ANKI_TEST_EXPECT_EQ(err, Error::NONE);

		a.sort([](I& a, I& b) -> Bool { return a > b; });

		Array<I, 4> arr2 = {{11, 10, 9, 2}};
		u = 0;

		err = a.iterateForward([&](const I& i) -> Error {
			if(arr2[u++] == i)
			{
				return Error::NONE;
			}
			else
			{
				return Error::UNKNOWN;
			}
		});

		ANKI_TEST_EXPECT_EQ(err, Error::NONE);

		a.destroy(alloc);
	}

	// Extreme sort
	{
		const U COUNT = 10000;
		List<Foo> a;
		std::list<Foo> b;

		for(U i = 0; i < COUNT; i++)
		{
			I32 randVal = rand();
			Foo f(randVal);

			a.pushBack(alloc, f);
			b.push_back(f);
		}

		// auto ta = HighRezTimer::getCurrentTime();
		b.sort([](const Foo& a, const Foo& b) { return a.x < b.x; });
		// auto tb = HighRezTimer::getCurrentTime();
		a.sort([](const Foo& a, const Foo& b) { return a.x < b.x; });
		// auto tc = HighRezTimer::getCurrentTime();

		// printf("%f %f\n", tb - ta, tc - tb);

		auto ait = a.getBegin();
		auto bit = b.begin();
		auto aend = a.getEnd();
		auto bend = b.end();

		while(ait != aend && bit != bend)
		{
			const Foo& afoo = *ait;
			const Foo& bfoo = *bit;

			ANKI_TEST_EXPECT_EQ(afoo, bfoo);
			++ait;
			++bit;
		}

		ANKI_TEST_EXPECT_EQ(ait, aend);
		ANKI_TEST_EXPECT_EQ(bit, bend);
		a.destroy(alloc);
	}

	// Iterate
	{
		List<I> a;
		Error err = Error::NONE;

		a.emplaceBack(alloc, 10);
		a.emplaceBack(alloc, 9);
		a.emplaceBack(alloc, 11);
		a.emplaceBack(alloc, 2);

		Array<I, 4> arr = {{10, 9, 11, 2}};
		U count = 0;

		// Forward
		List<I>::ConstIterator it = a.getBegin();
		for(; it != a.getEnd() && !err; ++it)
		{
			if(*it != arr[count++])
			{
				err = Error::UNKNOWN;
			}
		}

		ANKI_TEST_EXPECT_EQ(err, Error::NONE);

		// Backwards
		--it;
		for(; it != a.getBegin() && !err; --it)
		{
			if(*it != arr[--count])
			{
				err = Error::UNKNOWN;
			}
		}

		ANKI_TEST_EXPECT_EQ(err, Error::NONE);

		a.destroy(alloc);
	}

	// Erase
	{
		List<I> a;

		a.emplaceBack(alloc, 10);

		a.erase(alloc, a.getBegin());
		ANKI_TEST_EXPECT_EQ(a.isEmpty(), true);

		a.emplaceBack(alloc, 10);
		a.emplaceBack(alloc, 20);

		a.erase(alloc, a.getBegin() + 1);
		ANKI_TEST_EXPECT_EQ(10, *a.getBegin());

		a.emplaceFront(alloc, 5);
		a.emplaceBack(alloc, 30);

		ANKI_TEST_EXPECT_EQ(5, *(a.getBegin()));
		ANKI_TEST_EXPECT_EQ(10, *(a.getEnd() - 2));
		ANKI_TEST_EXPECT_EQ(30, *(a.getEnd() - 1));

		a.erase(alloc, a.getEnd() - 2);
		ANKI_TEST_EXPECT_EQ(5, *(a.getBegin()));
		ANKI_TEST_EXPECT_EQ(30, *(a.getEnd() - 1));

		a.erase(alloc, a.getBegin());
		a.erase(alloc, a.getBegin());
	}
}
