// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/ThreadPool.h>
#include <anki/util/Logger.h>
#include <cstdlib>
#include <new>

namespace anki
{

namespace detail
{

/// The thread that executes a ThreadPoolTask
class ThreadPoolThread
{
public:
	U32 m_id; ///< An ID
	Thread m_thread; ///< Runs the workingFunc
	ThreadPoolTask* m_task; ///< Its NULL if there is no pending task
	ThreadPool* m_threadpool;
	Bool m_quit = false;

	/// Constructor
	ThreadPoolThread(U32 id, ThreadPool* threadpool, Bool pinToCore)
		: m_id(id)
		, m_thread("anki_threadpool")
		, m_task(nullptr)
		, m_threadpool(threadpool)
	{
		ANKI_ASSERT(threadpool);
		m_thread.start(this, threadCallback, (pinToCore) ? I32(m_id) : -1);
	}

private:
	/// Thread callaback
	static Error threadCallback(ThreadCallbackInfo& info)
	{
		ThreadPoolThread& self = *static_cast<ThreadPoolThread*>(info.m_userData);
		Barrier& barrier = self.m_threadpool->m_barrier;
		const PtrSize threadCount = self.m_threadpool->getThreadCount();
		Bool quit = false;

		while(!quit)
		{
			// Wait for something
			barrier.wait();
			quit = self.m_quit;

			// Exec
			Error err = (*self.m_task)(self.m_id, threadCount);
			if(err)
			{
				self.m_threadpool->m_err = err;
			}

			// Sync with main thread
			barrier.wait();
		}

		return Error::NONE;
	}
};

} // end namespace detail

ThreadPool::DummyTask ThreadPool::m_dummyTask;

ThreadPool::ThreadPool(U32 threadCount, Bool pinToCores)
	: m_barrier(threadCount + 1)
{
	m_threadsCount = threadCount;
	ANKI_ASSERT(m_threadsCount <= MAX_THREADS && m_threadsCount > 0);

	m_threads = static_cast<detail::ThreadPoolThread*>(malloc(sizeof(detail::ThreadPoolThread) * m_threadsCount));

	if(m_threads == nullptr)
	{
		ANKI_UTIL_LOGF("Out of memory");
	}

	while(threadCount-- != 0)
	{
		::new(&m_threads[threadCount]) detail::ThreadPoolThread(threadCount, this, pinToCores);
	}
}

ThreadPool::~ThreadPool()
{
	// Terminate threads
	U count = m_threadsCount;
	while(count-- != 0)
	{
		detail::ThreadPoolThread& thread = m_threads[count];

		thread.m_quit = true;
		thread.m_task = &m_dummyTask;
	}

	// Wakeup the threads
	m_barrier.wait();

	// Wait the threads
	m_barrier.wait();

	while(m_threadsCount-- != 0)
	{
		Error err = m_threads[m_threadsCount].m_thread.join();
		(void)err;
		m_threads[m_threadsCount].~ThreadPoolThread();
	}

	if(m_threads)
	{
		free(m_threads);
	}
}

void ThreadPool::assignNewTask(U32 slot, ThreadPoolTask* task)
{
	ANKI_ASSERT(slot < getThreadCount());
	if(task == nullptr)
	{
		task = &m_dummyTask;
	}

	m_threads[slot].m_task = task;
	++m_tasksAssigned;

	if(m_tasksAssigned == m_threadsCount)
	{
		// Last task is assigned. Wake all threads
		m_barrier.wait();
	}
}

} // end namespace anki
