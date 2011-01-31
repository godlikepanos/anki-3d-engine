#include <cstdio>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include "Exception.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Exception::init(const char* err_, const char* file, int line, const char* func)
{
	err = std::string("\n(") + file + ":" + boost::lexical_cast<std::string>(line) + " " + func + ") " + err_;
}
