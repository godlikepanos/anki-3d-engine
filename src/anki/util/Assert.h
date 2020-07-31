// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Common.h>

/// Assertion. Print an error and stop the debugger (if it runs through a debugger) and then abort.
#if !ANKI_EXTRA_CHECKS
#	define ANKI_ASSERT(x) ((void)0)
#	define ANKI_ENABLE_ASSERTS 0
#else

namespace anki
{

void akassert(const char* exprTxt, const char* file, int line, const char* func);

} // namespace anki

#	define ANKI_ASSERT(x) \
		do \
		{ \
			if(ANKI_UNLIKELY(!(x))) \
			{ \
				anki::akassert(#x, ANKI_FILE, __LINE__, ANKI_FUNC); \
				ANKI_UNREACHABLE(); \
			} \
		} while(0)
#	define ANKI_ENABLE_ASSERTS 1

#endif
