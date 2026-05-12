// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Util/ThreadJobManager.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/Tracer.h>

using namespace anki;

ANKI_TEST(Util, ThreadJobManager)
{
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);

	// Simple test
	{
		constexpr U32 kTaskCount = 64;

		ThreadJobManager manager(getCpuCoresCount(), true, 16);

		Atomic<U32> atomic(0);

		for(U32 i = 0; i < kTaskCount; ++i)
		{
			manager.dispatchTask([&atomic]([[maybe_unused]] U32 tid) {
				HighRezTimer::sleep(1.0_sec);
				atomic.fetchAdd(1);
			});
		}

		manager.waitForAllTasksToFinish();

		ANKI_TEST_EXPECT_EQ(atomic.load(), kTaskCount);
	}

	DefaultMemoryPool::freeSingleton();
}

ANKI_TEST(Util, ThreadJobManagerBench)
{
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);

	const U32 threadCount = getCpuCoresCount() - 2; // Leave 2 cores for the system to breathe

	const U64 time = HighRezTimer::getCurrentTimeUs();
	constexpr U32 kTaskCount = 20 * 1024 * 1024;

	{
		ThreadJobManager manager(threadCount, true, 256);

		Atomic<U32> atomic(0);

		for(U32 i = 0; i < kTaskCount; ++i)
		{
			manager.dispatchTask([&atomic]([[maybe_unused]] U32 tid) {
				atomic.fetchAdd(1);
			});
		}

		manager.waitForAllTasksToFinish();

		ANKI_TEST_EXPECT_EQ(atomic.load(), kTaskCount);
	}

	const U64 timeDiff = HighRezTimer::getCurrentTimeUs() - time;
	ANKI_TEST_LOGI("Time spent %" PRIu64 " us / %" PRId64 " ms. %f ops per ms", timeDiff, timeDiff / 1000, F64(kTaskCount) / F64(timeDiff / 1000));

	DefaultMemoryPool::freeSingleton();
}
