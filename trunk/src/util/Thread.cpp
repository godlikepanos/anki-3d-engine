#include "anki/util/Thread.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"

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
	while(threadsCount-- != 0)
	{
		delete threads[threadsCount];
	}

	if(threads)
	{
		delete[] threads;
	}
}

//==============================================================================
void Threadpool::init(U count)
{
	threadsCount = count;
	ANKI_ASSERT(threadsCount <= MAX_THREADS && threadsCount > 0);

	barrier.reset(new Barrier(threadsCount + 1));

	threads = new ThreadpoolThread*[threadsCount];

	while(count-- != 0)
	{
		threads[count] = new ThreadpoolThread(count, barrier.get(), this);
	}
}

} // end namespace
