// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
class Task: public AsyncLoader::Task
{
public:
	F32 m_sleepTime = 0.0;
	Barrier* m_barrier = nullptr;
	AtomicU32* m_count = nullptr;
	I32 m_id = -1;

	Task(F32 time, Barrier* barrier, AtomicU32* count, I32 id = -1)
	:	m_sleepTime(time),
		m_barrier(barrier),
		m_count(count),
		m_id(id)
	{}

	void operator()()
	{
		if(m_count)
		{
			auto x = m_count->fetch_add(1);

			if(m_id >= 0)
			{
				if(m_id != static_cast<I32>(x))
				{
					throw ANKI_EXCEPTION("Wrong excecution order");
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
	}
};

//==============================================================================
class MemTask: public AsyncLoader::Task
{
public:
	HeapAllocator<U8> m_alloc;
	Barrier* m_barrier = nullptr;

	MemTask(HeapAllocator<U8> alloc, Barrier* barrier)
	:	m_alloc(alloc),
		m_barrier(barrier)
	{}

	void operator()()
	{
		void* mem = m_alloc.allocate(10);
		HighRezTimer::sleep(0.1);

		m_alloc.deallocate(mem, 10);

		if(m_barrier)
		{
			m_barrier->wait();
		}
	}
};

//==============================================================================
ANKI_TEST(Resource, AsyncLoader)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	// Simple create destroy
	{
		AsyncLoader a(alloc);
	}

	// Simple task that will finish
	{
		AsyncLoader a(alloc);
		Barrier barrier(2);

		a.newTask<Task>(0.0, &barrier, nullptr);
		barrier.wait();
	}

	// Many tasks that will finish
	{
		AsyncLoader a(alloc);
		Barrier barrier(2);
		AtomicU32 counter = {0};

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
		AsyncLoader a(alloc);

		for(U i = 0; i < 100; i++)
		{
			a.newTask<Task>(0.0, nullptr, nullptr);
		}
	}

	// Tasks that allocate
	{
		AsyncLoader a(alloc);
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
		AsyncLoader a(alloc);

		for(U i = 0; i < 10; i++)
		{
			a.newTask<MemTask>(alloc, nullptr);
		}
	}

	// Random struf
	{
		AsyncLoader a(alloc);
		Barrier barrier(2);
		AtomicU32 counter = {0};

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
