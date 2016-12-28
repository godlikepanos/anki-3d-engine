// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/Atomic.h>

ANKI_TEST(Util, Atomic)
{
	{
		Atomic<I32> a{MAX_I32};
		I32 b = MAX_I32;
		a.fetchAdd(1);
		++b;
		ANKI_TEST_EXPECT_EQ(a.load(), b);
	}

	{
		Atomic<I32> a{MAX_I32};
		I32 b = MAX_I32;
		a.fetchAdd(-1);
		--b;
		ANKI_TEST_EXPECT_EQ(a.load(), b);
	}

	{
		Atomic<I32> a{MAX_I32};
		I32 b = MAX_I32;
		a.fetchSub(1);
		--b;
		ANKI_TEST_EXPECT_EQ(a.load(), b);
	}

	{
		Atomic<U32> a{3};
		a.fetchSub(1);
		ANKI_TEST_EXPECT_EQ(a.load(), 2);
	}

	{
		U32 u[2];
		Atomic<U32*> a{&u[0]};
		a.fetchAdd(1);
		ANKI_TEST_EXPECT_EQ(a.load(), &u[1]);
	}

	{
		U32 u[2];
		Atomic<U32*> a{&u[1]};
		a.fetchSub(1);
		ANKI_TEST_EXPECT_EQ(a.load(), &u[0]);
	}

	// Pointer of pointer
	{
		U32* pu = reinterpret_cast<U32*>(0xFFFAA);
		Atomic<U32**> a{&pu};
		a.fetchAdd(1);
		ANKI_TEST_EXPECT_EQ(a.load(), &pu + 1);
	}
}
