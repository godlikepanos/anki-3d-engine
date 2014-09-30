// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Thread.h"
#include "anki/util/Exception.h"
#include <windows.h>

namespace anki {

//==============================================================================
// Thread                                                                      =
//==============================================================================

//==============================================================================
static DWORD WINAPI threadCallback(LPVOID ud)
{
	ANKI_ASSERT(ud != nullptr);
	Thread* thread = reinterpret_cast<Thread*>(ud);

	// Set thread name
	if(thread->_getName()[0] != '\0')
	{
		// TODO
	}

	// Call the callback
	Thread::Info info;
	info.m_userData = thread->_getUserData();
	info.m_threadName = thread->_getName();

	I err = thread->_getCallback()(info);

	return err;
}

//==============================================================================
Thread::Thread(const char* name)
{
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

	m_callback = callback;
	m_userData = userData;

	m_impl = CreateThread(nullptr, 0, threadCallback, this, 0, nullptr);
	if(m_impl == nullptr)
	{
		throw ANKI_EXCEPTION("CreateThread() failed");
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

	// Wait thread
	WaitForSingleObject(m_impl, INFINITE);
	
	// Get return code
	DWORD exitCode = 0;
	ok = GetExitCodeThread(m_impl, &exitCode);
	if(!ok)
	{
		throw ANKI_EXCEPTION("GetExitCodeThread() failed");
	}

	// Delete handle
	BOOL ok = CloseHandle(m_impl);
	if(!ok)
	{
		throw ANKI_EXCEPTION("CloseHandle() failed");
	}

	return exitCode;
}

//==============================================================================
Thread::Id Thread::getCurrentThreadId()
{
	HANDLE x = GetCurrentThread();
	return x;
}

//==============================================================================
// Mutex                                                                       =
//==============================================================================

//==============================================================================
Mutex::Mutex()
{
	CRITICAL_SECTION* mtx = 
		reinterpret_cast<CRITICAL_SECTION*>(malloc(sizeof(CRITICAL_SECTION)));
	if(mtx == nullptr)
	{
		throw ANKI_EXCEPTION("Out of memory");
	}

	m_impl = mtx;

	BOOL ok = InitializeCriticalSectionAndSpinCount(mtx, 0x400);
	if(!ok)
	{
		free(m_impl);
		m_impl = nullptr;
		throw ANKI_EXCEPTION("InitializeCriticalSectionAndSpinCount() failed");
	}
}

//==============================================================================
Mutex::~Mutex()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);
	DeleteCriticalSection(mtx);

	free(m_impl);
	m_impl = nullptr;
}

//==============================================================================
void Mutex::lock()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);
	EnterCriticalSection(mtx);
}

//==============================================================================
Bool Mutex::tryLock()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);

	BOOL enter = EnterCriticalSection(mtx);
	return enter;
}

//==============================================================================
void Mutex::unlock()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);

	LeaveCriticalSection(mtx);
}

//==============================================================================
// ConditionVariable                                                           =
//==============================================================================

//==============================================================================
ConditionVariable::ConditionVariable()
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(
		malloc(sizeof(CONDITION_VARIABLE)));
	if(cond == nullptr)
	{
		throw ANKI_EXCEPTION("Out of memory");
	}

	m_impl = cond;

	InitializeConditionVariable(cond);
}

//==============================================================================
ConditionVariable::~ConditionVariable()
{
	free(m_impl);
	m_impl = nullptr;
}

//==============================================================================
void ConditionVariable::notifyOne()
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(m_impl);
	WakeConditionVariable(cond);
}

//==============================================================================
void ConditionVariable::notifyAll()
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(m_impl);
	WakeAllConditionVariable(cond);
}

//==============================================================================
void ConditionVariable::wait(Mutex& amtx)
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(m_impl);
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);

	SleepConditionVariableCS(cond, INFINITE);
}

} // end namespace anki

