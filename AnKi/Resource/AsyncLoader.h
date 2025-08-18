// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/List.h>

namespace anki {

// Forward
class AsyncLoader;

/// @addtogroup resource
/// @{

class AsyncLoaderTaskContext
{
public:
	/// Resubmit the same task at the end of the queue.
	Bool m_resubmitTask = false;
};

/// Interface for tasks for the AsyncLoader.
class AsyncLoaderTask : public IntrusiveListEnabled<AsyncLoaderTask>
{
public:
	virtual ~AsyncLoaderTask()
	{
	}

	virtual Error operator()(AsyncLoaderTaskContext& ctx) = 0;
};

/// Asynchronous resource loader.
class AsyncLoader : public MakeSingleton<AsyncLoader>
{
public:
	AsyncLoader();

	~AsyncLoader();

	/// Submit a task.
	void submitTask(AsyncLoaderTask* task);

	/// Create a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	TTask* newTask(TArgs&&... args)
	{
		return newInstance<TTask>(ResourceMemoryPool::getSingleton(), std::forward<TArgs>(args)...);
	}

	/// Create and submit a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	void submitNewTask(TArgs&&... args)
	{
		submitTask(newTask<TTask>(std::forward<TArgs>(args)...));
	}

	/// Get the total number of completed tasks.
	U32 getTasksInFlightCount() const
	{
		return m_tasksInFlightCount.load();
	}

private:
	Thread m_thread;
	Barrier m_barrier = {2};

	Mutex m_mtx;
	ConditionVariable m_condVar;
	IntrusiveList<AsyncLoaderTask> m_taskQueue;
	Bool m_quit = false;

	Atomic<U32> m_tasksInFlightCount = {0};

	/// Thread callback
	static Error threadCallback(ThreadCallbackInfo& info);

	Error threadWorker();

	void stop();
};
/// @}

} // end namespace anki
