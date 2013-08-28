#include "anki/util/Assert.h"
#include "anki/util/System.h"
#include <cstdlib>
#include <iostream>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android/log.h>
#endif

namespace anki {

#if ANKI_DEBUG

//==============================================================================
void akassert(bool expr, const char* exprTxt, const char* file, int line,
	const char* func)
{
	if(!expr)
	{
#if ANKI_OS == ANKI_OS_LINUX
		std::cerr << "\033[1;31m(" << file << ":" << line << " "
			<< func << ") " << "Assertion failed: " << exprTxt << "\033[0m"
			<< std::endl;
#elif ANKI_OS == ANKI_OS_ANDROID
		__android_log_print(ANDROID_LOG_ERROR, "AnKi",
			"(%s:%d %s) Assertion failed: %s", file, line,
			func, exprTxt);
#else
		std::cerr << "(" << file << ":" << line << " "
			<< func << ") " << "Assertion failed: " << exprTxt
			<< std::endl;
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
