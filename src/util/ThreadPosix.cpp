// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Thread.h"
#include "anki/util/Logger.h"
#include <cstring>
#include <algorithm>
#include <pthread.h>

namespace anki {

//==============================================================================
// Thread                                                                      =
//==============================================================================

//==============================================================================
static void* pthreadCallback(void* ud)
{
	ANKI_ASSERT(ud != nullptr);
	Thread* thread = reinterpret_cast<Thread*>(ud);

	// Set thread name
	if(thread->_getName()[0] != '\0')
	{
		pthread_setname_np(pthread_self(), &thread->_getName()[0]);
	}

	// Call the callback
	Thread::Info info;
	info.m_userData = thread->_getUserData();
	info.m_threadName = thread->_getName();

	Error err = thread->_getCallback()(info);

	return reinterpret_cast<void*>(static_cast<PtrSize>(err._getCode()));
}

//==============================================================================
Thread::Thread(const char* name)
{
	m_impl = malloc(sizeof(pthread_t));
	if(m_impl == nullptr)
	{
		ANKI_LOGF("Out of memory");
	}

	// Init the name
	if(name)
	{
		U len = std::strlen(name);
		len = std::min<U>(len, sizeof(m_name) - 1);
		std::memcpy(&m_name[0], &name[0], len);
		m_name[len] = '\0';
	}
	else
	{
		m_name[0] = '\0';
	}
}

//==============================================================================
Thread::~Thread()
{
	ANKI_ASSERT(!m_started && "Thread probably not joined");
	free(m_impl);
	m_impl = nullptr;
}

//==============================================================================
void Thread::start(void* userData, Callback callback)
{
	ANKI_ASSERT(!m_started);
	ANKI_ASSERT(callback != nullptr);

	pthread_t* thread = reinterpret_cast<pthread_t*>(m_impl);

	m_callback = callback;
	m_userData = userData;

	I err = pthread_create(thread, nullptr, pthreadCallback, this);
	if(err)
	{
		ANKI_LOGF("pthread_create() failed");
	}
	else
	{
#if ANKI_ASSERTIONS
		m_started = true;
#endif
	}
}

//==============================================================================
Error Thread::join()
{
	ANKI_ASSERT(m_started);
	pthread_t* thread = reinterpret_cast<pthread_t*>(m_impl);

	void* out;
	I err = pthread_join(*thread, &out);
	if(err)
	{
		ANKI_LOGF("pthread_join() failed");
	}

#if ANKI_ASSERTIONS
	m_started = false;
#endif

	// Set return error code
	ErrorCode code = static_cast<ErrorCode>(reinterpret_cast<PtrSize>(out));
	return code;
}

//==============================================================================
Thread::Id Thread::getCurrentThreadId()
{
	pthread_t pid = pthread_self();
	return pid;
}

//==============================================================================
// Mutex                                                                       =
//==============================================================================

//==============================================================================
Mutex::Mutex()
{
	pthread_mutex_t* mtx = 
		reinterpret_cast<pthread_mutex_t*>(malloc(sizeof(pthread_mutex_t)));
	if(mtx == nullptr)
	{
		ANKI_LOGF("Out of memory");
	}

	I err = pthread_mutex_init(mtx, nullptr);
	if(err)
	{
		free(mtx);
		ANKI_LOGF("pthread_mutex_init() failed");
	}

	m_impl = mtx;
}

//==============================================================================
Mutex::~Mutex()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);
	pthread_mutex_destroy(mtx);

	free(m_impl);
	m_impl = nullptr;
}

//==============================================================================
void Mutex::lock()
{
	ANKI_ASSERT(m_impl);
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);

	I err = pthread_mutex_lock(mtx);
	if(err)
	{
		ANKI_LOGF("pthread_mutex_lock() failed");
	}
}

//==============================================================================
Bool Mutex::tryLock()
{
	ANKI_ASSERT(m_impl);
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);

	I err = pthread_mutex_trylock(mtx);
	return err == 0;
}

//==============================================================================
void Mutex::unlock()
{
	ANKI_ASSERT(m_impl);
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);

	I err = pthread_mutex_unlock(mtx);
	if(err)
	{
		ANKI_LOGF("pthread_mutex_unlock() failed");
	}
}

//==============================================================================
// ConditionVariable                                                           =
//==============================================================================

//==============================================================================
ConditionVariable::ConditionVariable()
{
	pthread_cond_t* cond = 
		reinterpret_cast<pthread_cond_t*>(malloc(sizeof(pthread_cond_t)));
	if(cond == nullptr)
	{
		ANKI_LOGF("Out of memory");
	}

	I err = pthread_cond_init(cond, nullptr);
	if(err)
	{
		free(cond);
		ANKI_LOGF("pthread_cond_init() failed");
	}

	m_impl = cond;
}

//==============================================================================
ConditionVariable::~ConditionVariable()
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_cond_destroy(cond);

	free(m_impl);
	m_impl = nullptr;
}

//==============================================================================
void ConditionVariable::notifyOne()
{
	ANKI_ASSERT(m_impl);
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_cond_signal(cond);
}

//==============================================================================
void ConditionVariable::notifyAll()
{
	ANKI_ASSERT(m_impl);
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_cond_broadcast(cond);
}

//==============================================================================
void ConditionVariable::wait(Mutex& amtx)
{
	ANKI_ASSERT(m_impl);
	ANKI_ASSERT(amtx.m_impl);
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(amtx.m_impl);

	I err = pthread_cond_wait(cond, mtx);
	if(err)
	{
		ANKI_LOGF("pthread_cond_wait() failed");
	}
}

//==============================================================================
// Barrier                                                                     =
//==============================================================================

//==============================================================================
Barrier::Barrier(U32 count)
{
	ANKI_ASSERT(count > 1);

	pthread_barrier_t* barrier = 
		reinterpret_cast<pthread_barrier_t*>(malloc(sizeof(pthread_barrier_t)));
	if(barrier == nullptr)
	{
		ANKI_LOGF("Out of memory");
	}

	I err = pthread_barrier_init(barrier, nullptr, count);
	if(err)
	{
		free(barrier);
		ANKI_LOGF("pthread_barrier_init() failed");
	}

	m_impl = barrier;
}

//==============================================================================
Barrier::~Barrier()
{
	ANKI_ASSERT(m_impl);
	pthread_barrier_t* barrier = reinterpret_cast<pthread_barrier_t*>(m_impl);

	I err = pthread_barrier_destroy(barrier);
	if(err)
	{
		ANKI_LOGE("pthread_barrier_destroy() failed");
	}

	free(barrier);
	m_impl = nullptr;
}

//==============================================================================
Bool Barrier::wait()
{
	ANKI_ASSERT(m_impl);

	pthread_barrier_t* barrier = 
		reinterpret_cast<pthread_barrier_t*>(m_impl);

	I err = pthread_barrier_wait(barrier);
	if(err != PTHREAD_BARRIER_SERIAL_THREAD && err != 0)
	{
		ANKI_LOGF("pthread_barrier_wait() failed");
	}

	return true;
}

} // end namespace anki

