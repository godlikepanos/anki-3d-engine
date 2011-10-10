#include "anki/util/Exception.h"
#include <sstream>


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Exception::Exception(const std::string& err_, const char* file_, int line_, 
	const char* func_)
:	err(err_),
	file(file_),
	line(line_),
	func(func_)
{}


//==============================================================================
// Copy constructor                                                            =
//==============================================================================
Exception::Exception(const Exception& e):
	err(e.err),
	file(e.file),
	line(e.line),
	func(e.func)
{}


//==============================================================================
// getInfoStr                                                                  =
//==============================================================================
std::string Exception::getInfoStr() const
{
	std::stringstream ss;
	
	ss << '(' << file << ':' << line << ' ' << func << ')';
	return ss.str();
}


//==============================================================================
// what                                                                        =
//==============================================================================
const char* Exception::what() const throw()
{
	std::stringstream ss;
	ss << "\n" << getInfoStr() << " AnKi exception: " << err;
	errWhat = ss.str();
	return errWhat.c_str();
}
