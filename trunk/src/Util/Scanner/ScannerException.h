#ifndef SCANNER_EXCEPTION_H
#define SCANNER_EXCEPTION_H

#include "Exception.h"


namespace Scanner {


class Exception: public ::Exception
{
	public:
		typedef ::Exception BaseClass;

		/// Constructor
		Exception(const std::string& err, const std::string& scriptFilename, int scriptLineNmbr,
				  const char* file = "unknown", int line = -1, const char* func = "unknown");

		/// Copy constructor
		Exception(const Exception& e);

		/// Destructor. Do nothing
		~Exception() throw() {}

		/// Return the error code
		virtual const char* what() const throw();

	private:
		std::string scriptFilename;
		int scriptLineNmbr;
};


} // End namespace


#endif
