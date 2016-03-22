// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Thread.h>
#include <anki/util/DArray.h>
#include <anki/util/Allocator.h>

namespace anki
{

// Forward
class ThreadHive;
class ThreadHiveThread;

/// @addtogroup util_thread
/// @{

/// Opaque handle that defines a ThreadHive depedency.
/// @memberof ThreadHive
using ThreadHiveDependencyHandle = U16;

/// The callback that defines a ThreadHibe task.
/// @memberof ThreadHive
using ThreadHiveTaskCallback = void (*)(void*, U32 threadId, ThreadHive& hive);

/// Task for the ThreadHive.
/// @memberof ThreadHive
class ThreadHiveTask
{
public:
	/// What this task will do.
	ThreadHiveTaskCallback m_callback ANKI_DBG_NULLIFY_PTR;

	/// Arguments to pass to the m_callback.
	void* m_argument ANKI_DBG_NULLIFY_PTR;

	/// The tasks that this task will depend on.
	WArray<ThreadHiveDependencyHandle> m_inDependencies;

	/// Will be filled after the submission of the task. Can be used to set
	/// dependencies to future tasks.
	ThreadHiveDependencyHandle m_outDependency;
};

/// A scheduler of small tasks. It takes a number of tasks and schedules them in 
/// one of the threads. The tasks can depend on previously submitted tasks or be
/// completely independent.
class ThreadHive : public NonCopyable
{
	friend class ThreadHiveThread;

public:
	/// Create the hive.
	ThreadHive(U threadCount, GenericMemoryPoolAllocator<U8> alloc);

	~ThreadHive();

	/// Submit tasks. The ThreadHiveTaskCallback callbacks can also call this.
	void submitTasks(ThreadHiveTask* tasks, U taskCount);

	/// Submit a single task without dependencies. The ThreadHiveTaskCallback
	/// callbacks can also call this.
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
	static const U MAX_DEPS = 2;

	/// Lightweight task.
	class Task
	{
	public:
		Task* m_next; ///< Next in the list.

		ThreadHiveTaskCallback m_cb; ///< Callback that defines the task.
		void* m_arg; ///< Args for the callback.

		U16 m_depCount;
		Array<ThreadHiveDependencyHandle, MAX_DEPS> m_deps;
		Bool8 m_othersDepend; ///< Other tasks depend on this one.

		Bool done() const
		{
			return m_cb == nullptr;
		}
	};

	static_assert(sizeof(Task) == (sizeof(void*) * 3 + 8), "Wrong size");

	GenericMemoryPoolAllocator<U8> m_alloc;
	ThreadHiveThread* m_threads = nullptr;
	U32 m_threadCount = 0;

	DArray<Task> m_storage; ///< Task storage.
	Task* m_head = nullptr; ///< Head of the task list.
	Task* m_tail = nullptr; ///< Tail of the task list.
	Bool m_quit = false;
	U32 m_pendingTasks = 0;
	U32 m_allocatedTasks = 0;

	Mutex m_mtx;
	ConditionVariable m_cvar;

	void threadRun(U threadId);

	/// Wait for more tasks.
	Bool waitForWork(U threadId, Task*& task);

	/// Get new work from the queue.
	Task* getNewTask();

	/// Complete a task.
	void completeTask(U taskId);
};
/// @}

} // end namespace anki
