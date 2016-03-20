// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/ThreadHive.h>
#include <cstring>
#include <cstdio>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

#define ANKI_ENABLE_HIVE_DEBUG_PRINT 1

#if ANKI_ENABLE_HIVE_DEBUG_PRINT
#define ANKI_HIVE_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define ANKI_HIVE_DEBUG_PRINT(...) ((void)0)
#endif

class ThreadHiveThread
{
public:
	U32 m_id; ///< An ID
	Thread m_thread; ///< Runs the workingFunc
	ThreadHive* m_hive;

	/// Constructor
	ThreadHiveThread(U32 id, ThreadHive* hive)
		: m_id(id)
		, m_thread("anki_threadhive")
		, m_hive(hive)
	{
		ANKI_ASSERT(hive);
		m_thread.start(this, threadCallback);
	}

private:
	/// Thread callaback
	static Error threadCallback(Thread::Info& info)
	{
		ThreadHiveThread& self =
			*reinterpret_cast<ThreadHiveThread*>(info.m_userData);

		self.m_hive->run(self.m_id);
		return ErrorCode::NONE;
	}
};

//==============================================================================
// ThreadHive                                                                  =
//==============================================================================

//==============================================================================
ThreadHive::ThreadHive(U threadCount, GenericMemoryPoolAllocator<U8> alloc)
	: m_alloc(alloc)
	, m_threadCount(threadCount)
{
	m_threads = reinterpret_cast<ThreadHiveThread*>(
		alloc.allocate(sizeof(ThreadHiveThread) * threadCount));
	for(U i = 0; i < threadCount; ++i)
	{
		::new(&m_threads[i]) ThreadHiveThread(i, this);
	}

	m_queue.create(m_alloc, 1024);
}

//==============================================================================
ThreadHive::~ThreadHive()
{
	m_queue.destroy(m_alloc);

	if(m_threads)
	{
		{
			LockGuard<Mutex> lock(m_mtx);
			m_quit = true;

			// Wake the threads
			m_cvar.notifyAll();
		}

		// Join and destroy
		U threadCount = m_threadCount;
		while(threadCount-- != 0)
		{
			Error err = m_threads[threadCount].m_thread.join();
			(void)err;
			m_threads[threadCount].~ThreadHiveThread();
		}

		m_alloc.deallocate(static_cast<void*>(m_threads),
			m_threadCount * sizeof(ThreadHiveThread));
	}
}

//==============================================================================
void ThreadHive::submitTasks(ThreadHiveTask* tasks, U taskCount)
{
	ANKI_ASSERT(tasks && taskCount > 0);

	// Create the tasks to temp memory to decrease thread contention
	Array<Task, 64> tempTasks;
	for(U i = 0; i < taskCount; ++i)
	{
		tempTasks[i].m_cb = tasks[i].m_callback;
		tempTasks[i].m_arg = tasks[i].m_argument;
		tempTasks[i].m_depsU64 = 0;

		ANKI_ASSERT(tasks[i].m_inDependencies.getSize() <= MAX_DEPS
			&& "For now only limited deps");
		for(U j = 0; j < tasks[i].m_inDependencies.getSize(); ++j)
		{
			tempTasks[i].m_deps[j] = tasks[i].m_inDependencies[j];
		}
	}

	// Push work
	I firstTaskIdx;

	{
		LockGuard<Mutex> lock(m_mtx);

		// "Allocate" storage for tasks
		firstTaskIdx = m_tail + 1;
		m_tail += taskCount;

		// Store tasks
		memcpy(&m_queue[firstTaskIdx], &tempTasks[0], sizeof(Task) * taskCount);

		// Notify all threads
		m_cvar.notifyAll();
	}

	// Set the out dependencies
	for(U i = 0; i < taskCount; ++i)
	{
		tasks[i].m_outDependency = firstTaskIdx + i;
	}
}

//==============================================================================
void ThreadHive::run(U threadId)
{
	U64 threadMask = 1 << threadId;

	while(1)
	{
		// Wait for something
		ThreadHiveTaskCallback cb;
		void* arg;
		Bool quit;

		{
			LockGuard<Mutex> lock(m_mtx);

			ANKI_HIVE_DEBUG_PRINT("tid: %lu locking\n", threadId);

			while(!m_quit && (cb = getNewWork(arg)) == nullptr)
			{
				ANKI_HIVE_DEBUG_PRINT("tid: %lu waiting, cb %p\n", 
					threadId, 
					reinterpret_cast<void*>(cb));

				m_waitingThreadsMask |= threadMask;

				if(__builtin_popcount(m_waitingThreadsMask) == m_threadCount)
				{
					ANKI_HIVE_DEBUG_PRINT("tid: %lu all threads done. 0x%lu\n", 
						threadId, 
						m_waitingThreadsMask);
					LockGuard<Mutex> lock2(m_mainThreadMtx);

					m_mainThreadStopWaiting = true;

					// Everyone is waiting. Wake the main thread
					m_mainThreadCvar.notifyOne();
				}

				// Wait if there is no work.
				m_cvar.wait(m_mtx);
			}

			m_waitingThreadsMask &= ~threadMask;
			quit = m_quit;
		}

		if(quit)
		{
			break;
		}

		// Run the task
		cb(arg, threadId, *this);
		ANKI_HIVE_DEBUG_PRINT("dit: %lu executed\n", threadId);
	}

	ANKI_HIVE_DEBUG_PRINT("dit: %lu thread quits!\n", threadId);
}

//==============================================================================
ThreadHiveTaskCallback ThreadHive::getNewWork(void*& arg)
{
	ThreadHiveTaskCallback cb = nullptr;

	for(I i = m_head; cb == nullptr && i <= m_tail; ++i)
	{
		Task& task = m_queue[i];
		if(!task.done())
		{
			// We may have a candiate

			// Check if there are dependencies
			Bool allDepsCompleted = true;
			if(task.m_depsU64 != 0)
			{
				for(U j = 0; j < MAX_DEPS; ++j)
				{
					I32 dep = task.m_deps[j];

					if(dep < m_head || dep > m_tail || !m_queue[dep].done())
					{
						allDepsCompleted = false;
						break;
					}
				}
			}

			if(allDepsCompleted)
			{
				// Found something
				cb = task.m_cb;
				arg = task.m_arg;

				// "Complete" the task
				task.m_cb = nullptr;

				if(ANKI_UNLIKELY(m_head == m_tail))
				{
					// Reset it
					m_head = 0;
					m_tail = -1;
				}
				else if(i == m_head)
				{
					// Pop front
					++m_head;
				}
				else if(i == m_tail)
				{
					// Pop back
					--m_tail;
				}
			}
		}
	}

	return cb;
}

//==============================================================================
void ThreadHive::waitAllTasks()
{
	ANKI_HIVE_DEBUG_PRINT("mt: waiting all\n");
	LockGuard<Mutex> lock(m_mainThreadMtx);

	while(!m_mainThreadStopWaiting)
	{
		m_mainThreadCvar.wait(m_mainThreadMtx);
	}

	m_mainThreadStopWaiting = false;

	ANKI_HIVE_DEBUG_PRINT("mt: done waiting all\n");
}

} // end namespace anki
