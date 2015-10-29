// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/resource/AsyncLoader.h"
#include "anki/util/HighRezTimer.h"
#include "anki/util/Atomic.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
class Task: public AsyncLoaderTask
{
public:
	F32 m_sleepTime = 0.0;
	Barrier* m_barrier = nullptr;
	Atomic<U32>* m_count = nullptr;
	I32 m_id = -1;

	Task(F32 time, Barrier* barrier, Atomic<U32>* count, I32 id = -1)
		: m_sleepTime(time)
		, m_barrier(barrier)
		, m_count(count)
		, m_id(id)
	{}

	Error operator()()
	{
		if(m_count)
		{
			auto x = m_count->fetchAdd(1);

			if(m_id >= 0)
			{
				if(m_id != static_cast<I32>(x))
				{
					ANKI_LOGE("Wrong excecution order");
					return ErrorCode::FUNCTION_FAILED;
				}
			}

			return ErrorCode::NONE;
		}

		if(m_sleepTime != 0.0)
		{
			HighRezTimer::sleep(m_sleepTime);
		}

		if(m_barrier)
		{
			m_barrier->wait();
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
class MemTask: public AsyncLoaderTask
{
public:
	HeapAllocator<U8> m_alloc;
	Barrier* m_barrier = nullptr;

	MemTask(HeapAllocator<U8> alloc, Barrier* barrier)
		: m_alloc(alloc)
		, m_barrier(barrier)
	{}

	Error operator()()
	{
		void* mem = m_alloc.allocate(10);
		if(!mem) return ErrorCode::FUNCTION_FAILED;

		HighRezTimer::sleep(0.1);

		m_alloc.deallocate(mem, 10);

		if(m_barrier)
		{
			m_barrier->wait();
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
ANKI_TEST(Resource, AsyncLoader)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Simple create destroy
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));
	}

	// Simple task that will finish
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));
		Barrier barrier(2);

		a.newTask<Task>(0.0, &barrier, nullptr);
		barrier.wait();
	}

	// Many tasks that will finish
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));
		Barrier barrier(2);
		Atomic<U32> counter = {0};

		for(U i = 0; i < 100; i++)
		{
			Barrier* pbarrier = nullptr;

			if(i == 99)
			{
				pbarrier = &barrier;
			}

			a.newTask<Task>(0.0, pbarrier, &counter);
		}

		barrier.wait();

		ANKI_TEST_EXPECT_EQ(counter.load(), 100);
	}

	// Many tasks that will _not_ finish
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));

		for(U i = 0; i < 100; i++)
		{
			a.newTask<Task>(0.0, nullptr, nullptr);
		}
	}

	// Tasks that allocate
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));
		Barrier barrier(2);

		for(U i = 0; i < 10; i++)
		{
			Barrier* pbarrier = nullptr;

			if(i == 9)
			{
				pbarrier = &barrier;
			}

			a.newTask<MemTask>(alloc, pbarrier);
		}

		barrier.wait();
	}

	// Tasks that allocate and never finished
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));

		for(U i = 0; i < 10; i++)
		{
			a.newTask<MemTask>(alloc, nullptr);
		}
	}

	// Random struf
	{
		AsyncLoader a;
		ANKI_TEST_EXPECT_NO_ERR(a.create(alloc));
		Barrier barrier(2);
		Atomic<U32> counter = {0};

		for(U i = 0; i < 10; i++)
		{
			Barrier* pbarrier = nullptr;

			if(i == 9)
			{
				pbarrier = &barrier;
			}

			a.newTask<Task>(randRange(0.0, 0.5), pbarrier, &counter, i);
		}

		barrier.wait();
		ANKI_TEST_EXPECT_EQ(counter.load(), 10);
	}
}

} // end namespace anki
