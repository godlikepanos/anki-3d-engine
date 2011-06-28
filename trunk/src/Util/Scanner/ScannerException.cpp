#include <boost/lexical_cast.hpp>
#include "ScannerException.h"


namespace Scanner {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Exception::Exception(const std::string& err, int errNo_, const std::string& scriptFilename_, int scriptLineNmbr_):
	error(err),
	errNo(errNo_),
	scriptFilename(scriptFilename_),
	scriptLineNmbr(scriptLineNmbr_)
{}


//======================================================================================================================
// Copy constructor                                                                                                    =
//======================================================================================================================
Exception::Exception(const Exception& e):
	error(e.error),
	errNo(e.errNo),
	scriptFilename(e.scriptFilename),
	scriptLineNmbr(e.scriptLineNmbr)
{}


//======================================================================================================================
// what                                                                                                                =
//======================================================================================================================
const char* Exception::what() const throw()
{
	errWhat = "Scanner exception #" + boost::lexical_cast<std::string>(errNo) + " (" + scriptFilename + ':' +
		boost::lexical_cast<std::string>(scriptLineNmbr) + ") " + error;
	return errWhat.c_str();
}


} // end namespace
