// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Thread.h>
#include <anki/util/Logger.h>

namespace anki
{

void Thread::start(void* userData, ThreadCallback callback, I32 pinToCore)
{
	ANKI_ASSERT(!m_started);
	ANKI_ASSERT(callback != nullptr);
	m_callback = callback;
	m_userData = userData;
#if ANKI_EXTRA_CHECKS
	m_started = true;
#endif

	pthread_attr_t attr;
	cpu_set_t cpus;
	pthread_attr_init(&attr);

	if(pinToCore >= 0)
	{
		CPU_ZERO(&cpus);
		CPU_SET(pinToCore, &cpus);
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	}

	auto pthreadCallback = [](void* ud) -> void* {
		ANKI_ASSERT(ud != nullptr);
		Thread* thread = static_cast<Thread*>(ud);

		// Set thread name
		if(thread->m_name[0] != '\0')
		{
			pthread_setname_np(pthread_self(), &thread->m_name[0]);
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

} // end namespace anki
