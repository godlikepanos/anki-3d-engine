// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Memory.h"
#include <type_traits>

ANKI_TEST(Memory, StackMemoryPool)
{
	/*constexpr U n = 4;
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

	ANKI_TEST_EXPECT_EQ(alloc.getMemoryPool().getAllocatedSize(), 0);*/
}

ANKI_TEST(Memory, ChainMemoryPool)
{
	// Basic test
	{
		const U size = sizeof(PtrSize) + 10;
		ChainMemoryPool pool(
			size, size, ChainMemoryPool::MULTIPLY, 2, 1);

		void* mem = pool.allocate(1, 1);
		ANKI_TEST_EXPECT_NEQ(mem, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 1);

		void* mem1 = pool.allocate(10, 1);
		ANKI_TEST_EXPECT_NEQ(mem1, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 2);

		pool.free(mem1);
		pool.free(mem);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 0);
	}

	// Basic test 2
	{
		const U size = sizeof(PtrSize) + 10;
		ChainMemoryPool pool(
			size, size, ChainMemoryPool::MULTIPLY, 2, 1);

		void* mem = pool.allocate(1, 1);
		ANKI_TEST_EXPECT_NEQ(mem, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 1);

		void* mem1 = pool.allocate(10, 1);
		ANKI_TEST_EXPECT_NEQ(mem1, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 2);

		pool.free(mem);
		pool.free(mem1);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 0);
	}
}
