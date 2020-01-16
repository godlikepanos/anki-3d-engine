// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/Thread.h"
#include "anki/util/StdTypes.h"
#include "anki/util/HighRezTimer.h"
#include "anki/util/ThreadPool.h"
#include <cstring>

namespace anki
{

ANKI_TEST(Util, Thread)
{
	static const char* THREAD_NAME = "name";
	Thread t(THREAD_NAME);

	static const U64 NUMBER = 0xFAFAFAFABABABA;
	U64 u = NUMBER;

	t.start(&u, [](ThreadCallbackInfo& info) -> Error {
		Bool check = true;

		// Check name
		check = check && (std::strcmp(info.m_threadName, THREAD_NAME) == 0);

		// Check user value
		U64& num = *reinterpret_cast<U64*>(info.m_userData);
		check = check && (num == NUMBER);

		// Change number
		num = 0xF00;

		HighRezTimer::sleep(1.0);

		return (check != true) ? Error::FUNCTION_FAILED : Error::NONE;
	});

	ANKI_TEST_EXPECT_EQ(u, NUMBER); // It should be the old value
	Error err = t.join();
	ANKI_TEST_EXPECT_EQ(err, 0);
	ANKI_TEST_EXPECT_EQ(u, 0xF00);
}

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

	t0.start(&in, [](ThreadCallbackInfo& info) -> Error {
		In& in = *reinterpret_cast<In*>(info.m_userData);
		I64& num = in.m_num;
		Mutex& mtx = *in.m_mtx;

		for(U i = 0; i < ITERATIONS; i++)
		{
			mtx.lock();
			num += 2;
			mtx.unlock();
		}

		return Error::NONE;
	});

	t1.start(&in, [](ThreadCallbackInfo& info) -> Error {
		In& in = *reinterpret_cast<In*>(info.m_userData);
		I64& num = in.m_num;
		Mutex& mtx = *in.m_mtx;

		for(U i = 0; i < ITERATIONS; i++)
		{

			mtx.lock();
			--num;
			mtx.unlock();
		}

		return Error::NONE;
	});

	ANKI_TEST_EXPECT_NO_ERR(t0.join());
	ANKI_TEST_EXPECT_NO_ERR(t1.join());

	ANKI_TEST_EXPECT_EQ(in.m_num, ITERATIONS * 2);
}

/// Struct for our tests
struct TestJobTP : ThreadPoolTask
{
	U32 in = 0;
	U32 iterations = 0;

	Error operator()(U32 /*threadId*/, PtrSize /*threadsCount*/)
	{
		for(U32 i = 0; i < iterations; i++)
		{
			++in;
		}

		return Error::NONE;
	}
};

} // end namespace anki

ANKI_TEST(Util, ThreadPool)
{
	const U32 threadsCount = 4;
	const U32 repeat = 5;
	ThreadPool* tp = new ThreadPool(threadsCount);

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

		ANKI_TEST_EXPECT_NO_ERR(tp->waitForAllThreadsToFinish());

		for(U32 j = 0; j < threadsCount; j++)
		{
			ANKI_TEST_EXPECT_EQ(jobs[j].in, i + iterations);
		}
	}

	delete tp;

	// Test splitThreadedProblem()
	{
		const U ITERATIONS = 100000;

		for(U it = 0; it < ITERATIONS; ++it)
		{
			const U32 problemSize = max<U32>(1, rand());
			const U32 threadCount = max<U32>(1, rand() % 128);
			U32 totalCount = 0;
			for(U32 tid = 0; tid < threadCount; ++tid)
			{
				U32 start, end;
				splitThreadedProblem(tid, threadCount, problemSize, start, end);

				if(tid == 0)
				{
					ANKI_TEST_EXPECT_EQ(start, 0);
				}
				else if(tid == threadCount - 1)
				{
					ANKI_TEST_EXPECT_EQ(end, problemSize);
				}

				totalCount += end - start;
			}

			ANKI_TEST_EXPECT_EQ(totalCount, problemSize);
		}
	}
}

ANKI_TEST(Util, Barrier)
{
	// Simple test
	{
		Barrier b(2);
		Thread t(nullptr);

		t.start(&b, [](ThreadCallbackInfo& info) -> Error {
			Barrier& b = *reinterpret_cast<Barrier*>(info.m_userData);
			b.wait();
			return Error::NONE;
		});

		b.wait();
		ANKI_TEST_EXPECT_NO_ERR(t.join());
	}
}
