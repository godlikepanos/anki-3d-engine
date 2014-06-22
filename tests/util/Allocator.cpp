// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Allocator.h"
#include <string>

ANKI_TEST(Allocators, StackAllocator)
{
	Foo::reset();

	// With simple string
	{
		StackAllocator<char, false> alloc(
			StackMemoryPool(allocAligned, nullptr, 128));
		typedef std::basic_string<char, std::char_traits<char>, 
			StackAllocator<char, false>> Str;

		Str str(alloc);

		str = "lalala";
		str = "lalalalo";
	}

	// With vector
	{
		typedef StackAllocator<Foo, false> All;
		All alloc(StackMemoryPool(allocAligned, nullptr, 
			(sizeof(Foo) + 1) * 10, 1));
		std::vector<Foo, All> vec(alloc);

		vec.reserve(10);

		U sumi = 0;
		for(U i = 0; i < 10; i++)
		{
			std::cout << "pushing" << std::endl;
			vec.push_back(Foo(10 * i));
			sumi += 10 * i;
		}

		U sum = 0;

		for(const Foo& foo : vec)
		{
			sum += foo.x;
		}

		ANKI_TEST_EXPECT_EQ(sum, sumi);
	}

	ANKI_TEST_EXPECT_EQ(Foo::constructorCallCount, 
		Foo::destructorCallCount);

	// End
	Foo::reset();
}

