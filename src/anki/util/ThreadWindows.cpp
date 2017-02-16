// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Add support for condition variables
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <anki/util/Thread.h>
#include <anki/util/Functions.h>
#include <anki/util/Logger.h>
#include <cstring>
#include <Windows.h>
#include <WinBase.h>

namespace anki
{

static DWORD WINAPI threadCallback(LPVOID ud)
{
	ANKI_ASSERT(ud != nullptr);
	Thread* thread = reinterpret_cast<Thread*>(ud);

	// Set thread name
	if(thread->getName()[0] != '\0')
	{
		// TODO
	}

	// Call the callback
	ThreadCallbackInfo info;
	info.m_userData = thread->getUserData();
	info.m_threadName = thread->getName();

	Error err = thread->getCallback()(info);

	return err._getCodeInt();
}

Thread::Thread(const char* name)
{
	// Init the name
	if(name)
	{
		U len = std::strlen(name);
		len = min<U>(len, sizeof(m_name) - 1);
		std::memcpy(&m_name[0], &name[0], len);
		m_name[len] = '\0';
	}
	else
	{
		m_name[0] = '\0';
	}
}

Thread::~Thread()
{
	ANKI_ASSERT(!m_started && "Thread probably not joined");
	ANKI_ASSERT(m_impl == nullptr);
	m_impl = nullptr;
}

void Thread::start(void* userData, ThreadCallback callback, I pinToCore)
{
	ANKI_ASSERT(!m_started);
	ANKI_ASSERT(callback != nullptr);

	m_callback = callback;
	m_userData = userData;

	m_impl = CreateThread(nullptr, 0, threadCallback, this, 0, nullptr);
	if(m_impl == nullptr)
	{
		ANKI_UTIL_LOGF("CreateThread() failed");
	}
	else
	{
#if ANKI_EXTRA_CHECKS
		m_started = true;
#endif
	}
}

Error Thread::join()
{
	ANKI_ASSERT(m_started);

	// Wait thread
	WaitForSingleObject(m_impl, INFINITE);

	// Get return code
	DWORD exitCode = 0;
	BOOL ok = GetExitCodeThread(m_impl, &exitCode);
	if(!ok)
	{
		ANKI_UTIL_LOGF("GetExitCodeThread() failed");
	}

	// Delete handle
	ok = CloseHandle(m_impl);
	if(!ok)
	{
		ANKI_UTIL_LOGF("CloseHandle() failed");
	}

	m_impl = nullptr;

#if ANKI_EXTRA_CHECKS
	m_started = false;
#endif

	return static_cast<ErrorCode>(exitCode);
}

ThreadId Thread::getCurrentThreadId()
{
	DWORD x = GetCurrentThreadId();
	return PtrSize(x);
}

Mutex::Mutex()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(malloc(sizeof(CRITICAL_SECTION)));
	if(mtx == nullptr)
	{
		ANKI_UTIL_LOGF("Out of memory");
	}

	m_impl = mtx;

	InitializeCriticalSection(mtx);
}

Mutex::~Mutex()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);
	DeleteCriticalSection(mtx);

	free(m_impl);
	m_impl = nullptr;
}

void Mutex::lock()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);
	EnterCriticalSection(mtx);
}

Bool Mutex::tryLock()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);
	BOOL enter = TryEnterCriticalSection(mtx);
	return enter;
}

void Mutex::unlock()
{
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(m_impl);
	LeaveCriticalSection(mtx);
}

ConditionVariable::ConditionVariable()
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(malloc(sizeof(CONDITION_VARIABLE)));
	if(cond == nullptr)
	{
		ANKI_UTIL_LOGF("Out of memory");
	}

	m_impl = cond;

	InitializeConditionVariable(cond);
}

ConditionVariable::~ConditionVariable()
{
	free(m_impl);
	m_impl = nullptr;
}

void ConditionVariable::notifyOne()
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(m_impl);
	WakeConditionVariable(cond);
}

void ConditionVariable::notifyAll()
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(m_impl);
	WakeAllConditionVariable(cond);
}

void ConditionVariable::wait(Mutex& amtx)
{
	CONDITION_VARIABLE* cond = reinterpret_cast<CONDITION_VARIABLE*>(m_impl);
	CRITICAL_SECTION* mtx = reinterpret_cast<CRITICAL_SECTION*>(amtx.m_impl);

	SleepConditionVariableCS(cond, mtx, INFINITE);
}

struct BarrierImpl
{
	CONDITION_VARIABLE m_cvar;
	CRITICAL_SECTION m_mtx;
	U32 m_threshold;
	U32 m_count;
	U32 m_generation;
};

Barrier::Barrier(U32 count)
{
	ANKI_ASSERT(count > 1);

	BarrierImpl* barrier = reinterpret_cast<BarrierImpl*>(malloc(sizeof(BarrierImpl)));
	if(barrier == nullptr)
	{
		ANKI_UTIL_LOGF("Out of memory");
	}

	InitializeCriticalSection(&barrier->m_mtx);
	InitializeConditionVariable(&barrier->m_cvar);

	barrier->m_threshold = count;
	barrier->m_count = count;
	barrier->m_generation = 0;

	m_impl = barrier;
}

Barrier::~Barrier()
{
	ANKI_ASSERT(m_impl);
	BarrierImpl* barrier = reinterpret_cast<BarrierImpl*>(m_impl);

	DeleteCriticalSection(&barrier->m_mtx);
	free(barrier);
	m_impl = nullptr;
}

Bool Barrier::wait()
{
	ANKI_ASSERT(m_impl);
	BarrierImpl& barrier = *reinterpret_cast<BarrierImpl*>(m_impl);

	EnterCriticalSection(&barrier.m_mtx);
	U32 gen = barrier.m_generation;

	if(--barrier.m_count == 0)
	{
		++barrier.m_generation;
		barrier.m_count = barrier.m_threshold;
		WakeAllConditionVariable(&barrier.m_cvar);
		LeaveCriticalSection(&barrier.m_mtx);
		return true;
	}

	while(gen == barrier.m_generation)
	{
		SleepConditionVariableCS(&barrier.m_cvar, &barrier.m_mtx, INFINITE);
	}

	LeaveCriticalSection(&barrier.m_mtx);
	return false;
}

} // end namespace anki
