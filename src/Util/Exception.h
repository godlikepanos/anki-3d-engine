#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>


/// Custom exception that takes file, line and function that throwed it. Throw it using the EXCEPTION macro
class Exception: public std::exception
{
	public:
		/// Constructor #1
		Exception(const char* err_, const char* file, int line, const char* func) {init(err_, file, line, func);}

		/// Constructor #2
		Exception(const std::string& err_, const char* file, int line, const char* func);

		/// Destructor. Do nothing
		~Exception() throw() {}

		/// Return the error code
		const char* what() const throw() {return err.c_str();}

	private:
		std::string err;

		/// Common construction code
		void init(const char* err_, const char* file, int line, const char* func);
};


inline Exception::Exception(const std::string& err_, const char* file, int line, const char* func)
{
	init(err_.c_str(), file, line, func);
}


// A few macros

#define EXCEPTION(x) Exception(std::string() + x, __FILE__, __LINE__, __func__)

#if DEBUG_ENABLED == 1
	#define RASSERT_THROW_EXCEPTION(x) \
		if(x) \
			throw EXCEPTION("Reverse assertion failed: " #x)
#else
	#define RASSERT_THROW_EXCEPTION(x)
#endif



#endif
