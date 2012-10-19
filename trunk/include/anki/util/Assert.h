#ifndef ANKI_UTIL_ASSERT_H
#define ANKI_UTIL_ASSERT_H

#include "anki/Config.h"

/// Assertion. Print an error and stop the debugger (if it runs through a
/// debugger) and then abort
#if !ANKI_DEBUG
#	define ANKI_ASSERT(x) ((void)0)
#	define ANKI_ASSERTS_ENABLED 0
#else

namespace anki {

/// Its separate so we will not include iostream
extern void akassert(bool expr, const char* exprTxt, const char* file,
	int line, const char* func);

} // end namespace

#	define ANKI_ASSERT(x) akassert((x), #x, ANKI_FILE, __LINE__, ANKI_FUNC)
#	define ANKI_ASSERTS_ENABLED 1

#endif

#endif
