// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	Tracer::allocateSingleton();

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
	Tracer::freeSingleton();
}

ANKI_TEST(Util, ThreadJobManagerBench)
{
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);
	Tracer::allocateSingleton();

	const U64 time = HighRezTimer::getCurrentTimeUs();

	{
		constexpr U32 kTaskCount = 20 * 1024 * 1024;

		ThreadJobManager manager(getCpuCoresCount(), true, 256);

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
	ANKI_TEST_LOGI("Time spent %lu us / %lu ms", timeDiff, timeDiff / 1000);

	DefaultMemoryPool::freeSingleton();
	Tracer::freeSingleton();
}
