#include "anki/util/Exception.h"
#include "anki/util/Vector.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdarg>

// Instead of throwing abort. Its easier to debug
#define ANKI_ABORT_ON_THROW 0

namespace anki {

//==============================================================================
Exception::Exception(const char* file, I line, const char* func, 
	const char* fmt, ...) throw()
{
	char buffer[1024];
	const char* out = &buffer[0];
	Vector<char> largeStr;
	va_list args;

	va_start(args, fmt);
	I len = vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	if(len < 0)
	{
		// Error in vsnprintf()
		strcpy(buffer, "Error when throwing exception. vsnprintf() failed");
	}
	else if((PtrSize)len >= sizeof(buffer))
	{
		// The buffer is not big enough

		// Create a huge string
		largeStr.resize(len + 1, '-');
		out = &largeStr[0];

		va_start(args, fmt);
		len = vsnprintf(&largeStr[0], largeStr.size(), fmt, args);
		va_end(args);

		ANKI_ASSERT(len < (I)largeStr.size());
	}

	err = synthErr(out, file, line, func);

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
