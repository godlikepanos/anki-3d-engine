// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Thread.h"
#include "anki/util/Assert.h"
#include "anki/util/Logger.h"
#include "anki/util/Functions.h"
#include <cstdlib>
#include <new>
#include <cstdio>

namespace anki {

//==============================================================================
// Barrier                                                                     =
//==============================================================================

//==============================================================================
Bool Barrier::wait()
{
	m_mtx.lock();
	U32 gen = m_generation;

	if(--m_count == 0)
	{
		m_generation++;
		m_count = m_threshold;
		m_cond.notifyAll();
		m_mtx.unlock();
		return true;
	}

	while(gen == m_generation)
	{
		m_cond.wait(m_mtx);
	}

	m_mtx.unlock();
	return false;
}

//==============================================================================
// ThreadpoolThread                                                            =
//==============================================================================

#if !ANKI_DISABLE_THREADPOOL_THREADING

namespace detail {

/// The thread that executes a Threadpool::Task
class ThreadpoolThread
{
public:
	U32 m_id; ///< An ID
	Thread m_thread; ///< Runs the workingFunc
	Mutex m_mutex; ///< Protect the task
	ConditionVariable m_condVar; ///< To wake up the thread
	Threadpool::Task* m_task; ///< Its NULL if there is no pending task
	Threadpool* m_threadpool; 
	Bool8 m_quit = false;

	/// Constructor
	ThreadpoolThread(U32 id, Threadpool* threadpool)
	:	m_id(id),
		m_thread("anki_threadpool"),
		m_task(nullptr), 
		m_threadpool(threadpool)
	{
		ANKI_ASSERT(threadpool);
		m_thread.start(this, threadCallback);
	}

	/// Assign new task to the thread
	void assignNewTask(Threadpool::Task* task)
	{
		m_mutex.lock();
		ANKI_ASSERT(m_task == nullptr && "Probably forgot to wait for tasks");
		m_task = task;
		m_mutex.unlock();
		m_condVar.notifyOne(); // Wake the thread
	}

private:
	/// Thread callaback
	static Error threadCallback(Thread::Info& info)
	{
		ThreadpoolThread& self = 
			*reinterpret_cast<ThreadpoolThread*>(info.m_userData);
		Barrier& barrier = self.m_threadpool->m_barrier;
		Mutex& mtx = self.m_mutex; 
		const PtrSize threadCount = self.m_threadpool->getThreadsCount();
		Bool quit = false;

		while(!quit)
		{
			Threadpool::Task* task;

			// Wait for something
			{
				LockGuard<Mutex> lock(mtx);
				while(self.m_task == nullptr)
				{
					self.m_condVar.wait(mtx);
				}
				task = self.m_task;
				self.m_task = nullptr;

				quit = self.m_quit;
			}

			// Exec
			Error err = (*task)(self.m_id, threadCount);

			if(err)
			{
				LockGuard<Mutex> lock(mtx);
				self.m_threadpool->m_err = err;
			}

			// Sync with main thread
			barrier.wait();
		}

		return ErrorCode::NONE;
	}
};

} // end namespace detail

#endif

//==============================================================================
// Threadpool                                                                  =
//==============================================================================

//==============================================================================
Threadpool::DummyTask Threadpool::m_dummyTask;

//==============================================================================
Threadpool::Threadpool(U32 threadsCount)
#if !ANKI_DISABLE_THREADPOOL_THREADING
:	m_barrier(threadsCount + 1)
#endif
{
	m_threadsCount = threadsCount;
	ANKI_ASSERT(m_threadsCount <= MAX_THREADS && m_threadsCount > 0);

#if !ANKI_DISABLE_THREADPOOL_THREADING
	m_threads = reinterpret_cast<detail::ThreadpoolThread*>(
		malloc(sizeof(detail::ThreadpoolThread) * m_threadsCount));

	if(m_threads == nullptr)
	{
		ANKI_LOGF("Out of memory");
	}

	while(threadsCount-- != 0)
	{
		construct(&m_threads[threadsCount], threadsCount, this);
	}
#endif
}

//==============================================================================
Threadpool::~Threadpool()
{
#if !ANKI_DISABLE_THREADPOOL_THREADING
	// Terminate threads
	U count = m_threadsCount;
	while(count-- != 0)
	{
		detail::ThreadpoolThread& thread = m_threads[count];

		thread.m_quit = true;
		thread.assignNewTask(&m_dummyTask); // Wake it
	}
	
	Error err = waitForAllThreadsToFinish();
	(void)err;

	while(m_threadsCount-- != 0)
	{
		Error err = m_threads[m_threadsCount].m_thread.join();
		(void)err;
		m_threads[m_threadsCount].~ThreadpoolThread();
	}

	if(m_threads)
	{
		free(m_threads);
	}
#endif
}

//==============================================================================
void Threadpool::assignNewTask(U32 slot, Task* task)
{
	ANKI_ASSERT(slot < getThreadsCount());
	if(task == nullptr)
	{
		task = &m_dummyTask;
	}

#if !ANKI_DISABLE_THREADPOOL_THREADING
	m_threads[slot].assignNewTask(task);
#else
	Error err = (*task)(slot, m_threadsCount);

	if(err)
	{
		m_err = err;
	}
#endif
}

} // end namespace anki
