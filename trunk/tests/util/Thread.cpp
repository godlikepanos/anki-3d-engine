// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/Thread.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// Struct for our tests
struct TestJobTP: ThreadpoolTask
{
	U32 in = 0;
	U32 iterations = 0;

	void operator()(ThreadId /*threadId*/, U /*threadsCount*/)
	{
		for(U32 i = 0; i < iterations; i++)	
		{
			++in;
		}
	}
};

} // end namespace anki

ANKI_TEST(Core, Threadpool)
{
	const U32 threadsCount = 4;
	const U32 repeat = 500;
	Threadpool* tp = new Threadpool;

	tp->init(threadsCount);
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

