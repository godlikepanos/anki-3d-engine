// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_SYSTEM_H
#define ANKI_UTIL_SYSTEM_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_system
/// @{

/// Get the number of CPU cores
extern U32 getCpuCoresCount();

/// Print the backtrace
extern void printBacktrace();

/// @}

} // end namespace anki

#endif
