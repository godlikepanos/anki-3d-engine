// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <ctime>

namespace anki
{

/// @addtogroup util_system
/// @{

/// Get the number of CPU cores
U32 getCpuCoresCount();

/// Backtrace walker.
class BackTraceWalker
{
public:
	virtual ~BackTraceWalker()
	{
	}

	virtual void operator()(const char* symbol) = 0;
};

void getBacktrace(BackTraceWalker& walker);

/// Return true if the engine is running from a terminal emulator.
Bool runningFromATerminal();

/// Return the local time in a thread safe way.
std::tm getLocalTime();
/// @}

} // end namespace anki
