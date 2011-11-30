#ifndef ANKI_UTIL_EXCEPTION_H
#define ANKI_UTIL_EXCEPTION_H

#include <exception>
#include <string>


namespace anki {


/// Custom exception that takes file, line and function that throw it. Throw 
/// it using the ANKI_EXCEPTION macro
class Exception: public std::exception
{
public:
	/// Constructor
	Exception(const char* err, const char* file = "unknown",
		int line = -1, const char* func = "unknown")
	{
		synthErr(err, file, line, func);
	}

	/// Copy constructor
	Exception(const Exception& e)
		: err(e.err)
	{}

	/// For re-throws
	Exception(const char* err, const std::exception& e,
		const char* file = "unknown",
		int line = -1, const char* func = "unknown");

	/// Destructor. Do nothing
	~Exception() throw()
	{}

	/// Implements std::exception::what()
	const char* what() const throw()
	{
		return err.c_str();
	}

private:
	std::string err;

	/// XXX
	std::string synthErr(const char* error, const char* file,
		int line, const char* func);
};


} // end namespace


//==============================================================================
// Macros                                                                      =
//==============================================================================

#define ANKI_EXCEPTION(x) Exception((std::string() + x).c_str(), \
	__FILE__, __LINE__, __func__)

#define ANKI_EXCEPTION_R(x, e) Exception((std::string() + x).c_str(), e, \
	__FILE__, __LINE__, __func__)


#endif
