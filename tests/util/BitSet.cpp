// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/BitSet.h>

ANKI_TEST(Util, BitSet)
{
	{
		BitSet<4, U8> a = {false};
		ANKI_TEST_EXPECT_EQ(a.get(3), false);
		a.set(3);
		ANKI_TEST_EXPECT_EQ(a.get(3), true);
	}

	{
		BitSet<9, U8> a = {true};
		ANKI_TEST_EXPECT_EQ(a.get(8), true);
		a.set(8, false);
		ANKI_TEST_EXPECT_EQ(a.get(8), false);
	}

	{
		BitSet<4, U64> a = {true};
		ANKI_TEST_EXPECT_EQ(a.get(3), true);
		a.set(3, false);
		ANKI_TEST_EXPECT_EQ(a.get(3), false);
	}

	{
		const U N = 120;
		BitSet<N, U64> a = {false};

		for(U i = 0; i < 1000; ++i)
		{
			U r = rand() % N;
			ANKI_TEST_EXPECT_EQ(a.get(r), false);
			a.set(r, true);
			ANKI_TEST_EXPECT_EQ(a.get(r), true);

			for(U n = 0; n < N; ++n)
			{
				if(n != r)
				{
					ANKI_TEST_EXPECT_EQ(a.get(n), false);
				}
			}

			a.set(r, false);
		}
	}
}
