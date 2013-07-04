#ifndef ANKI_UTIL_SYSTEM_H
#define ANKI_UTIL_SYSTEM_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup system
/// @{

/// Get the number of CPU cores
extern U32 getCpuCoresCount();

/// Print the backtrace
extern void printBacktrace();

/// @}
/// @}

} // end namespace anki

#endif
