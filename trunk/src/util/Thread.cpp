#include "anki/util/Thread.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
// DualSyncThread                                                              =
//==============================================================================

//==============================================================================
void DualSyncThread::workingFunc()
{
	while(task)
	{
		// Exec the task
		(*task)(id);

		// Wait for the other thread to sync
		sync(0);

		// Do nothing

		// Wait for the other thread again
		sync(1);
	}
}

//==============================================================================
// ThreadpoolThread                                                            =
//==============================================================================

//==============================================================================
ThreadpoolThread::ThreadpoolThread(U32 id_, Barrier* barrier_, 
	Threadpool* threadpool_)
	: id(id_), barrier(barrier_), task(nullptr), threadpool(threadpool_)
{
	ANKI_ASSERT(barrier && threadpool);
#if !ANKI_DISABLE_THREADPOOL_THREADING
	start();
#endif
}

//==============================================================================
void ThreadpoolThread::assignNewTask(ThreadpoolTask* task_)
{
#if !ANKI_DISABLE_THREADPOOL_THREADING
	mutex.lock();
	ANKI_ASSERT(task == nullptr && "Probably forgot to wait for all tasks");
	task = task_;
	mutex.unlock();
	condVar.notify_one(); // Wake the thread
#else
	(*task_)(id, threadpool->getThreadsCount());
#endif
}

//==============================================================================
void ThreadpoolThread::workingFunc()
{
	while(1)
	{
		// Wait for something
		{
			std::unique_lock<std::mutex> lock(mutex);
			while(task == nullptr)
			{
				condVar.wait(lock);
			}
		}

		// Exec
		(*task)(id, threadpool->getThreadsCount());

		// Nullify
		{
			std::lock_guard<std::mutex> lock(mutex);
			task = nullptr;
		}

		barrier->wait();
	}
}

//==============================================================================
// Threadpool                                                                  =
//==============================================================================

//==============================================================================
Threadpool::~Threadpool()
{
	for(ThreadpoolThread* thread : threads)
	{
		delete thread;
	}
}

//==============================================================================
void Threadpool::init(U threadsCount)
{
	ANKI_ASSERT(threadsCount <= MAX_THREADS && threadsCount > 0);

	barrier.reset(new Barrier(threadsCount + 1));

	threads.resize(threadsCount);
	for(U i = 0; i < threadsCount; i++)
	{
		threads[i] = new ThreadpoolThread(i, barrier.get(), this);
	}
}

} // end namespace
