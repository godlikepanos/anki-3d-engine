#include "anki/util/Exception.h"
#include <sstream>
#include <iostream>

// Instead of throwing abort. Its easier to debug
#define ANKI_ABORT_ON_THROW 0

namespace anki {

//==============================================================================
Exception::Exception(const char* file, I line, const char* func, 
	const char* fmt, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	err = synthErr(buffer, file, line, func);

#if ANKI_ABORT_ON_THROW
	std::cerr << err << std::endl;
	abort();
#endif
}

//==============================================================================
Exception::Exception(const Exception& e)
	: err(e.err)
{
#if ANKI_ABORT_ON_THROW
	std::cerr << err << std::endl;
	abort();
#endif
}

//==============================================================================
std::string Exception::synthErr(const char* error, const char* file,
	I line, const char* func)
{
	std::stringstream ss;
	ss << "(" << file << ':' << line << ' ' << func << ") " << error;
	return ss.str();
}

//==============================================================================
Exception Exception::operator<<(const std::exception& e) const
{
	Exception out(*this);
	out.err += "\nFrom: ";
	out.err += e.what();
	return out;
}

} // end namespace anki
