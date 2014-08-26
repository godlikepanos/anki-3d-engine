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
Thread::Thread(const char* name)
{
	// Do some static assertions on the private data
	static_assert(sizeof(m_impl) >= sizeof(pthread_t), 
		"Incorrect impl size");

	static_assert(ALIGNMENT >= alignof(pthread_t), "Incorrect impl alignment");

#if ANKI_ASSERTIONS
	std::memset(&m_impl[0], 0, sizeof(m_impl));
#endif

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
	ANKI_ASSERT(!initialized() && "Thread probably not joined");
}

//==============================================================================
void* Thread::pthreadCallback(void* ud)
{
	ANKI_ASSERT(ud != nullptr);
	Thread* thread = reinterpret_cast<Thread*>(ud);

	// Set thread name
	if(thread->m_name[0] != '\0')
	{
		pthread_setname_np(pthread_self(), &thread->m_name[0]);
	}

	// Call the callback
	Info info;
	info.m_userData = thread->m_userData;
	info.m_threadName = &thread->m_name[0];

	I err = thread->m_callback(info);
	void* errVoidp = nullptr;
	std::memcpy(&errVoidp, &err, sizeof(err));

	return nullptr;
}

//==============================================================================
void Thread::start(void* userData, Callback callback)
{
	ANKI_ASSERT(callback != nullptr);

	ANKI_ASSERT(!initialized());
	pthread_t* impl = reinterpret_cast<pthread_t*>(&m_impl[0]);

	m_callback = callback;
	m_userData = userData;

	I err = pthread_create(impl, nullptr, pthreadCallback, this);
	if(err)
	{
#if ANKI_ASSERTIONS
		std::memset(&m_impl[0], 0, sizeof(m_impl));
#endif
		throw ANKI_EXCEPTION("pthread_create() failed");
	}
}

//==============================================================================
I Thread::join()
{
	ANKI_ASSERT(initialized());
	pthread_t* impl = reinterpret_cast<pthread_t*>(&m_impl[0]);

	void* out;
	U err = pthread_join(*impl, &out);
#if ANKI_ASSERTIONS
	std::memset(&m_impl[0], 0, sizeof(m_impl));
#endif
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_join() failed");
	}

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
	// Do some static assertions on the private data
	static_assert(sizeof(m_impl) >= sizeof(pthread_mutex_t), 
		"Incorrect impl size");

	static_assert(ALIGNMENT >= alignof(pthread_mutex_t), 
		"Incorrect impl alignment");

	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(&m_impl[0]);

	I err = pthread_mutex_init(mtx, nullptr);
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_mutex_init() failed");
	}
}

//==============================================================================
Mutex::~Mutex()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(&m_impl[0]);
	pthread_mutex_destroy(mtx);
}

//==============================================================================
void Mutex::lock()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(&m_impl[0]);

	I err = pthread_mutex_lock(mtx);
	if(err)
	{
		throw ANKI_EXCEPTION("pthread_mutex_lock() failed");
	}
}

//==============================================================================
Bool Mutex::tryLock()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(&m_impl[0]);

	I err = pthread_mutex_trylock(mtx);
	return err == 0;
}

//==============================================================================
void Mutex::unlock()
{
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(&m_impl[0]);

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
	// Do some static assertions on the private data
	static_assert(sizeof(m_impl) >= sizeof(pthread_cond_t),
		"Incorrect impl size");

	static_assert(ALIGNMENT >= alignof(pthread_cond_t),
		"Incorrect impl alignment");

	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(&m_impl[0]);

	I err = pthread_cond_init(cond, nullptr);
	if(err)
	{
#if ANKI_ASSERTIONS
		std::memset(&m_impl[0], 0, sizeof(m_impl));
#endif
		throw ANKI_EXCEPTION("pthread_cond_init() failed");
	}
}

//==============================================================================
ConditionVariable::~ConditionVariable()
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(&m_impl[0]);
	pthread_cond_destroy(cond);
}

//==============================================================================
void ConditionVariable::notifyOne()
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(&m_impl[0]);
	pthread_cond_signal(cond);
}

//==============================================================================
void ConditionVariable::notifyAll()
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(&m_impl[0]);
	pthread_cond_broadcast(cond);
}

//==============================================================================
void ConditionVariable::wait(Mutex& amtx)
{
	pthread_cond_t* cond = reinterpret_cast<pthread_cond_t*>(&m_impl[0]);
	pthread_mutex_t* mtx = reinterpret_cast<pthread_mutex_t*>(&amtx.m_impl[0]);

	pthread_cond_wait(cond, mtx);
}

} // end namespace anki

