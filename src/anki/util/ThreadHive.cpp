// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
#define ANKI_HIVE_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define ANKI_HIVE_DEBUG_PRINT(...) ((void)0)
#endif

class ThreadHive::Thread
{
public:
	U32 m_id; ///< An ID
	anki::Thread m_thread; ///< Runs the workingFunc
	ThreadHive* m_hive;

	/// Constructor
	Thread(U32 id, ThreadHive* hive)
		: m_id(id)
		, m_thread("anki_threadhive")
		, m_hive(hive)
	{
		ANKI_ASSERT(hive);
		m_thread.start(this, threadCallback);
	}

private:
	/// Thread callaback
	static Error threadCallback(anki::ThreadCallbackInfo& info)
	{
		Thread& self = *reinterpret_cast<Thread*>(info.m_userData);

		self.m_hive->threadRun(self.m_id);
		return ErrorCode::NONE;
	}
};

class ThreadHive::Task
{
public:
	Task* m_next; ///< Next in the list.

	ThreadHiveTaskCallback m_cb; ///< Callback that defines the task.
	void* m_arg; ///< Args for the callback.

	Task** m_deps;
	U16 m_depCount;
	Bool8 m_othersDepend; ///< Other tasks depend on this one.

	Task()
	{
	}

	Task(const Task& b) = delete;
	Task& operator=(const Task& b) = delete;

	Bool done() const
	{
		return m_cb == nullptr;
	}
};

ThreadHive::ThreadHive(U threadCount, GenericMemoryPoolAllocator<U8> alloc)
	: m_slowAlloc(alloc)
	, m_alloc(alloc.getMemoryPool().getAllocationCallback(),
		  alloc.getMemoryPool().getAllocationCallbackUserData(),
		  1024 * 4)
	, m_threadCount(threadCount)
{
	m_threads = reinterpret_cast<Thread*>(m_slowAlloc.allocate(sizeof(Thread) * threadCount));
	for(U i = 0; i < threadCount; ++i)
	{
		::new(&m_threads[i]) Thread(i, this);
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
		U threadCount = m_threadCount;
		while(threadCount-- != 0)
		{
			Error err = m_threads[threadCount].m_thread.join();
			(void)err;
			m_threads[threadCount].~Thread();
		}

		m_slowAlloc.deallocate(static_cast<void*>(m_threads), m_threadCount * sizeof(Thread));
	}
}

void ThreadHive::submitTasks(ThreadHiveTask* tasks, const U taskCount)
{
	ANKI_ASSERT(tasks && taskCount > 0);

	// Allocate tasks
	Task* const htasks = m_alloc.newArray<Task>(taskCount);

	// Allocate the dependency handles
	U depCount = 0;
	for(U i = 0; i < taskCount; ++i)
	{
		depCount += tasks[i].m_inDependencies.getSize();
	}

	Task** depHandles;
	if(depCount)
	{
		depHandles = m_alloc.newArray<Task*>(depCount);
	}
	else
	{
		depHandles = nullptr;
	}

	depCount = 0;

	// Initialize tasks
	for(U i = 0; i < taskCount; ++i)
	{
		const ThreadHiveTask& inTask = tasks[i];
		Task& outTask = htasks[i];

		outTask.m_cb = inTask.m_callback;
		outTask.m_arg = inTask.m_argument;
		outTask.m_depCount = 0;
		outTask.m_next = nullptr;
		outTask.m_othersDepend = false;

		// Set the dependencies
		if(inTask.m_inDependencies.getSize() > 0)
		{
			outTask.m_deps = &depHandles[depCount];
			depCount += inTask.m_inDependencies.getSize();
		}
		else
		{
			outTask.m_deps = nullptr;
		}
	}

	// Push work
	{
		LockGuard<Mutex> lock(m_mtx);

		for(U i = 0; i < taskCount; ++i)
		{
			const ThreadHiveTask& inTask = tasks[i];
			Task& outTask = htasks[i];

			for(U j = 0; j < inTask.m_inDependencies.getSize(); ++j)
			{
				ThreadHiveDependencyHandle dep = inTask.m_inDependencies[j];
				ANKI_ASSERT(dep);
				Task* depTask = static_cast<Task*>(dep);

				if(!depTask->done())
				{
					outTask.m_deps[outTask.m_depCount++] = depTask;
					depTask->m_othersDepend = true;
				}
			}

			// Push to the list
			ANKI_HIVE_DEBUG_PRINT(
				"pushing back %p (udata %p)\n", static_cast<void*>(&outTask), static_cast<void*>(outTask.m_arg));
			if(m_head != nullptr)
			{
				ANKI_ASSERT(m_tail && m_head);
				m_tail->m_next = &outTask;
				m_tail = &outTask;
			}
			else
			{
				ANKI_ASSERT(m_tail == nullptr);
				m_head = &outTask;
				m_tail = m_head;
			}
		}

		m_pendingTasks += taskCount;

		ANKI_HIVE_DEBUG_PRINT("submit tasks\n");
		// Notify all threads
		m_cvar.notifyAll();
	}

	// Set the out dependencies
	for(U i = 0; i < taskCount; ++i)
	{
		tasks[i].m_outDependency = static_cast<ThreadHiveDependencyHandle>(&htasks[i]);
	}
}

void ThreadHive::threadRun(U threadId)
{
	Task* task = nullptr;

	while(!waitForWork(threadId, task))
	{
		// Run the task
		ANKI_ASSERT(task && task->m_cb);
		ANKI_HIVE_DEBUG_PRINT(
			"tid: %lu will exec %p (udata: %p)\n", threadId, static_cast<void*>(task), static_cast<void*>(task->m_arg));
		task->m_cb(task->m_arg, threadId, *this);
	}

	ANKI_HIVE_DEBUG_PRINT("tid: %lu thread quits!\n", threadId);
}

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

ThreadHive::Task* ThreadHive::getNewTask()
{
	Task* prevTask = nullptr;
	Task* task = m_head;
	while(task)
	{
		ANKI_ASSERT(!task->done());

		// Check if there are dependencies
		Bool allDepsCompleted = true;
		for(U j = 0; j < task->m_depCount; ++j)
		{
			Task* depTask = task->m_deps[j];

			if(!depTask->done())
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
	m_allocatedTasks = 0;
	m_allocatedDeps = 0;
	m_alloc.getMemoryPool().reset();

	ANKI_HIVE_DEBUG_PRINT("mt: done waiting all\n");
}

} // end namespace anki
