// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki {

/// @addtogroup util_system
/// @{

/// Get the number of CPU cores
extern U32 getCpuCoresCount();

/// Print the backtrace
extern void printBacktrace();

/// @}

} // end namespace anki

