#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <sstream>


/// Custom exception that takes. Throw it used the THROW_EXCEPTION macro
class Exception: public std::exception
{
	public:
		Exception(const char* err_, const char* file, int line, const char* func) {init(err_, file, line, func);}
		Exception(const std::string& err_, const char* file, int line, const char* func);
		~Exception() throw() {}
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


#define THROW_EXCEPTION(x) throw Exception(std::string() + x, __FILE__, __LINE__, __func__)

#if DEBUG_ENABLED == 1
	#define RASSERT_THROW_EXCEPTION(x) \
		if(x) \
			THROW_EXCEPTION("Reverse assertion failed: " #x)
#else
	#define RASSERT_THROW_EXCEPTION(x)
#endif



#endif
