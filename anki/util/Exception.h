#ifndef ANKI_UTIL_EXCEPTION_H
#define ANKI_UTIL_EXCEPTION_H

#include <exception>
#include <string>


/// Custom exception that takes file, line and function that throw it. Throw 
/// it using the ANKI_EXCEPTION macro
class Exception: public std::exception
{
	public:
		/// Constructor
		Exception(const std::string& err, const char* file = "unknown", 
			int line = -1, const char* func = "unknown");

		/// Copy constructor
		Exception(const Exception& e);

		/// Destructor. Do nothing
		~Exception() throw() {}

		/// @name Accessors
		/// @{
		const std::string& getTheError() const {return err;}
		/// @}

		/// Return the error code formated with the other info
		virtual const char* what() const throw();

	protected:
		std::string err;
		mutable std::string errWhat;
		const char* file;
		int line;
		const char* func;

		std::string getInfoStr() const;
};


//==============================================================================
// Macros                                                                      =
//==============================================================================

#define ANKI_EXCEPTION(x) Exception(std::string() + x, \
	__FILE__, __LINE__, __func__)


#endif
