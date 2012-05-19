#include <cstdlib>
#include <iostream>

namespace anki {

#if !defined(NDEBUG)

//==============================================================================
void akassert(bool expr, const char* exprTxt, const char* file, int line,
	const char* func)
{
	if(!expr)
	{
		std::cerr << "(" << file << ":" << line << " " 
			<< func << ") " << "Assertion failed: " << exprTxt << std::endl;

		asm("int $3");
		abort();
	}
}

#endif

} // end namespace
