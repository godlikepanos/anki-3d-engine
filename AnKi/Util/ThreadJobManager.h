// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Function.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

/// @addtogroup util_thread
/// @{

/// Parallel task dispatcher. You feed it with tasks and sends them for execution in parallel and then waits for all to finish.
class ThreadJobManager
{
public:
	using Func = Function<void(U32 threadId)>;

	/// Constructor.
	ThreadJobManager(U32 threadCount, Bool pinToCores = false, U32 queueSize = 256);

	ThreadJobManager(const ThreadJobManager&) = delete; // Non-copyable

	~ThreadJobManager();

	ThreadJobManager& operator=(const ThreadJobManager&) = delete; // Non-copyable

	/// Assign a task to a working thread
	void dispatchTask(const Func& func)
	{
		m_tasksInFlightCount.fetchAdd(1);

		while(!pushBackTask(func))
		{
			m_cvar.notifyOne();
			std::this_thread::yield();
		}

		m_cvar.notifyOne();
	}

	/// Wait for all tasks to finish.
	void waitForAllTasksToFinish()
	{
		while(m_tasksInFlightCount.load() != 0)
		{
			m_cvar.notifyOne();
			std::this_thread::yield();
		}
	}

	U32 getThreadCount() const
	{
		return m_threads.getSize();
	}

private:
	class WorkerThread;

	DynamicArray<WorkerThread*> m_threads;

	DynamicArray<Func> m_tasks;
	U32 m_tasksFront = 0;
	U32 m_tasksBack = 0;
	Mutex m_tasksMtx;

	Atomic<U32> m_tasksInFlightCount = {0};

	ConditionVariable m_cvar;
	Mutex m_mtx;

	Bool m_quit = false;

	Bool pushBackTask(const Func& func);
	Bool popFrontTask(Func& func, Bool& quit);

	void threadRun(U32 threadId);
};
/// @}

} // end namespace anki
