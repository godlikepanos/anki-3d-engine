// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/Thread.h"
#include "anki/util/StdTypes.h"
#include "anki/util/HighRezTimer.h"
#include <cstring>

namespace anki {

//==============================================================================
ANKI_TEST(Util, Thread)
{
	static const char* THREAD_NAME = "name";
	Thread t(THREAD_NAME);

	static const U64 NUMBER = 0xFAFAFAFABABABA;
	U64 u = NUMBER;

	t.start(&u, [](Thread::Info& info) -> I
	{
		Bool check = true; 
		
		// Check name
		check = check && (std::strcmp(info.m_threadName, THREAD_NAME) == 0);

		// Check user value
		U64& num = *reinterpret_cast<U64*>(info.m_userData);
		check = check && (num == NUMBER);

		// Change number
		num = 0xF00;

		HighRezTimer::sleep(1.0);

		return check != true;
	});


	I err = t.join();
	ANKI_TEST_EXPECT_EQ(err, 0);
	ANKI_TEST_EXPECT_EQ(u, 0xF00);
}

//==============================================================================
ANKI_TEST(Util, Mutex)
{
	Thread t0(nullptr), t1(nullptr);
	Mutex mtx;

	static const U ITERATIONS = 1000000;

	class In
	{
	public:
		I64 m_num = ITERATIONS;
		Mutex* m_mtx;
	};

	In in;
	in.m_mtx = &mtx;

	t0.start(&in, [](Thread::Info& info) -> I
	{
		In& in = *reinterpret_cast<In*>(info.m_userData);
		I64& num = in.m_num;
		Mutex& mtx = *in.m_mtx;

		for(U i = 0; i < ITERATIONS; i++)
		{
			mtx.lock();
			num += 2;
			mtx.unlock();
		}

		return 0;
	});

	t1.start(&in, [](Thread::Info& info) -> I
	{
		In& in = *reinterpret_cast<In*>(info.m_userData);
		I64& num = in.m_num;
		Mutex& mtx = *in.m_mtx;

		for(U i = 0; i < ITERATIONS; i++)
		{
			mtx.lock();
			--num;
			mtx.unlock();
		}

		return 0;
	});


	t0.join();
	t1.join();

	ANKI_TEST_EXPECT_EQ(in.m_num, ITERATIONS * 2);
}

//==============================================================================

/// Struct for our tests
struct TestJobTP: Threadpool::Task
{
	U32 in = 0;
	U32 iterations = 0;

	void operator()(U32 /*threadId*/, PtrSize /*threadsCount*/)
	{
		for(U32 i = 0; i < iterations; i++)	
		{
			++in;
		}
	}
};

} // end namespace anki

ANKI_TEST(Util, Threadpool)
{
	const U32 threadsCount = 4;
	const U32 repeat = 500;
	Threadpool* tp = new Threadpool(threadsCount);

	TestJobTP jobs[threadsCount];

	for(U32 i = 1; i < repeat; i++)
	{
		U32 iterations = rand() % 100000;

		for(U32 j = 0; j < threadsCount; j++)
		{
			jobs[j].in = i;
			jobs[j].iterations = iterations;

			tp->assignNewTask(j, &jobs[j]);
		}

		tp->waitForAllThreadsToFinish();

		for(U32 j = 0; j < threadsCount; j++)
		{
			ANKI_TEST_EXPECT_EQ(jobs[j].in, i + iterations);
		}
	}

	delete tp;
}

