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

using ThreadHiveDependencyHandle = U16;

using ThreadHiveTaskCallback = void (*)(void*, U32 threadId, ThreadHive& hive);

/// Task for the ThreadHive.
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

/// A scheduler of small tasks. It takes tasks to be executed and schedules them
/// in one of the threads.
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
	static const U MAX_DEPS = 4;

	/// Lightweight task.
	class Task
	{
	public:
		ThreadHiveTaskCallback m_cb;
		void* m_arg;

		union
		{
			Array<ThreadHiveDependencyHandle, MAX_DEPS> m_deps;
			U64 m_depsU64;
		};

		Bool done() const
		{
			return m_cb == nullptr;
		}
	};

	static_assert(sizeof(Task) == sizeof(void*) * 2 + 8, "Too big");

	GenericMemoryPoolAllocator<U8> m_alloc;
	ThreadHiveThread* m_threads = nullptr;
	U32 m_threadCount = 0;

	DArray<Task> m_queue; ///< Task queue.
	I32 m_head = 0; ///< Head of m_queue.
	I32 m_tail = -1; ///< Tail of m_queue.
	U64 m_workingThreadsMask = 0; ///< Mask with the threads that have work.
	Bool m_quit = false;
	U64 m_waitingThreadsMask = 0;

	Mutex m_mtx; ///< Protect the queue
	ConditionVariable m_cvar;

	Bool m_mainThreadStopWaiting = false;
	Mutex m_mainThreadMtx;
	ConditionVariable m_mainThreadCvar;

	void run(U threadId);

	/// Get new work from the queue.
	ThreadHiveTaskCallback getNewWork(void*& arg);
};
/// @}

} // end namespace anki
