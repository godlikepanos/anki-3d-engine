#ifndef ANKI_UTIL_EXCEPTION_H
#define ANKI_UTIL_EXCEPTION_H

#include <exception>
#include <string>

namespace anki {

/// Mother of all AnKi exceptions.
///
/// Custom exception that takes file, line and function that throw it. Throw 
/// it using the ANKI_EXCEPTION macro
class Exception: public std::exception
{
public:
	/// Constructor
	Exception(const char* error, const char* file = "unknown",
		int line = -1, const char* func = "unknown")
	{
		err = synthErr(error, file, line, func);
	}

	/// Copy constructor
	Exception(const Exception& e)
		: err(e.err)
	{}

	/// Destructor. Do nothing
	~Exception() throw()
	{}

	/// For re-throws
	Exception operator<<(const std::exception& e) const;

	/// Implements std::exception::what()
	const char* what() const throw()
	{
		return err.c_str();
	}

private:
	std::string err;

	/// Synthesize the error string
	static std::string synthErr(const char* error, const char* file,
		int line, const char* func);
};

} // end namespace

/// Macro for easy throwing
#define ANKI_EXCEPTION(x) Exception((std::string() + x).c_str(), \
	__FILE__, __LINE__, __func__)

#endif
