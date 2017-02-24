// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

/// @addtogroup util_system
/// @{

/// Get the number of CPU cores
U32 getCpuCoresCount();

/// Visit the program stack.
class BackTraceWalker
{
public:
	BackTraceWalker(U stackSize = 50)
		: m_stackSize(stackSize)
	{
	}

	virtual ~BackTraceWalker()
	{
	}

	virtual void operator()(const char* symbol) = 0;

	void exec();

private:
	U m_stackSize;
};
/// @}

} // end namespace anki
