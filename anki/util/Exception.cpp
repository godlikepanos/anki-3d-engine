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
Exception::Exception(const char* err, const std::exception& e,
	const char* file, int line, const char* func)
{
	std::stringstream ss;
	ss << synthErr(error, file, line, func) << ". From here:\n" << e.what();
	err = ss.str();
}


} // end namespace
