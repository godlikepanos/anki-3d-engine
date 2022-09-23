// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Thread.h>

namespace anki {

// Forward
namespace detail {
class ThreadPoolThread;
}

/// @addtogroup util_thread
/// @{

/// A task assignment for a ThreadPool
/// @memberof ThreadPool
class ThreadPoolTask
{
public:
	virtual ~ThreadPoolTask()
	{
	}

	virtual Error operator()(U32 taskId, PtrSize threadsCount) = 0;
};

/// Parallel task dispatcher. You feed it with tasks and sends them for execution in parallel and then waits for all to
/// finish.
class ThreadPool
{
	friend class detail::ThreadPoolThread;

public:
	static constexpr U kMaxThreads = 32; ///< An absolute limit

	/// Constructor.
	ThreadPool(U32 threadCount, Bool pinToCores = false);

	ThreadPool(const ThreadPool&) = delete; // Non-copyable

	~ThreadPool();

	ThreadPool& operator=(const ThreadPool&) = delete; // Non-copyable

	/// Assign a task to a working thread
	/// @param slot The slot of the task
	/// @param task The task. If it's nullptr then a dummy task will be assigned
	void assignNewTask(U32 slot, ThreadPoolTask* task);

	/// Wait for all tasks to finish.
	/// @return The error code in one of the worker threads.
	Error waitForAllThreadsToFinish()
	{
		m_barrier.wait();
		m_tasksAssigned = 0;
		Error err = m_err;
		m_err = Error::kNone;
		return err;
	}

	/// @return The number of threads in the ThreadPool.
	PtrSize getThreadCount() const
	{
		return m_threadsCount;
	}

private:
	/// A dummy task for a ThreadPool
	class DummyTask : public ThreadPoolTask
	{
	public:
		Error operator()([[maybe_unused]] U32 taskId, [[maybe_unused]] PtrSize threadsCount)
		{
			return Error::kNone;
		}
	};

	Barrier m_barrier; ///< Synchronization barrier
	detail::ThreadPoolThread* m_threads = nullptr; ///< Threads array
	U32 m_tasksAssigned = 0;
	U32 m_threadsCount = 0;
	Error m_err = Error::kNone;
	static DummyTask m_dummyTask;
};
/// @}

} // end namespace anki
