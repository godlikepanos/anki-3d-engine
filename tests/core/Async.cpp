#include "tests/framework/Framework.h"
#include "anki/core/Async.h"
#include "anki/core/Logger.h"
#include "anki/util/HighRezTimer.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// Struct for our tests
struct TestJob: AsyncJob
{
	U32 result = 0;

	void operator()()
	{
		ANKI_LOGI("-- Job " << this << " starting");
		HighRezTimer::sleep(1.0);
		++result;
		ANKI_LOGI("-- Job " << this << " done");
	}

	void post()
	{
		ANKI_LOGI("-- Job " << this << " starting post");
		HighRezTimer::sleep(1.0);
		++result;
		ANKI_LOGI("-- Job " << this << " done post");
	}
};

} // end namespace anki

ANKI_TEST(Core, Async)
{
	Async a;
	TestJob pjob;

	a.start();
	a.assignNewJob(&pjob);

	HighRezTimer::sleep(2.0);

	a.cleanupFinishedJobs(123.0);
	a.stop();

	ANKI_TEST_EXPECT_EQ(pjob.result, 2);
}

