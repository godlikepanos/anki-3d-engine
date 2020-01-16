// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "tests/util/Foo.h"
#include "anki/util/Memory.h"
#include "anki/util/ThreadPool.h"
#include <type_traits>
#include <cstring>

ANKI_TEST(Util, HeapMemoryPool)
{
	// Simple
	{
		HeapMemoryPool pool;
		pool.create(allocAligned, nullptr);

		void* ptr = pool.allocate(123, 1);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

		pool.free(ptr);
	}

	// Simple array
	{
		HeapMemoryPool pool;
		pool.create(allocAligned, nullptr);

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

		pool.create(allocAligned, nullptr, 10);
	}

	// Allocate
	{
		StackMemoryPool pool;

		pool.create(allocAligned, nullptr, 100, 1.0, 0, true);

		void* a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 1);

		pool.free(a);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 0);

		// Allocate a few
		const U SIZE = 75;
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 4);

		// Reset
		pool.reset();
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 0);

		// Allocate again
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationsCount(), 4);
	}

	// Parallel
	{
		StackMemoryPool pool;
		const U THREAD_COUNT = 32;
		const U ALLOC_SIZE = 25;
		ThreadPool threadPool(THREAD_COUNT);

		class AllocateTask : public ThreadPoolTask
		{
		public:
			StackMemoryPool* m_pool = nullptr;
			Array<void*, 0xF> m_allocations;

			Error operator()(U32 taskId, PtrSize threadsCount)
			{
				for(U i = 0; i < m_allocations.getSize(); ++i)
				{
					void* ptr = m_pool->allocate(ALLOC_SIZE, 1);
					memset(ptr, U8((taskId << 4) | i), ALLOC_SIZE);
					m_allocations[i] = ptr;
				}

				return Error::NONE;
			}
		};

		pool.create(allocAligned, nullptr, 100, 1.0, 0, true);
		Array<AllocateTask, THREAD_COUNT> tasks;

		for(U32 i = 0; i < THREAD_COUNT; ++i)
		{
			tasks[i].m_pool = &pool;
			threadPool.assignNewTask(i, &tasks[i]);
		}

		ANKI_TEST_EXPECT_NO_ERR(threadPool.waitForAllThreadsToFinish());

		// Check
		for(U i = 0; i < THREAD_COUNT; ++i)
		{
			const auto& task = tasks[i];

			for(U j = 0; j < task.m_allocations.getSize(); ++j)
			{
				U8 magic = U8((i << 4) | j);
				U8* ptr = static_cast<U8*>(task.m_allocations[j]);

				for(U k = 0; k < ALLOC_SIZE; ++k)
				{
					ANKI_TEST_EXPECT_EQ(ptr[k], magic);
				}
			}
		}

		// Reset and allocate again
		pool.reset();
		for(U32 i = 0; i < THREAD_COUNT; ++i)
		{
			threadPool.assignNewTask(i, &tasks[i]);
		}
		ANKI_TEST_EXPECT_NO_ERR(threadPool.waitForAllThreadsToFinish());

		for(U i = 0; i < THREAD_COUNT; ++i)
		{
			const auto& task = tasks[i];

			for(U j = 0; j < task.m_allocations.getSize(); ++j)
			{
				U8 magic = U8((i << 4) | j);
				U8* ptr = static_cast<U8*>(task.m_allocations[j]);

				for(U k = 0; k < ALLOC_SIZE; ++k)
				{
					ANKI_TEST_EXPECT_EQ(ptr[k], magic);
				}
			}
		}
	}
}

ANKI_TEST(Util, ChainMemoryPool)
{
	// Basic test
	{
		const U size = 8;
		ChainMemoryPool pool;

		pool.create(allocAligned, nullptr, size, 2.0, 0, 1);

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

		pool.create(allocAligned, nullptr, size, 2.0, 0, 1);

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
