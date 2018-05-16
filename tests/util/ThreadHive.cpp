// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/ThreadHive.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/System.h>

namespace anki
{

class ThreadHiveTestContext
{
public:
	ThreadHiveTestContext()
	{
	}

	~ThreadHiveTestContext()
	{
	}

	union
	{
		Atomic<I32> m_countAtomic;
		I32 m_count;
	};
};

static void decNumber(void* arg, U32, ThreadHive& hive, ThreadHiveSemaphore* sem)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	ctx->m_countAtomic.fetchSub(2);
}

static void incNumber(void* arg, U32, ThreadHive& hive, ThreadHiveSemaphore* sem)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	ctx->m_countAtomic.fetchAdd(4);

	hive.submitTask(decNumber, arg);
}

static void taskToWaitOn(void* arg, U32, ThreadHive& hive, ThreadHiveSemaphore* sem)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	HighRezTimer::sleep(1.0);
	ctx->m_count = 10;
	HighRezTimer::sleep(0.1);
}

static void taskToWait(void* arg, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* sem)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	U prev = ctx->m_countAtomic.fetchAdd(1);
	ANKI_TEST_EXPECT_GEQ(prev, 10);
}

ANKI_TEST(Util, ThreadHive)
{
	const U32 threadCount = 4;
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ThreadHive hive(threadCount, alloc);

	// Simple test
	if(1)
	{
		ThreadHiveTestContext ctx;
		ctx.m_countAtomic.set(0);
		const U INITIAL_TASK_COUNT = 100;

		for(U i = 0; i < INITIAL_TASK_COUNT; ++i)
		{
			hive.submitTask(incNumber, &ctx);
		}

		hive.waitAllTasks();

		ANKI_TEST_EXPECT_EQ(ctx.m_countAtomic.get(), INITIAL_TASK_COUNT * 2);
	}

	// Depedency tests
	if(1)
	{
		ThreadHiveTestContext ctx;
		ctx.m_count = 0;

		ThreadHiveTask task;
		task.m_callback = taskToWaitOn;
		task.m_argument = &ctx;
		task.m_signalSemaphore = hive.newSemaphore(1);

		hive.submitTasks(&task, 1);

		const U DEP_TASKS = 10;
		ThreadHiveTask dtasks[DEP_TASKS];
		ThreadHiveSemaphore* sem = hive.newSemaphore(DEP_TASKS);

		for(U i = 0; i < DEP_TASKS; ++i)
		{
			dtasks[i].m_callback = taskToWait;
			dtasks[i].m_argument = &ctx;
			dtasks[i].m_waitSemaphore = task.m_signalSemaphore;
			dtasks[i].m_signalSemaphore = sem;
		}

		hive.submitTasks(&dtasks[0], DEP_TASKS);

		// Again
		ThreadHiveTask dtasks2[DEP_TASKS];
		for(U i = 0; i < DEP_TASKS; ++i)
		{
			dtasks2[i].m_callback = taskToWait;
			dtasks2[i].m_argument = &ctx;
			dtasks2[i].m_waitSemaphore = sem;
		}

		hive.submitTasks(&dtasks2[0], DEP_TASKS);

		hive.waitAllTasks();

		ANKI_TEST_EXPECT_EQ(ctx.m_countAtomic.get(), DEP_TASKS * 2 + 10);
	}

	// Fuzzy test
	if(1)
	{
		ThreadHiveTestContext ctx;
		ctx.m_count = 0;

		I number = 0;
		ThreadHiveSemaphore* sem = nullptr;

		const U SUBMISSION_COUNT = 100;
		const U TASK_COUNT = 1000;
		for(U i = 0; i < SUBMISSION_COUNT; ++i)
		{
			for(U j = 0; j < TASK_COUNT; ++j)
			{
				Bool cb = rand() % 2;

				number = (cb) ? number + 2 : number - 2;

				ThreadHiveTask task;
				task.m_callback = (cb) ? incNumber : decNumber;
				task.m_argument = &ctx;
				task.m_signalSemaphore = hive.newSemaphore(1);

				if((rand() % 3) == 0 && j > 0 && sem)
				{
					task.m_waitSemaphore = sem;
				}

				hive.submitTasks(&task, 1);

				if((rand() % 7) == 0)
				{
					sem = task.m_signalSemaphore;
				}
			}

			sem = nullptr;
			hive.waitAllTasks();
		}

		ANKI_TEST_EXPECT_EQ(ctx.m_countAtomic.get(), number);
	}
}

class FibTask
{
public:
	Atomic<U64>* m_sum;
	StackAllocator<U8> m_alloc;
	U64 m_n;

	FibTask(Atomic<U64>* sum, StackAllocator<U8>& alloc, U64 n)
		: m_sum(sum)
		, m_alloc(alloc)
		, m_n(n)
	{
	}

	void doWork(ThreadHive& hive)
	{
		if(m_n > 1)
		{
			FibTask* a = m_alloc.newInstance<FibTask>(m_sum, m_alloc, m_n - 1);
			FibTask* b = m_alloc.newInstance<FibTask>(m_sum, m_alloc, m_n - 2);

			Array<ThreadHiveTask, 2> tasks;
			tasks[0].m_callback = tasks[1].m_callback = FibTask::callback;
			tasks[0].m_argument = a;
			tasks[1].m_argument = b;

			hive.submitTasks(&tasks[0], tasks.getSize());
		}
		else
		{
			m_sum->fetchAdd(m_n);
		}
	}

	static void callback(void* arg, U32, ThreadHive& hive, ThreadHiveSemaphore* sem)
	{
		static_cast<FibTask*>(arg)->doWork(hive);
	}
};

static U64 fib(U64 n)
{
	if(n > 1)
	{
		return fib(n - 1) + fib(n - 2);
	}
	else
	{
		return n;
	}
}

ANKI_TEST(Util, ThreadHiveBench)
{
	static const U FIB_N = 32;

	const U32 threadCount = getCpuCoresCount();
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ThreadHive hive(threadCount, alloc, true);

	StackAllocator<U8> salloc(allocAligned, nullptr, 1024);
	Atomic<U64> sum = {0};
	FibTask task(&sum, salloc, FIB_N);

	auto timeA = HighRezTimer::getCurrentTime();
	hive.submitTask(FibTask::callback, &task);
	hive.waitAllTasks();

	auto timeB = HighRezTimer::getCurrentTime();
	const U64 serialFib = fib(FIB_N);
	auto timeC = HighRezTimer::getCurrentTime();

	ANKI_TEST_LOGI("Total time %fms. Ground truth %fms", (timeB - timeA) * 1000.0, (timeC - timeB) * 1000.0);
	ANKI_TEST_EXPECT_EQ(sum.get(), serialFib);
}

} // end namespace anki
