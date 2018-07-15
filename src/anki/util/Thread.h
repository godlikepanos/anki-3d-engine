// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Array.h>
#include <anki/util/NonCopyable.h>
#include <atomic>

namespace anki
{

/// @addtogroup util_thread
/// @{

/// The thread ID.
using ThreadId = U64;

/// It holds some information to be passed to the thread's callback
class ThreadCallbackInfo
{
public:
	void* m_userData;
	const char* m_threadName;
};

/// The type of the tread callback
using ThreadCallback = Error (*)(ThreadCallbackInfo&);

/// Thread implementation
class Thread : public NonCopyable
{
public:
	/// Create a thread with or without a name
	/// @param[in] name The name of the new thread. Can be nullptr.
	Thread(const char* name);

	~Thread();

	/// Start the thread.
	/// @param userData The user data of the thread callback
	/// @param callback The thread callback that will be executed
	/// @param pinToCore Pin the thread to a core.
	void start(void* userData, ThreadCallback callback, I pinToCore = -1);

	/// Wait for the thread to finish
	/// @return The error code of the thread's callback
	ANKI_USE_RESULT Error join();

	/// Identify the current thread
	static ThreadId getCurrentThreadId();

anki_internal:
	const char* getName() const
	{
		return &m_name[0];
	}

	void* getUserData() const
	{
		return m_userData;
	}

	ThreadCallback getCallback() const
	{
		return m_callback;
	}

private:
	void* m_impl = nullptr; ///< The system native type
	Array<char, 32> m_name; ///< The name of the thread
	ThreadCallback m_callback = nullptr; ///< The callback
	void* m_userData = nullptr; ///< The user date to pass to the callback

#if ANKI_EXTRA_CHECKS
	Bool8 m_started = false;
#endif
};

/// Mutual exclusion primitive.
class Mutex : public NonCopyable
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
	void* m_impl = nullptr; ///< The system native type
};

/// Condition variable.
class ConditionVariable : public NonCopyable
{
public:
	ConditionVariable();

	~ConditionVariable();

	/// Signal one thread
	void notifyOne();

	/// Signal all threads
	void notifyAll();

	/// Bock until signaled.
	/// @param mtx The mutex.
	void wait(Mutex& mtx);

private:
	void* m_impl = nullptr; ///< The system native type
};

/// Mutual exclusion primitive. Like Mutex. It's better than Mutex only if the critical section will be executed in a
/// very short period of time.
class SpinLock : public NonCopyable
{
public:
	/// Lock
	void lock()
	{
		while(m_lock.test_and_set(std::memory_order_acquire))
		{
		}
	}

	/// Unlock
	void unlock()
	{
		m_lock.clear(std::memory_order_release);
	}

private:
	std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};

/// Lock guard. When constructed it locks a TMutex and unlocks it when it gets destroyed.
/// @tparam TMutex Can be Mutex or SpinLock.
template<typename TMutex>
class LockGuard
{
public:
	LockGuard(TMutex& mtx)
		: m_mtx(&mtx)
	{
		m_mtx->lock();
	}

	LockGuard(const LockGuard& b) = delete;

	LockGuard(LockGuard&& b)
	{
		m_mtx = b.m_mtx;
		b.m_mtx = nullptr;
	}

	~LockGuard()
	{
		if(m_mtx)
		{
			m_mtx->unlock();
		}
	}

	LockGuard& operator=(LockGuard&& b) = delete;

	LockGuard& operator=(const LockGuard& b) = delete;

private:
	TMutex* m_mtx;
};

/// A barrier for thread synchronization. It works almost like boost::barrier.
class Barrier : public NonCopyable
{
public:
	Barrier(U32 count);

	~Barrier();

	/// Wait until all threads call wait().
	Bool wait();

private:
	void* m_impl = nullptr;
};

/// Semaphore for thread synchronization.
class Semaphore : public NonCopyable
{
public:
	Semaphore(I32 initialValue);

	~Semaphore();

	/// Same as sem_wait().
	void wait();

	/// Same as sem_post().
	void post();

private:
	void* m_impl = nullptr;
};
/// @}

} // end namespace anki
