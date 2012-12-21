#ifndef ANKI_UTIL_EXCEPTION_H
#define ANKI_UTIL_EXCEPTION_H

#include "anki/Config.h"
#include "anki/util/StdTypes.h"
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
	explicit Exception(const char* error, const char* file = "unknown",
		I line = -1, const char* func = "unknown");

	/// Constructor 2
	explicit Exception(const std::string& error, const char* file = "unknown",
		I line = -1, const char* func = "unknown");

	/// Copy constructor
	Exception(const Exception& e);

	/// Destructor. Do nothing
	~Exception() throw()
	{}

	/// For re-throws
	/// Usage:
	/// @code
	/// catch(std::exception& e) {
	/// 	throw Exception("message", ...) << e;
	/// }
	/// @endcode
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
		I line, const char* func);
};

} // end namespace anki

/// Macro for easy throwing
#define ANKI_EXCEPTION(x) \
	Exception(std::string() + x, ANKI_FILE, __LINE__, ANKI_FUNC)

#endif
