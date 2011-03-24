#ifndef ASSERT_H
#define ASSERT_H

#include <iostream>
#include <cstdlib>


/// Assertion. Print an error and stop the debugger and then abort
#if defined(NDEBUG)
	#define ASSERT(x) ((void)0)
#else
	#define ASSERT(x) \
		if(!(x)) { \
			std::cerr << "(" << __FILE__ << ":" << __LINE__ << " " << __func__ << ") " << \
			             "Assertion failed: " << #x << std::endl; \
			asm("int $3"); \
			abort(); \
		}
#endif


#endif
