// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Memory.h"
#include <type_traits>

ANKI_TEST(Util, HeapMemoryPool)
{
	// Simple
	{
		HeapMemoryPool pool;
		ANKI_TEST_EXPECT_NO_ERR(pool.create(allocAligned, nullptr));

		void* ptr = pool.allocate(123, 1);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

		pool.free(ptr);
	}

	// Simple array
	{
		HeapMemoryPool pool;
		ANKI_TEST_EXPECT_NO_ERR(pool.create(allocAligned, nullptr));

		void* ptr = pool.allocate(2, 1);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

		pool.free(ptr);
	}
}

ANKI_TEST(Util, StackMemoryPool)
{
	// Create/destroy test
	{
		StackMemoryPool pool;
	}

	// Create/destroy test #2
	{
		StackMemoryPool pool;

		ANKI_TEST_EXPECT_NO_ERR(pool.create(allocAligned, nullptr, 10, 4));
	}

	// Allocate
	{
		StackMemoryPool pool;

		ANKI_TEST_EXPECT_NO_ERR(pool.create(allocAligned, nullptr, 100, 4));

		void* a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 1);
		ANKI_TEST_EXPECT_GEQ(pool.getAllocatedSize(), 25);

		pool.free(a);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 0);

		// Allocate a few
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_EQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 3);

		// Reset
		pool.reset();
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 0);

		// Allocate again
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_EQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 3);
	}
}

ANKI_TEST(Util, ChainMemoryPool)
{
	// Basic test
	{
		const U size = 8;
		ChainMemoryPool pool;

		Error error = pool.create(
			allocAligned, nullptr,
			size, size + 1, 
			ChainMemoryPool::ChunkGrowMethod::MULTIPLY, 2, 1);
		ANKI_TEST_EXPECT_EQ(error, ErrorCode::NONE);

		void* mem = pool.allocate(5, 1);
		ANKI_TEST_EXPECT_NEQ(mem, nullptr);

		void* mem1 = pool.allocate(5, 1);
		ANKI_TEST_EXPECT_NEQ(mem1, nullptr);

		pool.free(mem1);
		pool.free(mem);
		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 0);
	}

	// Basic test 2
	{
		const U size = sizeof(PtrSize) + 10;
		ChainMemoryPool pool;
		
		Error error = pool.create(
			allocAligned, nullptr,
			size, size * 2, 
			ChainMemoryPool::ChunkGrowMethod::MULTIPLY, 2, 1);
		ANKI_TEST_EXPECT_EQ(error, ErrorCode::NONE);

		void* mem = pool.allocate(size, 1);
		ANKI_TEST_EXPECT_NEQ(mem, nullptr);

		void* mem1 = pool.allocate(size, 1);
		ANKI_TEST_EXPECT_NEQ(mem1, nullptr);

		void* mem3 = pool.allocate(size, 1);
		ANKI_TEST_EXPECT_NEQ(mem1, nullptr);

		pool.free(mem1);

		mem1 = pool.allocate(size, 1);
		ANKI_TEST_EXPECT_NEQ(mem1, nullptr);

		pool.free(mem3);
		pool.free(mem1);
		pool.free(mem);

		ANKI_TEST_EXPECT_EQ(pool.getChunksCount(), 0);
	}
}
