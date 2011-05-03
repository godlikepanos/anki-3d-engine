#include <boost/lexical_cast.hpp>
#include "ScannerException.h"


namespace Scanner {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Exception::Exception(const std::string& err, const std::string& scriptFilename_, int scriptLineNmbr_,
                     const char* file, int line, const char* func):
	BaseClass(err, file, line, func),
	scriptFilename(scriptFilename_),
	scriptLineNmbr(scriptLineNmbr_)
{}


//======================================================================================================================
// Copy constructor                                                                                                    =
//======================================================================================================================
Exception::Exception(const Exception& e):
	BaseClass(e),
	scriptFilename(e.scriptFilename),
	scriptLineNmbr(e.scriptLineNmbr)
{}


//======================================================================================================================
// what                                                                                                                =
//======================================================================================================================
const char* Exception::what() const throw()
{
	errWhat = "\n" + getInfoStr() + " Scanner exception (" + scriptFilename + ':' +
	          boost::lexical_cast<std::string>(scriptLineNmbr) + ") " + err;
	return errWhat.c_str();
}


} // end namespace
