#include "anki/util/System.h"
#include "anki/Config.h"

#if ANKI_POSIX
#	include <unistd.h>
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

} // end namespace anki
