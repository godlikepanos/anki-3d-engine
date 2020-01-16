// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Allocator.h"
#include <string>
#include <iostream>

ANKI_TEST(Util, StackAllocator)
{
	Foo::reset();

	// With simple string
	{
		StackAllocator<char> alloc(allocAligned, nullptr, 128);
		using Str = std::basic_string<char, std::char_traits<char>, StackAllocator<char>>;

		Str str(alloc);

		str = "lalala";
		str = "lalalalo";
	}

	// With vector
	{
		using All = StackAllocator<Foo>;
		All alloc(allocAligned, nullptr, (sizeof(Foo) + 1) * 10);
		std::vector<Foo, All> vec(alloc);

		vec.reserve(10);

		U sumi = 0;
		for(U i = 0; i < 10; i++)
		{
			std::cout << "pushing" << std::endl;
			vec.push_back(Foo(10 * I32(i)));
			sumi += 10 * i;
		}

		U sum = 0;

		for(const Foo& foo : vec)
		{
			sum += foo.x;
		}

		ANKI_TEST_EXPECT_EQ(sum, sumi);
	}

	// Copy around
	{
		using Alloc = StackAllocator<Foo>;

		Alloc a(allocAligned, nullptr, (sizeof(Foo) + 1) * 10);

		Alloc b(allocAligned, nullptr, (sizeof(Foo) + 1) * 10);

		a = b;
		ANKI_TEST_EXPECT_EQ(a.getMemoryPool().getUsersCount(), 2);

		b = a;
		ANKI_TEST_EXPECT_EQ(&a.getMemoryPool(), &b.getMemoryPool());
		ANKI_TEST_EXPECT_EQ(a.getMemoryPool().getUsersCount(), 2);
	}

	ANKI_TEST_EXPECT_EQ(Foo::constructorCallCount, Foo::destructorCallCount);

	// End
	Foo::reset();
}
