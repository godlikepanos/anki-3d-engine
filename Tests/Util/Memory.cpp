// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Util/Foo.h>
#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/ThreadPool.h>
#include <type_traits>
#include <cstring>

ANKI_TEST(Util, HeapMemoryPool)
{
	// Simple
	{
		HeapMemoryPool pool(allocAligned, nullptr);

		void* ptr = pool.allocate(123, 1);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

		pool.free(ptr);
	}

	// Simple array
	{
		HeapMemoryPool pool(allocAligned, nullptr);

		void* ptr = pool.allocate(2, 1);
		ANKI_TEST_EXPECT_NEQ(ptr, nullptr);

		pool.free(ptr);
	}
}

ANKI_TEST(Util, StackMemoryPool)
{
	// Create/destroy test
	{
		StackMemoryPool pool(allocAligned, nullptr, 10);
	}

	// Allocate
	{
		StackMemoryPool pool(allocAligned, nullptr, 100, 1.0, 0, true);

		void* a = pool.allocate(25, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationCount(), 1);

		pool.free(a);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationCount(), 0);

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
		ANKI_TEST_EXPECT_EQ(pool.getAllocationCount(), 4);

		// Reset
		pool.reset();
		ANKI_TEST_EXPECT_EQ(pool.getAllocationCount(), 0);

		// Allocate again
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		a = pool.allocate(SIZE, 1);
		ANKI_TEST_EXPECT_NEQ(a, nullptr);
		ANKI_TEST_EXPECT_EQ(pool.getAllocationCount(), 4);
	}

	// Parallel
	{
		StackMemoryPool pool(allocAligned, nullptr, 100, 1.0, 0, true);
		const U THREAD_COUNT = 32;
		const U ALLOC_SIZE = 25;
		ThreadPool threadPool(THREAD_COUNT);

		class AllocateTask : public ThreadPoolTask
		{
		public:
			StackMemoryPool* m_pool = nullptr;
			Array<void*, 0xF> m_allocations;

			Error operator()(U32 taskId, [[maybe_unused]] PtrSize threadsCount)
			{
				for(U i = 0; i < m_allocations.getSize(); ++i)
				{
					void* ptr = m_pool->allocate(ALLOC_SIZE, 1);
					memset(ptr, U8((taskId << 4) | i), ALLOC_SIZE);
					m_allocations[i] = ptr;
				}

				return Error::kNone;
			}
		};

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
