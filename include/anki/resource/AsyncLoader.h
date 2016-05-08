// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	/// Create a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	void newTask(TArgs&&... args);

	/// Pause the loader. This method will block the main thread for the current
	/// async task to finish. The rest of the tasks in the queue will not be
	/// executed until resume is called.
	void pause();

	/// Resume the async loading.
	void resume();

private:
	HeapAllocator<U8> m_alloc;
	Thread m_thread;
	Barrier m_barrier = {2};

	Mutex m_mtx;
	ConditionVariable m_condVar;
	IntrusiveList<AsyncLoaderTask> m_taskQueue;
	Bool8 m_quit = false;
	Bool8 m_paused = false;
	Bool8 m_sync = false;

	/// Thread callback
	static ANKI_USE_RESULT Error threadCallback(Thread::Info& info);

	Error threadWorker();

	void stop();
};

//==============================================================================
template<typename TTask, typename... TArgs>
inline void AsyncLoader::newTask(TArgs&&... args)
{
	TTask* newTask =
		m_alloc.template newInstance<TTask>(std::forward<TArgs>(args)...);

	// Append task to the list
	{
		LockGuard<Mutex> lock(m_mtx);
		m_taskQueue.pushBack(newTask);

		if(!m_paused)
		{
			// Wake up the thread if it's not paused
			m_condVar.notifyOne();
		}
	}
}

/// @}

} // end namespace anki
