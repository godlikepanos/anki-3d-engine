#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Memory.h"
#include <type_traits>

ANKI_TEST(Memory, Test)
{
	constexpr U n = 4;
	StackAllocator<U8, true> alloc(sizeof(Foo) * n + sizeof(PtrSize));

	Foo* f = ANKI_NEW(Foo, alloc, 123);

	ANKI_TEST_EXPECT_EQ(f->x, 123);

	ANKI_TEST_EXPECT_EQ(alloc.getMemoryPool().getAllocatedSize(),
		sizeof(Foo) + sizeof(U32));

	ANKI_DELETE(f, alloc);

	ANKI_TEST_EXPECT_EQ(alloc.getMemoryPool().getAllocatedSize(), 0);

	// Array
	f = ANKI_NEW_ARRAY(Foo, alloc, 2, 123);

	ANKI_TEST_EXPECT_EQ(alloc.getMemoryPool().getAllocatedSize(),
		2 * sizeof(Foo) + sizeof(U32) + sizeof(PtrSize));

	ANKI_TEST_EXPECT_NEQ(alloc.getMemoryPool().getAllocatedSize(), 0);

	ANKI_DELETE_ARRAY(f, alloc);

	ANKI_TEST_EXPECT_EQ(alloc.getMemoryPool().getAllocatedSize(), 0);
}

