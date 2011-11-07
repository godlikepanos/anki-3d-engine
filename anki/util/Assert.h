#ifndef ANKI_UTIL_ASSERT_H
#define ANKI_UTIL_ASSERT_H

#include <iostream>
#include <cstdlib>


/// Assertion. Print an error and stop the debugger (if it runs through a
/// debugger) and then abort
#if defined(NDEBUG)
#	define ANKI_ASSERT(x) ((void)0)
#else
#	define ANKI_ASSERT(x) \
		if(!(x)) { \
			std::cerr << "(" << __FILE__ << ":" << __LINE__ << " " << \
				__func__ << ") " << \
				"Assertion failed: " << #x << std::endl; \
			asm("int $3"); \
			abort(); \
		}
#endif


#endif
