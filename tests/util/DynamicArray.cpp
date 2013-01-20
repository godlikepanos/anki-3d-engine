#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/DynamicArray.h"

ANKI_TEST(DynamicArray, Test)
{
	constexpr U n = 4;
	StackAllocator<U8, true> alloc(sizeof(Foo) * n + sizeof(PtrSize));

	DynamicArray<Foo, StackAllocator<Foo, true>> arr(alloc);

	arr.resize(n, 1);

	U i = 0;
	for(const Foo& f : arr)
	{
		i += f.x;
	}

	ANKI_TEST_EXPECT_EQ(i, n);

	arr.clear();

	ANKI_TEST_EXPECT_EQ(arr.getSize(), 0);
	ANKI_TEST_EXPECT_EQ(alloc.dump(), true);

	// Again
	arr.resize(n);

	i = 0;
	for(const Foo& f : arr)
	{
		i += f.x;
	}

	ANKI_TEST_EXPECT_EQ(i, n * 666);

	arr.clear();

	ANKI_TEST_EXPECT_EQ(arr.getSize(), 0);
	ANKI_TEST_EXPECT_EQ(alloc.dump(), true);
}
