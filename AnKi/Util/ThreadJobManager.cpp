// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/ThreadJobManager.h>
#include <AnKi/Util/String.h>

namespace anki {

ThreadJobManager::WorkerThread::WorkerThread(ThreadJobManager* manager, U32 id, Bool pinToCore, CString threadName)
	: m_id(id)
	, m_thread(threadName.cstr())
	, m_manager(manager)
{
	m_thread.start(this, threadCallback, ThreadCoreAffinityMask(false).set(m_id, pinToCore));
}

Error ThreadJobManager::WorkerThread::threadCallback(ThreadCallbackInfo& info)
{
	WorkerThread& self = *static_cast<WorkerThread*>(info.m_userData);
	self.m_manager->threadRun(self.m_id);
	return Error::kNone;
}

ThreadJobManager::ThreadJobManager(U32 threadCount, Bool pinToCores, U32 queueSize)
{
	ANKI_ASSERT(threadCount);

	m_taskQueue.resize(queueSize);

	void* mem = DefaultMemoryPool::getSingleton().allocate(threadCount * sizeof(WorkerThread), alignof(WorkerThread));
	m_threads = WeakArray(static_cast<WorkerThread*>(mem), threadCount);

	for(U32 i = 0; i < threadCount; ++i)
	{
		String tname;
		tname.sprintf("AnKiJobManager#%u", i);
		callConstructor(m_threads[i], this, i, pinToCores, tname);
	}
}

ThreadJobManager::~ThreadJobManager()
{
	// Teardown will not execute in-flight tasks
	{
		LockGuard lock(m_mtx);
		m_quit = true;
	}

	m_cvar.notifyAll();

	for(WorkerThread& thread : m_threads)
	{
		[[maybe_unused]] const Error err = thread.m_thread.join();
	}

	for(WorkerThread& thread : m_threads)
	{
		callDestructor(thread);
	}

	DefaultMemoryPool::getSingleton().free(m_threads.getBegin());
}

Bool ThreadJobManager::pushBackTask(const Func& func)
{
	LockGuard lock(m_mtx);
	const U32 next = (m_tasksBack + 1) % m_taskQueue.getSize();
	if(next != m_tasksFront)
	{
		m_taskQueue[m_tasksBack] = func;
		m_tasksBack = next;
		m_activeTaskCount.fetchAdd(1);
		return true;
	}

	return false;
}

Bool ThreadJobManager::popFrontTask(Func& func)
{
	LockGuard lock(m_mtx);

	while(!m_quit && m_tasksBack == m_tasksFront)
	{
		// No tasks, wait
		m_cvar.wait(m_mtx);
	}

	if(!m_quit)
	{
		func = std::move(m_taskQueue[m_tasksFront]);
		m_tasksFront = (m_tasksFront + 1) % m_taskQueue.getSize();
	}

	return m_quit;
}

void ThreadJobManager::threadRun(U32 threadId)
{
	while(true)
	{
		Func func;
		const Bool quit = popFrontTask(func);

		if(quit)
		{
			break;
		}
		else
		{
			func(threadId);

			[[maybe_unused]] const U32 count = m_activeTaskCount.fetchSub(1);
			ANKI_ASSERT(count > 0);
		}
	}
}

} // end namespace anki
