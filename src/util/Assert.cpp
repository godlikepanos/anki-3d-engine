#include "anki/util/Assert.h"
#include <cstdlib>
#include <iostream>

namespace anki {

#if ANKI_DEBUG

//==============================================================================
void akassert(bool expr, const char* exprTxt, const char* file, int line,
	const char* func)
{
	if(!expr)
	{
		std::cerr << "\033[1;31m(" << file << ":" << line << " "
			<< func << ") " << "Assertion failed: " << exprTxt << "\033[0m"
			<< std::endl;

		asm("int $3");
		abort();
	}
}

#endif

} // end namespace anki
