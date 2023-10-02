// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/ThreadJobManager.h>
#include <AnKi/Util/String.h>

namespace anki {

class ThreadJobManager::WorkerThread
{
public:
	U32 m_id;
	Thread m_thread;
	ThreadJobManager* m_manager;

	WorkerThread(ThreadJobManager* manager, U32 id, Bool pinToCore, CString threadName)
		: m_id(id)
		, m_thread(threadName.cstr())
		, m_manager(manager)
	{
		m_thread.start(this, threadCallback, ThreadCoreAffinityMask(false).set(m_id, pinToCore));
	}

	static Error threadCallback(ThreadCallbackInfo& info)
	{
		WorkerThread& self = *static_cast<WorkerThread*>(info.m_userData);
		self.m_manager->threadRun(self.m_id);
		return Error::kNone;
	}
};

ThreadJobManager::ThreadJobManager(U32 threadCount, Bool pinToCores, U32 queueSize)
{
	ANKI_ASSERT(threadCount);
	m_threads.resize(threadCount);
	for(U32 i = 0; i < threadCount; ++i)
	{
		String threadName;
		threadName.sprintf("JobManager#%u", i);
		m_threads[i] = newInstance<WorkerThread>(DefaultMemoryPool::getSingleton(), this, i, pinToCores, threadName);
	}

	m_tasks.resize(queueSize);
}

ThreadJobManager::~ThreadJobManager()
{
	{
		LockGuard lock(m_tasksMtx);
		m_quit = true;
	}

	m_cvar.notifyAll();

	for(WorkerThread* thread : m_threads)
	{
		[[maybe_unused]] const Error err = thread->m_thread.join();
		deleteInstance(DefaultMemoryPool::getSingleton(), thread);
	}
}

Bool ThreadJobManager::pushBackTask(const Func& func)
{
	LockGuard lock(m_tasksMtx);
	const U32 next = (m_tasksBack + 1) % m_tasks.getSize();
	if(next != m_tasksFront)
	{
		m_tasks[m_tasksBack] = func;
		m_tasksBack = next;
		return true;
	}

	return false;
}

Bool ThreadJobManager::popFrontTask(Func& func, Bool& quit)
{
	LockGuard lock(m_tasksMtx);
	quit = m_quit;

	if(quit) [[unlikely]]
	{
		return true;
	}

	if(m_tasksBack != m_tasksFront)
	{
		func = m_tasks[m_tasksFront];
		m_tasksFront = (m_tasksFront + 1) % m_tasks.getSize();
		return true;
	}

	return false;
}

void ThreadJobManager::threadRun(U32 threadId)
{
	while(true)
	{
		Bool quit;
		Func func;
		if(popFrontTask(func, quit))
		{
			if(quit) [[unlikely]]
			{
				break;
			}

			func(threadId);
			[[maybe_unused]] const U32 count = m_tasksInFlightCount.fetchSub(1);
			ANKI_ASSERT(count > 0);
		}
		else
		{
			LockGuard lock(m_mtx);
			m_cvar.wait(m_mtx);
		}
	}
}

} // end namespace anki
