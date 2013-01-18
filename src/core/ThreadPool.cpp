#include "anki/core/ThreadPool.h"
#include "anki/util/Memory.h"

namespace anki {

//==============================================================================
// ThreadWorker                                                                =
//==============================================================================

//==============================================================================
ThreadWorker::ThreadWorker(U32 id_, Barrier* barrier_, ThreadPool* threadPool_)
	: id(id_), barrier(barrier_), threadPool(threadPool_)
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
		(*job)(id, threadPool->getThreadsCount());

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
ThreadPool::~ThreadPool()
{
	for(ThreadWorker* thread : jobs)
	{
		Delete<ThreadWorker>{}(thread);
	}

	Delete<Barrier>{}(barrier);
}

//==============================================================================
void ThreadPool::init(U threadsNum)
{
	ANKI_ASSERT(threadsNum <= MAX_THREADS);

	barrier = New<Barrier>{}(threadsNum + 1);

	jobs.resize(threadsNum);

	for(U i = 0; i < threadsNum; i++)
	{
		jobs[i] = New<ThreadWorker>{}(i, barrier, this);
	}
}


} // end namespace anki
