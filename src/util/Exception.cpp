#include "anki/util/Exception.h"
#include <sstream>

namespace anki {

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
