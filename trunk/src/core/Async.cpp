#include "anki/core/Async.h"
#include "anki/core/Logger.h"
#include "anki/util/HighRezTimer.h"
#include "anki/util/Functions.h"

#define DEBUG_ASYNC 1

namespace anki {

//==============================================================================
Async::~Async()
{
	ANKI_ASSERT(pendingJobs.size() == 0 && finishedJobs.size() == 0);
	ANKI_ASSERT(started == false);
}

//==============================================================================
void Async::start()
{
	ANKI_ASSERT(started == false);
	started = true;

#if DEBUG_ASYNC
	ANKI_LOGI("Starting async thread...");
#endif
	thread = std::thread(&Async::workingFunc, this);
}

//==============================================================================
void Async::stop()
{
	ANKI_ASSERT(started == true);

	assignNewJobInternal(nullptr);
	thread.join();
	started = false; // Do after join
}

//==============================================================================
void Async::assignNewJobInternal(AsyncJob* job)
{
	ANKI_ASSERT(started == true);

#if DEBUG_ASYNC
	ANKI_LOGI("Assigning new job: " << job);
#endif
	pendingJobsMtx.lock();
	pendingJobs.push_back(job);
	pendingJobsMtx.unlock();

	condVar.notify_one();
}

//==============================================================================
void Async::workingFunc()
{
	while(1)
	{
		AsyncJob* job;

		// Wait for something
		{
			std::unique_lock<std::mutex> lock(pendingJobsMtx);
			while(pendingJobs.empty())
			{
#if DEBUG_ASYNC
				ANKI_LOGI("Waiting...");
#endif
				condVar.wait(lock);
			}

			job = pendingJobs.front();
			pendingJobs.pop_front();
		}

		if(job == nullptr)
		{
#if DEBUG_ASYNC
			ANKI_LOGI("Assigned to a nullptr Job. Exiting thread");
#endif
			break;
		}

		// Exec the loader
		try
		{
			(*job)();
		}
		catch(const std::exception& e)
		{
			ANKI_LOGE("Job failed: " << e.what());
		}
#if DEBUG_ASYNC
		ANKI_LOGI("Job finished: " << job);
#endif

		// Put back the response
		{
			std::unique_lock<std::mutex> lock(finishedJobsMtx);
			finishedJobs.push_back(job);
		}
	} // end thread loop

#if DEBUG_ASYNC
	ANKI_LOGI("Working thread exiting");
#endif
}

//==============================================================================
void Async::cleanupFinishedJobs(F32 maxTime)
{
	ANKI_ASSERT(started == true);
	HighRezTimer tim;
	tim.start();

	while(1)
	{
		std::unique_lock<std::mutex> lock(finishedJobsMtx);

		if(finishedJobs.size() == 0)
		{
			break;
		}

		AsyncJob* job = finishedJobs.front();
#if DEBUG_ASYNC
		ANKI_LOGI("Executing post for job: " << job);
#endif
		job->post();

		if(job->garbageCollect)
		{
			propperDelete(job);
		}

		finishedJobs.pop_front();

		// Leave if you passed the max time
		if(tim.getElapsedTime() >= maxTime)
		{
			break;
		}
	}
}

} // end namespace anki
