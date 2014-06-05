// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Assert.h"
#include "anki/util/System.h"
#include <cstdlib>
#include <cstdio>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki {

#if ANKI_ASSERTIONS

//==============================================================================
void akassert(bool expr, const char* exprTxt, const char* file, int line,
	const char* func)
{
	if(!expr)
	{
#if ANKI_OS == ANKI_OS_LINUX
		fprintf(stderr, "\033[1;31m(%s:%d %s) Assertion failed: %s\033[0m\n",
			file, line , func, exprTxt);
#elif ANKI_OS == ANKI_OS_ANDROID
		__android_log_print(ANDROID_LOG_ERROR, "AnKi",
			"(%s:%d %s) Assertion failed: %s", file, line,
			func, exprTxt);
#else
		fprintf(stderr, "(%s:%d %s) Assertion failed: %s\n",
			file, line , func, exprTxt);
#endif

#if ANKI_CPU_ARCH == ANKI_CPU_ARCH_INTEL
		asm("int $3");
#endif
		printBacktrace();
		abort();
	}
}

#endif

} // end namespace anki
