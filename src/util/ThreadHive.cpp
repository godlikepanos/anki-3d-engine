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

#define ANKI_ENABLE_HIVE_DEBUG_PRINT 0

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

		self.m_hive->threadRun(self.m_id);
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

	m_storage.create(m_alloc, 1024);
}

//==============================================================================
ThreadHive::~ThreadHive()
{
	m_storage.destroy(m_alloc);

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

	U allocatedTasks;

	// Push work
	{
		LockGuard<Mutex> lock(m_mtx);

		for(U i = 0; i < taskCount; ++i)
		{
			const auto& inTask = tasks[i];
			Task& outTask = m_storage[m_allocatedTasks];

			outTask.m_cb = inTask.m_callback;
			outTask.m_arg = inTask.m_argument;
			outTask.m_depCount = 0;
			outTask.m_next = nullptr;
			outTask.m_othersDepend = false;

			// Set the dependencies
			ANKI_ASSERT(inTask.m_inDependencies.getSize() <= MAX_DEPS
				&& "For now only limited deps");
			for(U j = 0; j < inTask.m_inDependencies.getSize(); ++j)
			{
				ThreadHiveDependencyHandle dep = inTask.m_inDependencies[j];
				ANKI_ASSERT(dep < m_allocatedTasks);

				if(!m_storage[dep].done())
				{
					outTask.m_deps[outTask.m_depCount++] = dep;
					m_storage[dep].m_othersDepend = true;
				}
			}

			// Push to the list
			if(m_head == nullptr)
			{
				ANKI_ASSERT(m_tail == nullptr);
				m_head = &m_storage[m_allocatedTasks];
				m_tail = m_head;
			}
			else
			{
				ANKI_ASSERT(m_tail && m_head);
				m_tail->m_next = &outTask;
				m_tail = &outTask;
			}

			++m_allocatedTasks;
		}

		allocatedTasks = m_allocatedTasks;
		m_pendingTasks += taskCount;

		ANKI_HIVE_DEBUG_PRINT("submit tasks\n");
		// Notify all threads
		m_cvar.notifyAll();
	}

	// Set the out dependencies
	for(U i = 0; i < taskCount; ++i)
	{
		tasks[i].m_outDependency = allocatedTasks - taskCount + i;
	}
}

//==============================================================================
void ThreadHive::threadRun(U threadId)
{
	Task* task = nullptr;

	while(!waitForWork(threadId, task))
	{
		// Run the task
		ANKI_ASSERT(task && task->m_cb);
		task->m_cb(task->m_arg, threadId, *this);
		ANKI_HIVE_DEBUG_PRINT("tid: %lu executed\n", threadId);
	}

	ANKI_HIVE_DEBUG_PRINT("tid: %lu thread quits!\n", threadId);
}

//==============================================================================
Bool ThreadHive::waitForWork(U threadId, Task*& task)
{
	LockGuard<Mutex> lock(m_mtx);

	ANKI_HIVE_DEBUG_PRINT("tid: %lu locking\n", threadId);

	// Complete the previous task
	if(task)
	{
		task->m_cb = nullptr;
		--m_pendingTasks;

		if(task->m_othersDepend || m_pendingTasks == 0)
		{
			// A dependency got resolved or we are out of tasks. Wake them all
			ANKI_HIVE_DEBUG_PRINT("tid: %lu wake all\n", threadId);
			m_cvar.notifyAll();
		}
	}

	while(!m_quit && (task = getNewTask()) == nullptr)
	{
		ANKI_HIVE_DEBUG_PRINT("tid: %lu waiting\n", threadId);

		// Wait if there is no work.
		m_cvar.wait(m_mtx);
	}

	return m_quit;
}

//==============================================================================
ThreadHive::Task* ThreadHive::getNewTask()
{
	Task* prevTask = nullptr;
	Task* task = m_head;
	while(task)
	{
		if(!task->done())
		{
			// We may have a candiate

			// Check if there are dependencies
			Bool allDepsCompleted = true;
			for(U j = 0; j < task->m_depCount; ++j)
			{
				U dep = task->m_deps[j];

				if(!m_storage[dep].done())
				{
					allDepsCompleted = false;
					break;
				}
			}

			if(allDepsCompleted)
			{
				// Found something, pop it
				if(prevTask)
				{
					prevTask->m_next = task->m_next;
				}

				if(m_head == task)
				{
					m_head = task->m_next;
				}

				if(m_tail == task)
				{
					m_tail = prevTask;
				}

#if ANKI_DEBUG
				task->m_next = nullptr;
#endif
				break;
			}
		}

		prevTask = task;
		task = task->m_next;
	}

	return task;
}

//==============================================================================
void ThreadHive::waitAllTasks()
{
	ANKI_HIVE_DEBUG_PRINT("mt: waiting all\n");

	LockGuard<Mutex> lock(m_mtx);
	while(m_pendingTasks > 0)
	{
		m_cvar.wait(m_mtx);
	}

	m_head = nullptr;
	m_tail = nullptr;
	m_allocatedTasks = 0;

	ANKI_HIVE_DEBUG_PRINT("mt: done waiting all\n");
}

} // end namespace anki
