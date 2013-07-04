#include "anki/util/System.h"
#include "anki/Config.h"

#if ANKI_POSIX
#	include <unistd.h>
#	include <execinfo.h>
#	include <signal.h>
#else
#	error "Unimplemented"
#endif

namespace anki {

//==============================================================================
U32 getCpuCoresCount()
{
#if ANKI_POSIX
	return sysconf(_SC_NPROCESSORS_ONLN);
#else
#	error "Unimplemented"
#endif
}

//==============================================================================
void printBacktrace()
{
#if ANKI_POSIX
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	backtrace_symbols_fd(array, size, 2);	
#else
#	error "Unimplemented"
#endif
}

} // end namespace anki
