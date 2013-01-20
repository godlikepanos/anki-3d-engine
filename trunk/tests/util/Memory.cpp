#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Memory.h"

ANKI_TEST(Memory, Test)
{
	/*constexpr U n = 4;
	StackAllocator<U8, true> alloc(sizeof(Foo) * n + sizeof(PtrSize));

	Foo* f = New<Foo, StackAllocator<Foo, true>>(n, alloc)();

	ANKI_TEST_EXPECT_EQ(alloc.dump(), false);

	DeleteArray<Foo, StackAllocator<Foo, true>>{alloc}(f);

	ANKI_TEST_EXPECT_EQ(alloc.dump(), true);*/
}

