#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <sstream>


/// Custom exception that takes. Throw it used the THROW_EXCEPTION macro
class Exception: public std::exception
{
	public:
		Exception(const char* err_, const char* file, int line, const char* func);
		~Exception() throw() {}
		const char* what() const throw() {return err.c_str();}

	private:
		std::string err;
};


inline Exception::Exception(const char* err_, const char* file, int line, const char* func)
{
	using namespace std;
	stringstream ss;
	ss << "Exception (" << file << ":" << line << " " << func << "): " << err_;
	err = ss.str();
}


#define THROW_EXCEPTION(x) throw Exception((x), __FILE__, __LINE__, __func__)

#if DEBUG_ENABLED == 1
	#define RASSERT_THROW_EXCEPTION(x) \
		if(x) \
			THROW_EXCEPTION("Reverse assertion failed: " #x)
#else
	#define RASSERT_THROW_EXCEPTION(x)
#endif



#endif
