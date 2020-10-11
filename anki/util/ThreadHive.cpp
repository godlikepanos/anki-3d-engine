// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/ThreadHive.h>
#include <cstring>
#include <cstdio>

namespace anki
{

#define ANKI_ENABLE_HIVE_DEBUG_PRINT 0

#if ANKI_ENABLE_HIVE_DEBUG_PRINT
#	define ANKI_HIVE_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#	define ANKI_HIVE_DEBUG_PRINT(...) ((void)0)
#endif

class ThreadHive::Thread
{
public:
	U32 m_id; ///< An ID
	anki::Thread m_thread; ///< Runs the workingFunc
	ThreadHive* m_hive;

	/// Constructor
	Thread(U32 id, ThreadHive* hive, Bool pinToCores)
		: m_id(id)
		, m_thread("anki_threadhive")
		, m_hive(hive)
	{
		ANKI_ASSERT(hive);
		m_thread.start(this, threadCallback, (pinToCores) ? I32(m_id) : -1);
	}

private:
	/// Thread callaback
	static Error threadCallback(anki::ThreadCallbackInfo& info)
	{
		Thread& self = *static_cast<Thread*>(info.m_userData);

		self.m_hive->threadRun(self.m_id);
		return Error::NONE;
	}
};

class ThreadHive::Task : public NonCopyable
{
public:
	Task* m_next; ///< Next in the list.

	ThreadHiveTaskCallback m_cb; ///< Callback that defines the task.
	void* m_arg; ///< Args for the callback.

	ThreadHiveSemaphore* m_waitSemaphore;
	ThreadHiveSemaphore* m_signalSemaphore;
};

ThreadHive::ThreadHive(U32 threadCount, GenericMemoryPoolAllocator<U8> alloc, Bool pinToCores)
	: m_slowAlloc(alloc)
	, m_alloc(alloc.getMemoryPool().getAllocationCallback(), alloc.getMemoryPool().getAllocationCallbackUserData(),
			  1024 * 4)
	, m_threadCount(threadCount)
{
	m_threads = reinterpret_cast<Thread*>(m_slowAlloc.allocate(sizeof(Thread) * threadCount));
	for(U32 i = 0; i < threadCount; ++i)
	{
		::new(&m_threads[i]) Thread(i, this, pinToCores);
	}
}

ThreadHive::~ThreadHive()
{
	if(m_threads)
	{
		{
			LockGuard<Mutex> lock(m_mtx);
			m_quit = true;

			// Wake the threads
			m_cvar.notifyAll();
		}

		// Join and destroy
		U32 threadCount = m_threadCount;
		while(threadCount-- != 0)
		{
			Error err = m_threads[threadCount].m_thread.join();
			(void)err;
			m_threads[threadCount].~Thread();
		}

		m_slowAlloc.deallocate(static_cast<void*>(m_threads), m_threadCount * sizeof(Thread));
	}
}

void ThreadHive::submitTasks(ThreadHiveTask* tasks, const U32 taskCount)
{
	ANKI_ASSERT(tasks && taskCount > 0);

	// Allocate tasks
	Task* const htasks = m_alloc.newArray<Task>(taskCount);

	// Initialize tasks
	Task* prevTask = nullptr;
	for(U32 i = 0; i < taskCount; ++i)
	{
		const ThreadHiveTask& inTask = tasks[i];
		Task& outTask = htasks[i];

		outTask.m_next = nullptr;
		outTask.m_cb = inTask.m_callback;
		outTask.m_arg = inTask.m_argument;
		outTask.m_waitSemaphore = inTask.m_waitSemaphore;
		outTask.m_signalSemaphore = inTask.m_signalSemaphore;

		// Connect tasks
		if(prevTask)
		{
			prevTask->m_next = &outTask;
		}
		prevTask = &outTask;
	}

	// Push work
	{
		LockGuard<Mutex> lock(m_mtx);

		if(m_head != nullptr)
		{
			ANKI_ASSERT(m_tail && m_head);
			m_tail->m_next = &htasks[0];
			m_tail = &htasks[taskCount - 1];
		}
		else
		{
			ANKI_ASSERT(m_tail == nullptr);
			m_head = &htasks[0];
			m_tail = &htasks[taskCount - 1];
		}

		m_pendingTasks += taskCount;

		ANKI_HIVE_DEBUG_PRINT("submit tasks\n");
	}

	// Notify all threads
	m_cvar.notifyAll();
}

void ThreadHive::threadRun(U32 threadId)
{
	Task* task = nullptr;

	while(!waitForWork(threadId, task))
	{
		// Run the task
		ANKI_ASSERT(task && task->m_cb);
		ANKI_HIVE_DEBUG_PRINT("tid: %lu will exec %p (udata: %p)\n", threadId, static_cast<void*>(task),
							  static_cast<void*>(task->m_arg));
		task->m_cb(task->m_arg, threadId, *this, task->m_signalSemaphore);

#if ANKI_EXTRA_CHECKS
		task->m_cb = nullptr;
#endif

		// Signal the semaphore as early as possible
		if(task->m_signalSemaphore)
		{
			const U32 out = task->m_signalSemaphore->m_atomic.fetchSub(1);
			(void)out;
			ANKI_ASSERT(out > 0u);
			ANKI_HIVE_DEBUG_PRINT("\tsem is %u\n", out - 1u);
		}
	}

	ANKI_HIVE_DEBUG_PRINT("tid: %lu thread quits!\n", threadId);
}

Bool ThreadHive::waitForWork(U32 threadId, Task*& task)
{
	LockGuard<Mutex> lock(m_mtx);

	ANKI_HIVE_DEBUG_PRINT("tid: %lu locking\n", threadId);

	// Complete the previous task
	if(task)
	{
		--m_pendingTasks;

		if(task->m_signalSemaphore || m_pendingTasks == 0)
		{
			// A dependency maybe got resolved or we are out of tasks. Wake them all
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

ThreadHive::Task* ThreadHive::getNewTask()
{
	Task* prevTask = nullptr;
	Task* task = m_head;
	while(task)
	{
		// Check if there are dependencies
		const Bool allDepsCompleted = task->m_waitSemaphore == nullptr || task->m_waitSemaphore->m_atomic.load() == 0;

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

#if ANKI_EXTRA_CHECKS
			task->m_next = nullptr;
#endif
			break;
		}

		prevTask = task;
		task = task->m_next;
	}

	return task;
}

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
	m_alloc.getMemoryPool().reset();

	ANKI_HIVE_DEBUG_PRINT("mt: done waiting all\n");
}

} // end namespace anki
