// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/System.h>
#include <AnKi/Util/Logger.h>
#include <cstdio>

#if ANKI_POSIX
#	include <unistd.h>
#	include <signal.h>
#elif ANKI_OS_WINDOWS
#	include <AnKi/Util/Win32Minimal.h>
#else
#	error "Unimplemented"
#endif

// For print backtrace
#if ANKI_POSIX && !ANKI_OS_ANDROID
#	include <execinfo.h>
#	include <cstdlib>
#endif

namespace anki
{

U32 getCpuCoresCount()
{
#if ANKI_POSIX
	return U32(sysconf(_SC_NPROCESSORS_ONLN));
#elif ANKI_OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#else
#	error "Unimplemented"
#endif
}

void backtraceInternal(const Function<void(CString)>& lambda)
{
#if ANKI_POSIX && !ANKI_OS_ANDROID
	// Get addresses's for all entries on the stack
	const U32 maxStackSize = 64;
	void** array = static_cast<void**>(malloc(maxStackSize * sizeof(void*)));
	if(array)
	{
		const I32 size = ::backtrace(array, I32(maxStackSize));

		// Get symbols
		char** strings = backtrace_symbols(array, size);

		if(strings)
		{
			for(I32 i = 0; i < size; ++i)
			{
				lambda(strings[i]);
			}

			free(strings);
		}

		free(array);
	}
#else
	lambda("backtrace() not supported in " ANKI_OS_STR);
#endif
}

Bool runningFromATerminal()
{
#if ANKI_POSIX
	return isatty(fileno(stdin));
#else
	return false;
#endif
}

std::tm getLocalTime()
{
	std::time_t t = std::time(nullptr);
	std::tm tm;

#if ANKI_POSIX
	localtime_r(&t, &tm);
#elif ANKI_OS_WINDOWS
	localtime_s(&tm, &t);
#else
#	error See file
#endif

	return tm;
}

} // end namespace anki
