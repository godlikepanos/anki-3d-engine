// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Thread.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/MemoryPool.h>

namespace anki {

// Forward
class ThreadHive;

/// @addtogroup util_thread
/// @{

/// Opaque handle that defines a ThreadHive depedency. @memberof ThreadHive
class ThreadHiveSemaphore
{
	friend class ThreadHive;

public:
	/// Increase the value of the semaphore. It's easy to brake things with that.
	/// @note It's thread-safe.
	void increaseSemaphore(U32 increase)
	{
		m_atomic.fetchAdd(increase);
	}

private:
	Atomic<U32> m_atomic;

	// No need to construct it or delete it
	ThreadHiveSemaphore() = delete;
	~ThreadHiveSemaphore() = delete;
};

/// The callback that defines a ThreadHibe task.
/// @memberof ThreadHive
using ThreadHiveTaskCallback = void (*)(void* userData, U32 threadId, ThreadHive& hive,
										ThreadHiveSemaphore* signalSemaphore);

/// Task for the ThreadHive. @memberof ThreadHive
class ThreadHiveTask
{
public:
	/// What this task will do.
	ThreadHiveTaskCallback m_callback ANKI_DEBUG_CODE(= nullptr);

	/// Arguments to pass to the m_callback.
	void* m_argument ANKI_DEBUG_CODE(= nullptr);

	/// The task will start when that semaphore reaches zero.
	ThreadHiveSemaphore* m_waitSemaphore = nullptr;

	/// When the task is completed that semaphore will be decremented by one. Can be used to set dependencies to future
	/// tasks.
	ThreadHiveSemaphore* m_signalSemaphore = nullptr;
};

/// Initialize a ThreadHiveTask.
#define ANKI_THREAD_HIVE_TASK(callback_, argument_, waitSemaphore_, signalSemaphore_) \
	{ \
		[](void* ud, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive, \
		   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) { \
			[[maybe_unused]] auto self = static_cast<decltype(argument_)>(ud); \
			callback_ \
		}, \
			argument_, waitSemaphore_, signalSemaphore_ \
	}

/// A scheduler of small tasks. It takes a number of tasks and schedules them in one of the threads. The tasks can
/// depend on previously submitted tasks or be completely independent.
class ThreadHive
{
public:
	static constexpr U32 kMaxThreads = 32;

	/// Create the hive.
	ThreadHive(U32 threadCount, BaseMemoryPool* pool, Bool pinToCores = false);

	ThreadHive(const ThreadHive&) = delete; // Non-copyable

	~ThreadHive();

	ThreadHive& operator=(const ThreadHive&) = delete; // Non-copyable

	U32 getThreadCount() const
	{
		return m_threadCount;
	}

	/// Create a new semaphore with some initial value.
	/// @param initialValue Can't be zero.
	ThreadHiveSemaphore* newSemaphore(const U32 initialValue)
	{
		ANKI_ASSERT(initialValue > 0);
		ThreadHiveSemaphore* sem = static_cast<ThreadHiveSemaphore*>(
			m_pool.allocate(sizeof(ThreadHiveSemaphore), alignof(ThreadHiveSemaphore)));
		sem->m_atomic.setNonAtomically(initialValue);
		return sem;
	}

	/// Allocate some scratch memory. The memory becomes invalid after waitAllTasks() is called.
	void* allocateScratchMemory(PtrSize size, U32 alignment)
	{
		ANKI_ASSERT(size > 0 && alignment > 0);
		void* out = m_pool.allocate(size, alignment);
#if ANKI_ENABLE_ASSERTIONS
		memset(out, 0, size);
#endif
		return out;
	}

	/// Submit tasks. The ThreadHiveTaskCallback callbacks can also call this.
	void submitTasks(ThreadHiveTask* tasks, const U32 taskCount);

	/// Submit a single task without dependencies. The ThreadHiveTaskCallback callbacks can also call this.
	void submitTask(ThreadHiveTaskCallback callback, void* arg)
	{
		ThreadHiveTask task;
		task.m_callback = callback;
		task.m_argument = arg;
		submitTasks(&task, 1);
	}

	/// Wait for all tasks to finish. Will block.
	void waitAllTasks();

private:
	class Thread;

	/// Lightweight task.
	class Task;

	BaseMemoryPool* m_slowPool;
	StackMemoryPool m_pool;
	Thread* m_threads = nullptr;
	U32 m_threadCount = 0;

	Task* m_head = nullptr; ///< Head of the task list.
	Task* m_tail = nullptr; ///< Tail of the task list.
	Bool m_quit = false;
	U32 m_pendingTasks = 0;

	Mutex m_mtx;
	ConditionVariable m_cvar;

	static Atomic<U32> m_uuid;

	void threadRun(U32 threadId);

	/// Wait for more tasks.
	Bool waitForWork(U32 threadId, Task*& task);

	/// Get new work from the queue.
	Task* getNewTask();
};
/// @}

} // end namespace anki
