#include "anki/core/ThreadPool.h"

namespace anki {

//==============================================================================
// ThreadWorker                                                                =
//==============================================================================

//==============================================================================
ThreadWorker::ThreadWorker(U32 id_, Barrier* barrier_)
	: id(id_), barrier(barrier_)
{
	start();
}

//==============================================================================
void ThreadWorker::assignNewJob(ThreadJob* job_)
{
	mutex.lock();
	ANKI_ASSERT(job == nullptr);
	job = job_;
	mutex.unlock();
	condVar.notify_one(); // Wake the thread
}

//==============================================================================
void ThreadWorker::workingFunc()
{
	while(1)
	{
		// Wait for something
		{
			std::unique_lock<std::mutex> lock(mutex);
			while(job == nullptr)
			{
				condVar.wait(lock);
			}
		}

		// Exec
		(*job)();

		// Nullify
		{
			std::lock_guard<std::mutex> lock(mutex);
			job = nullptr;
		}

		barrier->wait();
	}
}

//==============================================================================
// ThreadPool                                                                  =
//==============================================================================

//==============================================================================
void ThreadPool::init(U threadsNum)
{
	barrier.reset(new Barrier(threadsNum + 1));

	for(U i = 0; i < threadsNum; i++)
	{
		jobs.push_back(new ThreadWorker(i, barrier.get()));
	}
}


} // end namespace anki
