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
#if !ANKI_DISABLE_THREADPOOL_THREADING
	start();
#endif
}

//==============================================================================
void ThreadWorker::assignNewJob(ThreadJob* job_)
{
#if !ANKI_DISABLE_THREADPOOL_THREADING
	mutex.lock();
	ANKI_ASSERT(job == nullptr && "Probably forgot to wait for all jobs");
	job = job_;
	mutex.unlock();
	condVar.notify_one(); // Wake the thread
#else
	(*job_)(id, threadPool->getThreadsCount());
#endif
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
		delete thread;
	}
}

//==============================================================================
void ThreadPool::init(U threadsCount)
{
	ANKI_ASSERT(threadsCount <= MAX_THREADS && threadsCount > 0);

	barrier.reset(new Barrier(threadsCount + 1));

	jobs.resize(threadsCount);
	for(U i = 0; i < threadsCount; i++)
	{
		jobs[i] = new ThreadWorker(i, barrier.get(), this);
	}
}

} // end namespace anki
