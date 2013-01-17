#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Allocator.h"
#include <string>

ANKI_TEST(Allocator, StackAllocator)
{
	Foo::reset();

	// With simple string
	{
		StackAllocator<char, false> alloc(128);
		typedef std::basic_string<char, std::char_traits<char>, 
			StackAllocator<char, false>> Str;

		Str str(alloc);

		str = "lalala";
		str = "lalalalo";
	}

	// With vector
	{
		StackAllocator<Foo> alloc(128);
		std::vector<Foo, StackAllocator<Foo>> vec(alloc);

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

