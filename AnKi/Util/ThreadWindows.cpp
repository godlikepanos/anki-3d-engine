// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Add support for condition variables
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Logger.h>

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

void Thread::start(void* userData, ThreadCallback callback, const ThreadCoreAffinityMask& coreAffintyMask)
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

	if(coreAffintyMask.getAny())
	{
		pinToCores(coreAffintyMask);
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

void Thread::pinToCores(const ThreadCoreAffinityMask& coreAffintyMask)
{
	static_assert(std::is_same<DWORD_PTR, U64>::value, "See file");

	ThreadCoreAffinityMask affinityTest = coreAffintyMask;
	DWORD_PTR affinity = 0;
	for(DWORD_PTR bit = 0; bit < 64; ++bit)
	{
		if(coreAffintyMask.get(bit))
		{
			affinity |= 1ull << bit;
			affinityTest.unset(bit);
		}
	}

	if(SetThreadAffinityMask(m_handle, affinity) == 0)
	{
		ANKI_UTIL_LOGF("SetThreadAffinityMask() failed");
	}

	if(affinityTest.getEnabledBitCount() > 0)
	{
		ANKI_UTIL_LOGE("Couldn't set affinity for all cores. Need to refactor the code");
	}
}

} // end namespace anki
