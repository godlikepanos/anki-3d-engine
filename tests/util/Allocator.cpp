#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Allocator.h"
#include <string>

ANKI_TEST(Allocator, StackAllocator)
{
	StackAllocator<char> alloc(12);
	typedef std::basic_string<char, std::char_traits<char>, 
		StackAllocator<char>> Str;

	{
		Str str(alloc);

		str = "lalala";
	}
}

