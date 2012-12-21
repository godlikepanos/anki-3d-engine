#include "anki/util/Exception.h"
#include <sstream>
#include <iostream>

// Instead of throwing abort. Its easier to debug
#define ANKI_ABORT_ON_THROW 0

namespace anki {

//==============================================================================
Exception::Exception(const char* error, const char* file,
	I line, const char* func)
{
	err = synthErr(error, file, line, func);

#if ANKI_ABORT_ON_THROW
	std::cerr << err << std::endl;
	abort();
#endif
}

//==============================================================================
Exception::Exception(const std::string& error, const char* file,
	I line, const char* func)
{
	err = synthErr(error.c_str(), file, line, func);

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
