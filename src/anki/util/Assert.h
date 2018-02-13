// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Common.h>

/// Assertion. Print an error and stop the debugger (if it runs through a debugger) and then abort.
#if !ANKI_EXTRA_CHECKS
#	define ANKI_ASSERT(x) ((void)0)
#	define ANKI_ASSERTS_ENABLED 0
#else

namespace anki
{

void akassert(const char* exprTxt, const char* file, int line, const char* func);

} // end namespace

#	define ANKI_ASSERT(x) \
		do \
		{ \
			if(!(x)) \
			{ \
				anki::akassert(#x, ANKI_FILE, __LINE__, ANKI_FUNC); \
			} \
		} while(0)
#	define ANKI_ASSERTS_ENABLED 1

#endif
