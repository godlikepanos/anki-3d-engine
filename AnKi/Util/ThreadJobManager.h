// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Function.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Parallel task dispatcher. You feed it with tasks and sends them for execution in parallel and then waits for all to finish.
class ThreadJobManager
{
public:
	using Func = Function<void(U32 threadId)>;

	ThreadJobManager(U32 threadCount, Bool pinToCores = false, U32 queueSize = 256);

	ThreadJobManager(const ThreadJobManager&) = delete; // Non-copyable

	~ThreadJobManager();

	ThreadJobManager& operator=(const ThreadJobManager&) = delete; // Non-copyable

	// Assign a task to a working thread
	void dispatchTask(const Func& func)
	{
		while(!pushBackTask(func))
		{
			std::this_thread::yield();
		}

		m_cvar.notifyOne();
	}

	// Wait for all tasks to finish.
	void waitForAllTasksToFinish()
	{
		while(m_activeTaskCount.load() > 0)
		{
			std::this_thread::yield();
		}
	}

	U32 getThreadCount() const
	{
		return m_threads.getSize();
	}

private:
	class alignas(ANKI_CACHE_LINE_SIZE) WorkerThread
	{
	public:
		U32 m_id;
		Thread m_thread;
		ThreadJobManager* m_manager;

		WorkerThread(ThreadJobManager* manager, U32 id, Bool pinToCore, CString threadName);

		static Error threadCallback(ThreadCallbackInfo& info);
	};

	WeakArray<WorkerThread> m_threads;

	DynamicArray<Func> m_taskQueue;
	U32 m_tasksFront = 0;
	U32 m_tasksBack = 0;
	Atomic<U32> m_activeTaskCount = {0};

	ConditionVariable m_cvar;
	Mutex m_mtx;

	Bool m_quit = false;

	Bool pushBackTask(const Func& func);
	Bool popFrontTask(Func& func);

	void threadRun(U32 threadId);
};

} // end namespace anki
