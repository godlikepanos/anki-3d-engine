#ifndef ASSERT_H
#define ASSERT_H

#include <iostream>
#include <cstdlib>


/// Assertion. Print an error and stop the debugger and then abort
#if DEBUG_ENABLED == 1
	#define ASSERT(x) \
		if(!(x)) { \
			std::cerr << "(" << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << ") " << \
			             "Assertion failed: " << #x << std::endl; \
			asm("int $3"); \
			abort(); \
		}
#else
	#define ASSERT(x) ((void)0)
#endif


#endif
