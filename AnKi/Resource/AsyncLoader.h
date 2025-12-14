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

/// @memberof AsyncLoader
enum class AsyncLoaderPriority : U8
{
	kHigh,
	kMedium,
	kLow,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(AsyncLoaderPriority)

/// @memberof AsyncLoader
class AsyncLoaderTaskContext
{
public:
	Bool m_resubmitTask = false; ///< Resubmit the same task at the end of the queue.
	AsyncLoaderPriority m_priority = AsyncLoaderPriority::kCount;
};

/// Interface for tasks for the AsyncLoader.
/// @memberof AsyncLoader
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

	/// Create a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	TTask* newTask(TArgs&&... args)
	{
		return newInstance<TTask>(ResourceMemoryPool::getSingleton(), std::forward<TArgs>(args)...);
	}

	/// Submit a task.
	void submitTask(AsyncLoaderTask* task, AsyncLoaderPriority priority);

	/// Get the total number of completed tasks.
	U32 getTasksInFlightCount() const
	{
		return m_tasksInFlightCount.load();
	}

private:
	Thread m_thread;

	Mutex m_mtx;
	ConditionVariable m_condVar;
	Array<IntrusiveList<AsyncLoaderTask>, U32(AsyncLoaderPriority::kCount)> m_taskQueues;
	Bool m_quit = false;

	Atomic<U32> m_tasksInFlightCount = {0};

	/// Thread callback
	static Error threadCallback(ThreadCallbackInfo& info);

	Error threadWorker();

	void stop();
};
/// @}

} // end namespace anki
