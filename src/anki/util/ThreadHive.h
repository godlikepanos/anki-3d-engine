// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Thread.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/Allocator.h>

namespace anki
{

// Forward
class ThreadHive;

/// @addtogroup util_thread
/// @{

/// Opaque handle that defines a ThreadHive depedency. @memberof ThreadHive
using ThreadHiveDependencyHandle = void*;

/// The callback that defines a ThreadHibe task.
/// @memberof ThreadHive
using ThreadHiveTaskCallback = void (*)(void*, U32 threadId, ThreadHive& hive);

/// Task for the ThreadHive. @memberof ThreadHive
class ThreadHiveTask
{
public:
	/// What this task will do.
	ThreadHiveTaskCallback m_callback ANKI_DBG_NULLIFY;

	/// Arguments to pass to the m_callback.
	void* m_argument ANKI_DBG_NULLIFY;

	/// The tasks that this task will depend on.
	WeakArray<ThreadHiveDependencyHandle> m_inDependencies;

	/// Will be filled after the submission of the task. Can be used to set dependencies to future tasks.
	ThreadHiveDependencyHandle m_outDependency;
};

/// A scheduler of small tasks. It takes a number of tasks and schedules them in one of the threads. The tasks can
/// depend on previously submitted tasks or be completely independent.
class ThreadHive : public NonCopyable
{
public:
	static const U MAX_THREADS = 32;

	/// Create the hive.
	ThreadHive(U threadCount, GenericMemoryPoolAllocator<U8> alloc);

	~ThreadHive();

	U getThreadCount() const
	{
		return m_threadCount;
	}

	/// Submit tasks. The ThreadHiveTaskCallback callbacks can also call this.
	void submitTasks(ThreadHiveTask* tasks, const U taskCount);

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

	GenericMemoryPoolAllocator<U8> m_slowAlloc;
	StackAllocator<U8> m_alloc;
	Thread* m_threads = nullptr;
	U32 m_threadCount = 0;

	Task* m_head = nullptr; ///< Head of the task list.
	Task* m_tail = nullptr; ///< Tail of the task list.
	Bool m_quit = false;
	U32 m_pendingTasks = 0;
	U32 m_allocatedTasks = 0;
	U32 m_allocatedDeps = 0;

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
