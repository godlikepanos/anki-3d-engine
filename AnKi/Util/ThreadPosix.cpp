// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/String.h>

namespace anki
{

void Thread::start(void* userData, ThreadCallback callback, const ThreadCoreAffinityMask& coreAffintyMask)
{
	ANKI_ASSERT(!m_started);
	ANKI_ASSERT(callback != nullptr);
	m_callback = callback;
	m_userData = userData;
#if ANKI_EXTRA_CHECKS
	m_started = true;
#endif

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	auto pthreadCallback = [](void* ud) -> void* {
		ANKI_ASSERT(ud != nullptr);
		Thread* thread = static_cast<Thread*>(ud);

		// Set thread name
		if(thread->m_name[0] != '\0')
		{
			setNameOfCurrentThread(&thread->m_name[0]);
		}

		// Call the callback
		ThreadCallbackInfo info;
		info.m_userData = thread->m_userData;
		info.m_threadName = &thread->m_name[0];

		const Error err = thread->m_callback(info);
		return numberToPtr<void*>(err._getCode());
	};

	if(ANKI_UNLIKELY(pthread_create(&m_handle, &attr, pthreadCallback, this)))
	{
		ANKI_UTIL_LOGF("pthread_create() failed");
	}

	pthread_attr_destroy(&attr);

	if(coreAffintyMask.getEnabledBitCount())
	{
		pinToCores(coreAffintyMask);
	}
}

Error Thread::join()
{
	void* out;
	if(ANKI_UNLIKELY(pthread_join(m_handle, &out)))
	{
		ANKI_UTIL_LOGF("pthread_join() failed");
	}

#if ANKI_EXTRA_CHECKS
	m_started = false;
#endif

	// Set return error code
	return Error(I32(ptrToNumber(out)));
}

void Thread::pinToCores(const ThreadCoreAffinityMask& coreAffintyMask)
{
	ANKI_ASSERT(m_started);

	cpu_set_t cpus;
	CPU_ZERO(&cpus);

	ThreadCoreAffinityMask affinity = coreAffintyMask;
	while(affinity.getEnabledBitCount() > 0)
	{
		const U32 msb = affinity.getMostSignificantBit();
		ANKI_ASSERT(msb != MAX_U32);
		affinity.unset(msb);
		CPU_SET(msb, &cpus);
	}

#if ANKI_OS_ANDROID
	if(sched_setaffinity(pthread_gettid_np(m_handle), sizeof(cpu_set_t), &cpus))
#else
	if(pthread_setaffinity_np(m_handle, sizeof(cpu_set_t), &cpus))
#endif
	{
		ANKI_UTIL_LOGF("pthread_setaffinity_np() failed");
	}
}

void Thread::setNameOfCurrentThread(const CString& name)
{
	pthread_setname_np(pthread_self(), name.cstr());
}

} // end namespace anki
