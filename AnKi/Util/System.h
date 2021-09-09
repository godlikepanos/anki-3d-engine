// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Function.h>
#include <AnKi/Util/String.h>
#include <ctime>

namespace anki
{

/// @addtogroup util_system
/// @{

/// Get the number of CPU cores
U32 getCpuCoresCount();

/// @internal
void backtraceInternal(const Function<void(CString)>& lambda);

/// Get a backtrace.
template<typename TFunc>
void backtrace(GenericMemoryPoolAllocator<U8> alloc, TFunc func)
{
	Function<void(CString)> f(alloc, func);
	backtraceInternal(f);
	f.destroy(alloc);
}

/// Return true if the engine is running from a terminal emulator.
Bool runningFromATerminal();

/// Return the local time in a thread safe way.
std::tm getLocalTime();
/// @}

} // end namespace anki
