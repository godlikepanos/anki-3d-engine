#include <cstdlib>
#include <iostream>


namespace anki {


//==============================================================================
void akassert(bool expr, const char* exprTxt, const char* file, int line,
	const char* func)
{
	if(!expr)
	{
		std::cerr << "(" << file << ":" << line << " " <<
			func << ") " << "Assertion failed: " << exprTxt << std::endl;

		asm("int $3");
		abort();
	}
}


} // end namespace
