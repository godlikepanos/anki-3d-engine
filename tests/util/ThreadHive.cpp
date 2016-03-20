// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/ThreadHive.h>
#include <chrono>
#include <thread>

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
		Atomic<U32> m_countAtomic;
		U32 m_count;
	};
};

//==============================================================================
static void decNumber(void* arg, U32, ThreadHive& hive)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	ctx->m_countAtomic.fetchSub(2);
}

//==============================================================================
static void incNumber(void* arg, U32, ThreadHive& hive)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	ctx->m_countAtomic.fetchAdd(4);

	hive.submitTask(decNumber, arg);
}

//==============================================================================
static void taskToWaitOn(void* arg, U32, ThreadHive& hive)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ctx->m_count = 10;
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

//==============================================================================
static void taskToWait(void* arg, U32, ThreadHive& hive)
{
	ThreadHiveTestContext* ctx = static_cast<ThreadHiveTestContext*>(arg);
	ANKI_TEST_EXPECT_EQ(ctx->m_count, 10);
}

//==============================================================================
ANKI_TEST(Util, ThreadHive)
{
	const U32 threadCount = 4;
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	ThreadHive hive(threadCount, alloc);

	// Simple test
	{
		ThreadHiveTestContext ctx;
		ctx.m_countAtomic.set(0);
		const U INITIAL_TASK_COUNT = 10;

		for(U i = 0; i < INITIAL_TASK_COUNT; ++i)
		{
			hive.submitTask(incNumber, &ctx);
		}

		hive.waitAllTasks();

		ANKI_TEST_EXPECT_EQ(ctx.m_countAtomic.get(), INITIAL_TASK_COUNT * 2);
	}

	// Depedency tests
	if(0)
	{
		ThreadHiveTestContext ctx;
		ctx.m_count = 0;

		ThreadHiveTask task;
		task.m_callback = taskToWaitOn;
		task.m_argument = &ctx;

		hive.submitTasks(&task, 1);

		const U DEP_TASKS = 10;
		ThreadHiveTask dtasks[DEP_TASKS];

		for(U i = 0; i < DEP_TASKS; ++i)
		{
			dtasks[i].m_callback = taskToWait;
			dtasks[i].m_argument = &ctx;
			dtasks[i].m_inDependencies =
				WArray<ThreadHiveDependencyHandle>(&task.m_outDependency, 1);
		}

		hive.submitTasks(&dtasks[0], DEP_TASKS);

		hive.waitAllTasks();
	}
}

} // end namespace anki
