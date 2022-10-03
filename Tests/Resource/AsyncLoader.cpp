// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/Functions.h>

using namespace anki;

namespace {

class Task : public AsyncLoaderTask
{
public:
	F32 m_sleepTime = 0.0;
	Barrier* m_barrier = nullptr;
	Atomic<U32>* m_count = nullptr;
	I32 m_id = -1;
	Bool m_pause;
	Bool m_resubmit;

	Task(F32 time, Barrier* barrier, Atomic<U32>* count, I32 id = -1, Bool pause = false, Bool resubmit = false)
		: m_sleepTime(time)
		, m_barrier(barrier)
		, m_count(count)
		, m_id(id)
		, m_pause(pause)
		, m_resubmit(resubmit)
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx)
	{
		if(m_count)
		{
			auto x = m_count->fetchAdd(1);

			if(m_id >= 0)
			{
				if(m_id != static_cast<I32>(x))
				{
					ANKI_LOGE("Wrong excecution order");
					return Error::kFunctionFailed;
				}
			}
		}

		if(m_sleepTime != 0.0)
		{
			HighRezTimer::sleep(m_sleepTime);
		}

		if(m_barrier)
		{
			m_barrier->wait();
		}

		ctx.m_pause = m_pause;
		ctx.m_resubmitTask = m_resubmit;
		m_resubmit = false;

		return Error::kNone;
	}
};

class MemTask : public AsyncLoaderTask
{
public:
	HeapMemoryPool* m_pool;
	Barrier* m_barrier = nullptr;

	MemTask(HeapMemoryPool* pool, Barrier* barrier)
		: m_pool(pool)
		, m_barrier(barrier)
	{
	}

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx)
	{
		void* mem = m_pool->allocate(10, 16);
		if(!mem)
			return Error::kFunctionFailed;

		HighRezTimer::sleep(0.1);

		m_pool->free(mem);

		if(m_barrier)
		{
			m_barrier->wait();
		}

		return Error::kNone;
	}
};

} // namespace

ANKI_TEST(Resource, AsyncLoader)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Simple create destroy
	{
		AsyncLoader a;
		a.init(&pool);
	}

	// Simple task that will finish
	{
		AsyncLoader a;
		a.init(&pool);
		Barrier barrier(2);

		a.submitNewTask<Task>(0.0f, &barrier, nullptr);
		barrier.wait();
	}

	// Many tasks that will finish
	{
		AsyncLoader a;
		a.init(&pool);
		Barrier barrier(2);
		Atomic<U32> counter = {0};
		const U COUNT = 100;

		for(U i = 0; i < COUNT; i++)
		{
			Barrier* pbarrier = nullptr;

			if(i == COUNT - 1)
			{
				pbarrier = &barrier;
			}

			a.submitNewTask<Task>(0.01f, pbarrier, &counter);
		}

		barrier.wait();

		ANKI_TEST_EXPECT_EQ(counter.load(), COUNT);
	}

	// Many tasks that will _not_ finish
	{
		AsyncLoader a;
		a.init(&pool);

		for(U i = 0; i < 100; i++)
		{
			a.submitNewTask<Task>(0.0f, nullptr, nullptr);
		}
	}

	// Tasks that allocate
	{
		AsyncLoader a;
		a.init(&pool);
		Barrier barrier(2);

		for(U i = 0; i < 10; i++)
		{
			Barrier* pbarrier = nullptr;

			if(i == 9)
			{
				pbarrier = &barrier;
			}

			a.submitNewTask<MemTask>(&pool, pbarrier);
		}

		barrier.wait();
	}

	// Tasks that allocate and never finished
	{
		AsyncLoader a;
		a.init(&pool);

		for(U i = 0; i < 10; i++)
		{
			a.submitNewTask<MemTask>(&pool, nullptr);
		}
	}

	// Pause/resume
	{
		AsyncLoader a;
		a.init(&pool);
		Atomic<U32> counter(0);
		Barrier barrier(2);

		// Check if the pause will sync
		a.submitNewTask<Task>(0.5f, nullptr, &counter, 0);
		HighRezTimer::sleep(0.25); // Wait for the thread to pick the task...
		a.pause(); /// ...and then sync
		ANKI_TEST_EXPECT_EQ(counter.load(), 1);

		// Test resume
		a.submitNewTask<Task>(0.1f, nullptr, &counter, 1);
		HighRezTimer::sleep(1.0);
		ANKI_TEST_EXPECT_EQ(counter.load(), 1);
		a.resume();

		// Sync
		a.submitNewTask<Task>(0.1f, &barrier, &counter, 2);
		barrier.wait();

		ANKI_TEST_EXPECT_EQ(counter.load(), 3);
	}

	// Pause/resume
	{
		AsyncLoader a;
		a.init(&pool);
		Atomic<U32> counter(0);
		Barrier barrier(2);

		// Check task resubmit
		a.submitNewTask<Task>(0.0f, &barrier, &counter, -1, false, true);
		barrier.wait();
		barrier.wait();
		ANKI_TEST_EXPECT_EQ(counter.load(), 2);

		// Check task pause
		a.submitNewTask<Task>(0.0f, nullptr, &counter, -1, true, false);
		a.submitNewTask<Task>(0.0f, nullptr, &counter, -1, false, false);
		HighRezTimer::sleep(1.0);
		ANKI_TEST_EXPECT_EQ(counter.load(), 3);
		a.resume();
		HighRezTimer::sleep(1.0);
		ANKI_TEST_EXPECT_EQ(counter.load(), 4);

		// Check both
		counter.setNonAtomically(0);
		a.submitNewTask<Task>(0.0f, nullptr, &counter, 0, false, false);
		a.submitNewTask<Task>(0.0f, nullptr, &counter, -1, true, true);
		a.submitNewTask<Task>(0.0f, nullptr, &counter, 2, false, false);

		HighRezTimer::sleep(1.0);
		ANKI_TEST_EXPECT_EQ(counter.load(), 2);
		a.resume();
		HighRezTimer::sleep(1.0);
		ANKI_TEST_EXPECT_EQ(counter.load(), 4);
	}

	// Fuzzy test
	{
		AsyncLoader a;
		a.init(&pool);
		Barrier barrier(2);
		Atomic<U32> counter = {0};

		for(U32 i = 0; i < 10; i++)
		{
			Barrier* pbarrier = nullptr;

			if(i == 9)
			{
				pbarrier = &barrier;
			}

			a.submitNewTask<Task>(getRandomRange(0.0f, 0.5f), pbarrier, &counter, i);
		}

		barrier.wait();
		ANKI_TEST_EXPECT_EQ(counter.load(), 10);
	}
}
