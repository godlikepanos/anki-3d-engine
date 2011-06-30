#ifndef SCANNER_EXCEPTION_H
#define SCANNER_EXCEPTION_H

#include <exception>
#include <string>


namespace Scanner {


class Exception: public std::exception
{
	public:
		/// Constructor
		Exception(const std::string& err, int errNo,
			const std::string& scriptFilename, int scriptLineNmbr);

		/// Copy constructor
		Exception(const Exception& e);

		/// Destructor. Do nothing
		~Exception() throw() {}

		/// Return the error code
		virtual const char* what() const throw();

	private:
		std::string error;
		int errNo; ///< Error number
		std::string scriptFilename;
		int scriptLineNmbr;
		mutable std::string errWhat;
};


} // End namespace


#endif
