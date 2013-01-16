#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Allocator.h"
#include <string>

ANKI_TEST(Allocator, StackAllocator)
{
	StackAllocator<char, false> alloc(128);
	typedef std::basic_string<char, std::char_traits<char>, 
		StackAllocator<char, false>> Str;

	{
		Str str(alloc);

		str = "lalala";
		str = "lalalalo";
	}
}

