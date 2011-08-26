#include <boost/lexical_cast.hpp>
#include "util/Exception.h"


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
	return std::string("(") + file + ":" + 
		boost::lexical_cast<std::string>(line) + " " + func + ")";
}


//==============================================================================
// what                                                                        =
//==============================================================================
const char* Exception::what() const throw()
{
	errWhat = "\n" + getInfoStr() + " " + err;
	return errWhat.c_str();
}
