// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/util/Thread.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class AsyncLoader;

/// @addtogroup resource
/// @{

class AsyncLoaderTaskContext
{
public:
	/// Pause the async loader.
	Bool m_pause = false;

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

	virtual ANKI_USE_RESULT Error operator()(AsyncLoaderTaskContext& ctx) = 0;
};

/// Asynchronous resource loader.
class AsyncLoader
{
public:
	AsyncLoader();

	~AsyncLoader();

	void init(const HeapAllocator<U8>& alloc);

	/// Submit a task.
	void submitTask(AsyncLoaderTask* task);

	/// Create a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	TTask* newTask(TArgs&&... args)
	{
		return m_alloc.template newInstance<TTask>(std::forward<TArgs>(args)...);
	}

	/// Create and submit a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	void submitNewTask(TArgs&&... args)
	{
		submitTask(newTask<TTask>(std::forward<TArgs>(args)...));
	}

	/// Pause the loader. This method will block the caller for the current async task to finish. The rest of the
	/// tasks in the queue will not be executed until resume is called.
	void pause();

	/// Resume the async loading.
	void resume();

	HeapAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	/// Get the total number of completed tasks.
	U64 getCompletedTaskCount() const
	{
		return m_completedTaskCount.load();
	}

private:
	HeapAllocator<U8> m_alloc;
	Thread m_thread;
	Barrier m_barrier = {2};

	Mutex m_mtx;
	ConditionVariable m_condVar;
	IntrusiveList<AsyncLoaderTask> m_taskQueue;
	Bool m_quit = false;
	Bool m_paused = false;
	Bool m_sync = false;

	Atomic<U64> m_completedTaskCount = {0};

	/// Thread callback
	static ANKI_USE_RESULT Error threadCallback(ThreadCallbackInfo& info);

	Error threadWorker();

	void stop();
};
/// @}

} // end namespace anki
