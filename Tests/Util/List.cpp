// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Util/Foo.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/HighRezTimer.h>
#include <list>

ANKI_TEST(Util, List)
{
	// Simple
	{
		List<Foo> a;
		Error err = Error::kNone;

		a.emplaceBack(10);
		a.emplaceBack(11);

		U sum = 0;

		err = a.iterateForward([&](const Foo& f) -> Error {
			sum += f.x;
			return Error::kNone;
		});

		ANKI_TEST_EXPECT_EQ(err, Error::kNone);
		ANKI_TEST_EXPECT_EQ(sum, 21);
	}

	// Sort
	{
		List<I> a;
		Error err = Error::kNone;

		a.emplaceBack(10);
		a.emplaceBack(9);
		a.emplaceBack(11);
		a.emplaceBack(2);

		a.sort();

		Array<I, 4> arr = {{2, 9, 10, 11}};
		U u = 0;

		err = a.iterateForward([&](const I& i) -> Error {
			if(arr[u++] == i)
			{
				return Error::kNone;
			}
			else
			{
				return Error::kUnknown;
			}
		});

		ANKI_TEST_EXPECT_EQ(err, Error::kNone);

		a.sort([](I& a, I& b) -> Bool {
			return a > b;
		});

		Array<I, 4> arr2 = {{11, 10, 9, 2}};
		u = 0;

		err = a.iterateForward([&](const I& i) -> Error {
			if(arr2[u++] == i)
			{
				return Error::kNone;
			}
			else
			{
				return Error::kUnknown;
			}
		});

		ANKI_TEST_EXPECT_EQ(err, Error::kNone);
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

			a.pushBack(f);
			b.push_back(f);
		}

		// auto ta = HighRezTimer::getCurrentTime();
		b.sort([](const Foo& a, const Foo& b) {
			return a.x < b.x;
		});
		// auto tb = HighRezTimer::getCurrentTime();
		a.sort([](const Foo& a, const Foo& b) {
			return a.x < b.x;
		});
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
	}

	// Iterate
	{
		List<I> a;
		Error err = Error::kNone;

		a.emplaceBack(10);
		a.emplaceBack(9);
		a.emplaceBack(11);
		a.emplaceBack(2);

		Array<I, 4> arr = {{10, 9, 11, 2}};
		U count = 0;

		// Forward
		List<I>::ConstIterator it = a.getBegin();
		for(; it != a.getEnd() && !err; ++it)
		{
			if(*it != arr[count++])
			{
				err = Error::kUnknown;
			}
		}

		ANKI_TEST_EXPECT_EQ(err, Error::kNone);

		// Backwards
		--it;
		for(; it != a.getBegin() && !err; --it)
		{
			if(*it != arr[--count])
			{
				err = Error::kUnknown;
			}
		}

		ANKI_TEST_EXPECT_EQ(err, Error::kNone);
	}

	// Erase
	{
		List<I> a;

		a.emplaceBack(10);

		a.erase(a.getBegin());
		ANKI_TEST_EXPECT_EQ(a.isEmpty(), true);

		a.emplaceBack(10);
		a.emplaceBack(20);

		a.erase(a.getBegin() + 1);
		ANKI_TEST_EXPECT_EQ(10, *a.getBegin());

		a.emplaceFront(5);
		a.emplaceBack(30);

		ANKI_TEST_EXPECT_EQ(5, *(a.getBegin()));
		ANKI_TEST_EXPECT_EQ(10, *(a.getEnd() - 2));
		ANKI_TEST_EXPECT_EQ(30, *(a.getEnd() - 1));

		a.erase(a.getEnd() - 2);
		ANKI_TEST_EXPECT_EQ(5, *(a.getBegin()));
		ANKI_TEST_EXPECT_EQ(30, *(a.getEnd() - 1));

		a.erase(a.getBegin());
		a.erase(a.getBegin());
	}
}
