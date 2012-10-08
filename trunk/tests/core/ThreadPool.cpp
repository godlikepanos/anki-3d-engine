#include "tests/framework/Framework.h"
#include "anki/core/ThreadPool.h"
#include "anki/core/Logger.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// Struct for our tests
struct TestJobTP: ThreadJob
{
	U32 in = 0;
	U32 iterations = 0;

	void operator()(U /*threadId*/, U /*threadsCount*/)
	{
		for(U32 i = 0; i < iterations; i++)	
		{
			++in;
		}
	}
};

} // end namespace anki

ANKI_TEST(Core, ThreadPool)
{
	const U32 threadsCount = 4;
	const U32 repeat = 500;
	ThreadPool* tp = new ThreadPool;

	tp->init(threadsCount);
	TestJobTP jobs[threadsCount];

	for(U32 i = 1; i < repeat; i++)
	{
		U32 iterations = rand() % 100000;

		for(U32 j = 0; j < threadsCount; j++)
		{
			jobs[j].in = i;
			jobs[j].iterations = iterations;

			tp->assignNewJob(j, &jobs[j]);
		}

		tp->waitForAllJobsToFinish();

		for(U32 j = 0; j < threadsCount; j++)
		{
			ANKI_TEST_EXPECT_EQ(jobs[j].in, i + iterations);
		}
	}
}

