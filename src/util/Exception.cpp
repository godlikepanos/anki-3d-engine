#include "anki/util/Exception.h"
#include <sstream>

// Instead of throwing abort. Its easier to debug
#define ANKI_ABORT_ON_THROW 1

namespace anki {

//==============================================================================
Exception::Exception(const char* error, const char* file,
	int line, const char* func)
{
	err = synthErr(error, file, line, func);

#if defined(ANKI_ABORT_ON_THROW)
	abort();
#endif
}

//==============================================================================
Exception::Exception(const Exception& e)
	: err(e.err)
{
#if defined(ANKI_ABORT_ON_THROW)
	abort();
#endif
}

//==============================================================================
std::string Exception::synthErr(const char* error, const char* file,
	int line, const char* func)
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

} // end namespace
