#include "anki/util/Thread.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#if ANKI_POSIX
#	include <pthread.h>
#endif

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
void setCurrentThreadName(const char* name)
{
#if ANKI_POSIX
	pthread_setname_np(pthread_self(), name);
#else
	printf("Unimplemented\n");
#endif
}

//==============================================================================
// DualSyncThread                                                              =
//==============================================================================

//==============================================================================
void DualSyncThread::workingFunc()
{
	while(m_task)
	{
		// Exec the task
		(*m_task)(m_id);

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
ThreadpoolThread::ThreadpoolThread(U32 id, Barrier* barrier, 
	Threadpool* threadpool)
	: m_id(id), m_barrier(barrier), m_task(nullptr), m_threadpool(threadpool)
{
	ANKI_ASSERT(barrier && threadpool);
#if !ANKI_DISABLE_THREADPOOL_THREADING
	start();
#endif
}

//==============================================================================
void ThreadpoolThread::assignNewTask(ThreadpoolTask* task)
{
#if !ANKI_DISABLE_THREADPOOL_THREADING
	m_mutex.lock();
	ANKI_ASSERT(m_task == nullptr && "Probably forgot to wait for all tasks");
	m_task = task;
	m_mutex.unlock();
	m_condVar.notify_one(); // Wake the thread
#else
	(*task)(id, m_threadpool->getThreadsCount());
#endif
}

//==============================================================================
void ThreadpoolThread::workingFunc()
{
	setCurrentThreadName("anki_threadpool");

	while(1)
	{
		// Wait for something
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while(m_task == nullptr)
			{
				m_condVar.wait(lock);
			}
		}

		// Exec
		(*m_task)(m_id, m_threadpool->getThreadsCount());

		// Nullify
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_task = nullptr;
		}

		m_barrier->wait();
	}
}

//==============================================================================
// Threadpool                                                                  =
//==============================================================================

//==============================================================================
Threadpool::~Threadpool()
{
	while(m_threadsCount-- != 0)
	{
		delete m_threads[m_threadsCount];
	}

	if(m_threads)
	{
		delete[] m_threads;
	}
}

//==============================================================================
void Threadpool::init(U count)
{
	m_threadsCount = count;
	ANKI_ASSERT(m_threadsCount <= MAX_THREADS && m_threadsCount > 0);

	m_barrier.reset(new Barrier(m_threadsCount + 1));

	m_threads = new ThreadpoolThread*[m_threadsCount];

	while(count-- != 0)
	{
		m_threads[count] = new ThreadpoolThread(count, m_barrier.get(), this);
	}
}

} // end namespace anki
