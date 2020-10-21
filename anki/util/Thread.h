// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Array.h>
#include <anki/util/Atomic.h>
#include <thread>
#if ANKI_SIMD_SSE
#	include <xmmintrin.h>
#endif
#if ANKI_POSIX
#	include <pthread.h>
#	include <semaphore.h>
#else
#	include <anki/util/Win32Minimal.h>
#endif

namespace anki
{

/// @addtogroup util_thread
/// @{

/// The thread ID.
/// @memberof Thread
using ThreadId = U64;

/// It holds some information to be passed to the thread's callback.
/// @memberof Thread
class ThreadCallbackInfo
{
public:
	void* m_userData;
	const char* m_threadName;
};

/// The type of the tread callback.
/// @memberof Thread
using ThreadCallback = Error (*)(ThreadCallbackInfo&);

/// Thread implementation.
class Thread
{
public:
	/// Create a thread with or without a name
	/// @param[in] name The name of the new thread. Can be nullptr.
	Thread(const char* name)
	{
		if(name)
		{
			PtrSize len = std::strlen(name);
			len = std::min<PtrSize>(len, sizeof(m_name) - 1);
			memcpy(&m_name[0], &name[0], len);
			m_name[len] = '\0';
		}
		else
		{
			m_name[0] = '\0';
		}
	}

	Thread(const Thread&) = delete;

	~Thread()
	{
		ANKI_ASSERT(!m_started && "Thread probably not joined");
		m_handle = {};
	}

	Thread& operator=(const Thread&) = delete;

	/// Start the thread.
	/// @param userData The user data of the thread callback
	/// @param callback The thread callback that will be executed
	/// @param pinToCore Pin the thread to a core.
	void start(void* userData, ThreadCallback callback, I32 pinToCore = -1);

	/// Wait for the thread to finish
	/// @return The error code of the thread's callback
	ANKI_USE_RESULT Error join();

	/// Identify the current thread
	static ThreadId getCurrentThreadId()
	{
#if ANKI_POSIX
		return pthread_self();
#else
		return GetCurrentThreadId();
#endif
	}

private:
	/// The system native type.
#if ANKI_POSIX
	pthread_t m_handle = {};
#else
	HANDLE m_handle = 0; ///< The user date to pass to the callback.
	Error m_returnCode = Error::NONE;
#endif
	void* m_userData = nullptr; ///< The user date to pass to the callback.
	Array<char, 32> m_name; ///< The name of the thread.
	ThreadCallback m_callback = nullptr; ///< The callback.

#if ANKI_EXTRA_CHECKS
	Bool m_started = false;
#endif

#if ANKI_OS_WINDOWS
	static DWORD ANKI_WINAPI threadCallback(LPVOID ud);
#endif
};

/// Mutual exclusion primitive.
class Mutex
{
	friend class ConditionVariable;

public:
	Mutex()
	{
#if ANKI_POSIX
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
		pthread_mutex_init(&m_handle, &attr);
		pthread_mutexattr_destroy(&attr);
#else
		InitializeCriticalSection(&m_handle);
#endif
	}

	Mutex(const Mutex&) = delete;

	~Mutex()
	{
#if ANKI_POSIX
		pthread_mutex_destroy(&m_handle);
#else
		DeleteCriticalSection(&m_handle);
#endif
	}

	Mutex& operator=(const Mutex&) = delete;

	/// Lock
	void lock()
	{
#if ANKI_POSIX
		pthread_mutex_lock(&m_handle);
#else
		EnterCriticalSection(&m_handle);
#endif
	}

	/// Try lock
	/// @return True if it was locked successfully
	Bool tryLock()
	{
#if ANKI_POSIX
		return pthread_mutex_trylock(&m_handle) == 0;
#else
		const BOOL enter = TryEnterCriticalSection(&m_handle);
		return enter != 0;
#endif
	}

	/// Unlock
	void unlock()
	{
#if ANKI_POSIX
		pthread_mutex_unlock(&m_handle);
#else
		LeaveCriticalSection(&m_handle);
#endif
	}

private:
	/// The system native type.
#if ANKI_POSIX
	pthread_mutex_t m_handle = {};
#else
	CRITICAL_SECTION m_handle = {};
#endif
};

/// Read write mutex.
class RWMutex
{
public:
	RWMutex()
	{
#if ANKI_POSIX
		pthread_rwlockattr_t attr;
		pthread_rwlockattr_init(&attr);
		pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
		pthread_rwlock_init(&m_handle, &attr);
		pthread_rwlockattr_destroy(&attr);
#else
		InitializeSRWLock(&m_handle);
#endif
	}

	RWMutex(const RWMutex&) = delete;

	~RWMutex()
	{
#if ANKI_POSIX
		pthread_rwlock_destroy(&m_handle);
#else
		// Nothing
#endif
	}

	RWMutex& operator=(const RWMutex&) = delete;

	/// Lock for reading.
	void lockRead()
	{
#if ANKI_POSIX
		pthread_rwlock_rdlock(&m_handle);
#else
		AcquireSRWLockShared(&m_handle);
#endif
	}

	/// Unlock from reading.
	void unlockRead()
	{
#if ANKI_POSIX
		pthread_rwlock_unlock(&m_handle);
#else
		ReleaseSRWLockShared(&m_handle);
#endif
	}

	/// Lock for writing.
	void lockWrite()
	{
#if ANKI_POSIX
		pthread_rwlock_wrlock(&m_handle);
#else
		AcquireSRWLockExclusive(&m_handle);
#endif
	}

	/// Unlock from writing.
	void unlockWrite()
	{
#if ANKI_POSIX
		pthread_rwlock_unlock(&m_handle);
#else
		ReleaseSRWLockExclusive(&m_handle);
#endif
	}

private:
#if ANKI_POSIX
	pthread_rwlock_t m_handle;
#else
	SRWLOCK m_handle;
#endif
};

/// Condition variable.
class ConditionVariable
{
public:
	ConditionVariable()
	{
#if ANKI_POSIX
		pthread_condattr_t attr;
		pthread_condattr_init(&attr);
		pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
		pthread_cond_init(&m_handle, &attr);
		pthread_condattr_destroy(&attr);
#else
		InitializeConditionVariable(&m_handle);
#endif
	}

	ConditionVariable(const ConditionVariable&) = delete;

	~ConditionVariable()
	{
#if ANKI_POSIX
		pthread_cond_destroy(&m_handle);
#else
		// Nothing
#endif
	}

	ConditionVariable& operator=(const ConditionVariable&) = delete;

	/// Signal one thread
	void notifyOne()
	{
#if ANKI_POSIX
		pthread_cond_signal(&m_handle);
#else
		WakeConditionVariable(&m_handle);
#endif
	}

	/// Signal all threads
	void notifyAll()
	{
#if ANKI_POSIX
		pthread_cond_broadcast(&m_handle);
#else
		WakeAllConditionVariable(&m_handle);
#endif
	}

	/// Bock until signaled.
	/// @param mtx The mutex.
	void wait(Mutex& mtx)
	{
#if ANKI_POSIX
		pthread_cond_wait(&m_handle, &mtx.m_handle);
#else
		SleepConditionVariableCS(&m_handle, &mtx.m_handle, MAX_U32);
#endif
	}

private:
#if ANKI_POSIX
	pthread_cond_t m_handle;
#else
	CONDITION_VARIABLE m_handle;
#endif
};

/// Mutual exclusion primitive. Like Mutex. It's better than Mutex only if the critical section will be executed in a
/// very short period of time.
class SpinLock
{
public:
	SpinLock() = default;

	SpinLock(const SpinLock&) = delete;

	SpinLock& operator=(const SpinLock&) = delete;

	/// Lock.
	void lock()
	{
		for(U spinCount = 0; !tryLock(); ++spinCount)
		{
			if(spinCount < 16)
			{
#if ANKI_SIMD_SSE
				_mm_pause();
#endif
			}
			else
			{
				std::this_thread::yield();
				spinCount = 0;
			}
		}
	}

	/// Unlock.
	void unlock()
	{
		m_lock.store(false, AtomicMemoryOrder::RELEASE);
	}

	/// Try to lock.
	Bool tryLock()
	{
		return !m_lock.load(AtomicMemoryOrder::RELAXED) && !m_lock.exchange(true, AtomicMemoryOrder::ACQUIRE);
	}

private:
	Atomic<Bool> m_lock = {false};
};

/// A barrier for thread synchronization. It works almost like boost::barrier.
class Barrier
{
public:
	Barrier(U32 count)
	{
#if ANKI_POSIX
		pthread_barrierattr_t attr;
		pthread_barrierattr_init(&attr);
		pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
		pthread_barrier_init(&m_handle, &attr, count);
		pthread_barrierattr_destroy(&attr);
#else
		InitializeCriticalSection(&m_mtx);
		InitializeConditionVariable(&m_cvar);

		m_threshold = count;
		m_count = count;
		m_generation = 0;
#endif
	}

	Barrier(const Barrier&) = delete;

	~Barrier()
	{
#if ANKI_POSIX
		pthread_barrier_destroy(&m_handle);
#else
		DeleteCriticalSection(&m_mtx);
#endif
	}

	Barrier& operator=(const Barrier&) = delete;

	/// Wait until all threads call wait().
	void wait()
	{
#if ANKI_POSIX
		pthread_barrier_wait(&m_handle);
#else
		EnterCriticalSection(&m_mtx);
		const U32 gen = m_generation;

		if(--m_count == 0)
		{
			++m_generation;
			m_count = m_threshold;
			WakeAllConditionVariable(&m_cvar);
		}
		else
		{
			while(gen == m_generation)
			{
				SleepConditionVariableCS(&m_cvar, &m_mtx, MAX_U32);
			}
		}

		LeaveCriticalSection(&m_mtx);
#endif
	}

private:
#if ANKI_POSIX
	pthread_barrier_t m_handle;
#else
	CONDITION_VARIABLE m_cvar;
	CRITICAL_SECTION m_mtx;
	U32 m_threshold;
	U32 m_count;
	U32 m_generation;
#endif
};

/// Semaphore for thread synchronization.
class Semaphore
{
public:
	Semaphore(I32 initialValue)
	{
#if ANKI_POSIX
		sem_init(&m_handle, 0, initialValue);
#else
		ANKI_ASSERT(!"TODO");
#endif
	}

	Semaphore(const Semaphore&) = delete;

	~Semaphore()
	{
#if ANKI_POSIX
		sem_destroy(&m_handle);
#else
		ANKI_ASSERT(!"TODO");
#endif
	}

	Semaphore& operator=(const Semaphore&) = delete;

	/// Same as sem_wait().
	/// @code
	/// if(value == 0) wait();
	/// --value;
	/// @endcode
	void wait()
	{
#if ANKI_POSIX
		sem_wait(&m_handle);
#else
		ANKI_ASSERT(!"TODO");
#endif
	}

	/// Same as sem_post().
	/// @code
	/// ++value;
	/// wakeupWaiters();
	/// @endcode
	void post()
	{
#if ANKI_POSIX
		sem_post(&m_handle);
#else
		ANKI_ASSERT(!"TODO");
#endif
	}

private:
#if ANKI_POSIX
	sem_t m_handle;
#endif
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

/// Read/write lock guard. When constructed it locks a TMutex and unlocks it when it gets destroyed.
/// @tparam TMutex Can be RWMutex.
template<typename TMutex, Bool READER>
class RWLockGuard
{
public:
	RWLockGuard(TMutex& mtx)
		: m_mtx(&mtx)
	{
		if(READER)
		{
			m_mtx->lockRead();
		}
		else
		{
			m_mtx->lockWrite();
		}
	}

	RWLockGuard(const RWLockGuard& b) = delete;

	~RWLockGuard()
	{
		if(READER)
		{
			m_mtx->unlockRead();
		}
		else
		{
			m_mtx->unlockWrite();
		}
	}

	RWLockGuard& operator=(const RWLockGuard& b) = delete;

private:
	TMutex* m_mtx;
};

/// Read lock guard.
template<typename TMutex>
using RLockGuard = RWLockGuard<TMutex, true>;

/// Write lock guard.
template<typename TMutex>
using WLockGuard = RWLockGuard<TMutex, false>;
/// @}

} // end namespace anki
