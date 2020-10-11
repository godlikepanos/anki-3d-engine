// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Add support for condition variables
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <anki/util/Thread.h>
#include <anki/util/Logger.h>

namespace anki
{

DWORD ANKI_WINAPI Thread::threadCallback(LPVOID ud)
{
	ANKI_ASSERT(ud != nullptr);
	Thread* thread = reinterpret_cast<Thread*>(ud);

	// Set thread name
	if(thread->m_name[0] != '\0')
	{
		// TODO
	}

	// Call the callback
	ThreadCallbackInfo info;
	info.m_userData = thread->m_userData;
	info.m_threadName = &thread->m_name[0];

	thread->m_returnCode = thread->m_callback(info);

	return thread->m_returnCode._getCode();
}

void Thread::start(void* userData, ThreadCallback callback, I32 pinToCore)
{
	ANKI_ASSERT(!m_started);
	ANKI_ASSERT(callback != nullptr);
	m_callback = callback;
	m_userData = userData;
#if ANKI_EXTRA_CHECKS
	m_started = true;
#endif
	m_returnCode = Error::NONE;

	m_handle = CreateThread(nullptr, 0, threadCallback, this, 0, nullptr);
	if(m_handle == nullptr)
	{
		ANKI_UTIL_LOGF("CreateThread() failed");
	}

	if(pinToCore >= 0)
	{
		if(SetThreadAffinityMask(m_handle, DWORD_PTR(1) << DWORD_PTR(pinToCore)) == 0)
		{
			ANKI_UTIL_LOGF("SetThreadAffinityMask() failed");
		}
	}
}

Error Thread::join()
{
	ANKI_ASSERT(m_started);

	// Wait thread
	WaitForSingleObject(m_handle, INFINITE);

	// Delete handle
	const BOOL ok = CloseHandle(m_handle);
	if(!ok)
	{
		ANKI_UTIL_LOGF("CloseHandle() failed");
	}

	m_handle = nullptr;

#if ANKI_EXTRA_CHECKS
	m_started = false;
#endif

	return m_returnCode;
}

} // end namespace anki
