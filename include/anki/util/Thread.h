// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_THREAD_H
#define ANKI_UTIL_THREAD_H

#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/util/NonCopyable.h"
#include <atomic>
#include <mutex> // XXX
#include <thread> // XXX
#include <condition_variable>

#define ANKI_DISABLE_THREADPOOL_THREADING 0

namespace anki {

// Forward
class Threadpool;

/// @addtogroup util_thread
/// @{

/// Thread implementation
class Thread: public NonCopyable
{
public:
	/// It holds some information to be passed to the thread's callback
	class Info
	{
	public:
		void* m_userData;
		const char* m_threadName;
	};

	/// The type of the tread callback
	using Callback = I(*)(Info&);

	/// Create a thread with or without a name
	/// @param[in] name The name of the new thread. Can be nullptr
	Thread(const char* name);

	~Thread();

	/// Start the thread
	/// @param userData The user data of the thread callback
	/// @param callback The thread callback that will be executed
	void start(void* userData, Callback callback);

	/// Wait for the thread to finish
	/// @return The error code of the thread's callback
	I join();

	/// Identify the current thread
	static U64 getCurrentThreadId();

private:
	static constexpr U ALIGNMENT = 8;
	alignas(ALIGNMENT) Array<PtrSize, 1> m_impl; ///< The system native type
	Array<char, 32> m_name; ///< The name of the thread
	Callback m_callback = nullptr; ///< The callback
	void* m_userData = nullptr; ///< The user date to pass to the callback

	/// Pthreads specific function
	static void* pthreadCallback(void* ud);

	Bool initialized() const
	{
		return m_impl[0] != 0;
	}
};

/// Mutex
class Mutex: public NonCopyable
{
	friend class ConditionVariable;

public:
	Mutex();

	~Mutex();

	/// Lock
	void lock();

	/// Try lock
	/// @return True if it was locked successfully
	Bool tryLock();

	/// Unlock
	void unlock();

private:
	static constexpr U ALIGNMENT = 8;
	alignas(ALIGNMENT) Array<PtrSize, 10> m_impl; ///< The system native type
};

/// Condition variable
class ConditionVariable: public NonCopyable
{
public:
	ConditionVariable();

	~ConditionVariable();

	/// Signal one thread
	void notifyOne();

	/// Signal all threads
	void notifyAll();

	/// Bock until signaled
	void wait(Mutex& mtx);

private:
	static constexpr U ALIGNMENT = 16;
	alignas(ALIGNMENT) Array<PtrSize, 12> m_impl; ///< The system native type
};

/// Spin lock. Good if the critical section will be executed in a short period
/// of time
class SpinLock: public NonCopyable
{
public:
	/// Lock 
	void lock()
	{
		while(m_lock.test_and_set(std::memory_order_acquire))
		{}
	}

	/// Unlock
	void unlock()
	{
		m_lock.clear(std::memory_order_release);
	}

private:
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;	
};

/// A barrier for thread synchronization. It works just like boost::barrier
class Barrier: public NonCopyable
{
public:
	Barrier(U32 count)
	:	m_threshold(count), 
		m_count(count), 
		m_generation(0)
	{
		ANKI_ASSERT(count != 0);
	}

	~Barrier() = default;

	/// TODO
	Bool wait();

private:
	Mutex m_mtx;
	ConditionVariable m_cond;
	U32 m_threshold;
	U32 m_count;
	U32 m_generation;
};

// Forward
namespace detail {
class ThreadpoolThread;
}

/// Parallel task dispatcher. You feed it with tasks and sends them for
/// execution in parallel and then waits for all to finish
class Threadpool: public NonCopyable
{
	friend class detail::ThreadpoolThread;

public:
	static constexpr U MAX_THREADS = 32; ///< An absolute limit

	/// A task assignment for a Threadpool
	class Task
	{
	public:
		virtual ~Task()
		{}

		virtual void operator()(U32 taskId, PtrSize threadsCount) = 0;

		/// Chose a starting and end index
		static void choseStartEnd(U32 taskId, PtrSize threadsCount, 
			PtrSize elementsCount, PtrSize& start, PtrSize& end)
		{
			start = taskId * (elementsCount / threadsCount);
			end = (taskId == threadsCount - 1)
				? elementsCount
				: start + elementsCount / threadsCount;
		}
	};

	/// Constructor 
	Threadpool(U32 threadsCount);

	~Threadpool();

	/// Assign a task to a working thread
	/// @param slot The slot of the task
	/// @param task The task. If it's nullptr then a dummy task will be assigned
	void assignNewTask(U32 slot, Task* task);

	/// Wait for all tasks to finish
	void waitForAllThreadsToFinish()
	{
#if !ANKI_DISABLE_THREADPOOL_THREADING
		m_barrier.wait();
#endif
	}

	PtrSize getThreadsCount() const
	{
		return m_threadsCount;
	}

private:
	/// A dummy task for a Threadpool
	class DummyTask: public Task
	{
	public:
		void operator()(U32 taskId, PtrSize threadsCount)
		{
			(void)taskId;
			(void)threadsCount;
		}
	};

#if !ANKI_DISABLE_THREADPOOL_THREADING
	Barrier m_barrier; ///< Synchronization barrier
	detail::ThreadpoolThread** m_threads = nullptr; ///< Threads array
#endif
	U8 m_threadsCount = 0;
	static DummyTask m_dummyTask;
};

/// @}

} // end namespace anki

#endif

