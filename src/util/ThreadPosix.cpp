// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Thread.h"
#include "anki/util/Exception.h"
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

	I err = thread->_getCallback()(info);
	void* errVoidp = nullptr;
	std::memcpy(&errVoidp, &err, sizeof(err));

	return nullptr;
}

//==============================================================================
Thread::Thread(const char* name)
{
	m_impl = malloc(sizeof(pthread_t));
	if(m_impl == nullptr)
	{
		throw ANKI_EXCEPTION("Out of memory");
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

	pthread_t* impl = reinterpret_cast<pthread_t*>(m_impl);

	m_callback = callback;
	m_userData = userData;

	I err = pthread_create(impl, nullptr, pthreadCallback, this);
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_create() failed");
	}
	else
	{
#if ANKI_ASSERTIONS
		m_started = true;
#endif
	}
}

//==============================================================================
I Thread::join()
{
	ANKI_ASSERT(m_started);
	pthread_t* impl = reinterpret_cast<pthread_t*>(m_impl);

	void* out;
	U err = pthread_join(*impl, &out);
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_join() failed");
	}

#if ANKI_ASSERTIONS
	m_started = false;
#endif

	// Set return error code
	I callbackErr;
	std::memcpy(&callbackErr, &out, sizeof(callbackErr));
	return callbackErr;
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
		throw ANKI_EXCEPTION("Out of memory");
	}

	m_impl = mtx;

	I err = pthread_mutex_init(mtx, nullptr);
	if(err)
	{
		free(m_impl);
		m_impl = nullptr;
		throw ANKI_EXCEPTION("pthread_mutex_init() failed");
	}
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
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);

	I err = pthread_mutex_lock(mtx);
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_mutex_lock() failed");
	}
}

//==============================================================================
Bool Mutex::tryLock()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);

	I err = pthread_mutex_trylock(mtx);
	return err == 0;
}

//==============================================================================
void Mutex::unlock()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(m_impl);

	I err = pthread_mutex_unlock(mtx);
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_mutex_unlock() failed");
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
		throw ANKI_EXCEPTION("Out of memory");
	}

	m_impl = cond;

	I err = pthread_cond_init(cond, nullptr);
	if(err)
	{
		free(m_impl);
		m_impl = nullptr;
		throw ANKI_EXCEPTION("pthread_cond_init() failed");
	}
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
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_cond_signal(cond);
}

//==============================================================================
void ConditionVariable::notifyAll()
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_cond_broadcast(cond);
}

//==============================================================================
Bool ConditionVariable::wait(Mutex& amtx, F64 timeoutSeconds)
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(m_impl);
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(amtx.m_impl);

	Bool timeout = false;
	I err = 0;

	if(timeoutSeconds == 0.0)
	{
		err = pthread_cond_wait(cond, mtx);
	}
	else
	{
		U64 ns = static_cast<U64>(timeoutSeconds * 1e+9);
		struct timespec reltime;
		reltime.tv_sec = ns / 1000000000;
		reltime.tv_nsec = (ns % 1000000000);

		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);

		struct timespec abstime;
		memset(&abstime, 0, sizeof(abstime));
		abstime.tv_sec = now.tv_sec + reltime.tv_sec;
		abstime.tv_nsec = now.tv_nsec + reltime.tv_nsec;
		
		err = pthread_cond_timedwait(cond, mtx, &abstime);
	}

	if(err == 0)
	{
		// Do nothing
	}
	else if(err == ETIMEDOUT)
	{
		timeout = true;
	}
	else
	{
		throw ANKI_EXCEPTION("pthread_cond_wait() or pthread_cond_timedwait()"
			" failed: %d", err);
	}

	return timeout;
}

} // end namespace anki

