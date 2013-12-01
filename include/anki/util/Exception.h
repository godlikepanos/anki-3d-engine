#ifndef ANKI_UTIL_EXCEPTION_H
#define ANKI_UTIL_EXCEPTION_H

#include "anki/Config.h"
#include "anki/util/StdTypes.h"
#include <exception>
#include <string>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup misc
/// @{

/// Mother of all AnKi exceptions.
///
/// Custom exception that takes file, line and function that throw it. Throw 
/// it using the ANKI_EXCEPTION macro
class Exception: public std::exception
{
public:
	/// Constructor
	explicit Exception(const char* file, I line, const char* func, 
		const char* errorFmt, ...) throw();

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

/// Macro for easy throwing
#define ANKI_EXCEPTION(...) \
	Exception(ANKI_FILE, __LINE__, ANKI_FUNC, __VA_ARGS__)

/// @}
/// @}

} // end namespace anki

#endif
