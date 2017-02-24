// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/System.h>
#include <anki/util/Logger.h>
#include <cstdio>

#if ANKI_POSIX
#include <unistd.h>
#include <signal.h>
#elif ANKI_OS == ANKI_OS_WINDOWS
#include <windows.h>
#else
#error "Unimplemented"
#endif

// For print backtrace
#if ANKI_POSIX && ANKI_OS != ANKI_OS_ANDROID
#include <execinfo.h>
#include <cstdlib>
#endif

namespace anki
{

U32 getCpuCoresCount()
{
#if ANKI_POSIX
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif ANKI_OS == ANKI_OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#else
#error "Unimplemented"
#endif
}

void BackTraceWalker::exec()
{
#if ANKI_POSIX && ANKI_OS != ANKI_OS_ANDROID
	// Get addresses's for all entries on the stack
	void** array = static_cast<void**>(malloc(m_stackSize * sizeof(void*)));
	if(array)
	{
		size_t size = backtrace(array, m_stackSize);

		// Get symbols
		char** strings = backtrace_symbols(array, size);

		if(strings)
		{
			for(size_t i = 0; i < size; ++i)
			{
				operator()(strings[i]);
			}

			free(strings);
		}

		free(array);
	}
#else
	ANKI_UTIL_LOGW("BackTraceWalker::exec() Not supported in this platform");
#endif
}

} // end namespace anki
