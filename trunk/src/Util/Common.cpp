#include <iostream>
#include <GL/glew.h>
#include <GL/glu.h>
#include <boost/filesystem.hpp>
#include "Common.h"
#include "App.h"
#include "Util.h"


// for the colors and formating see http://www.dreamincode.net/forums/showtopic75171.htm
static const char* terminalColors [MT_NUM + 1] = {
	"\033[1;31;6m", // error
	"\033[1;33;6m", // warning
	"\033[1;31;6m", // fatal
	"\033[1;31;6m", // dbg error
	"\033[1;32;6m", // info
	"\033[0;;m" // default color
};


//======================================================================================================================
// getFunctionFromPrettyFunction                                                                                       =
//======================================================================================================================
/**
 * The function gets __PRETTY_FUNCTION__ and strips it to get only the function name with its namespace
 */
static string getFunctionFromPrettyFunction(const char* prettyFunction)
{
	string ret(prettyFunction);

	size_t index = ret.find("(");

	if (index != string::npos)
		ret.erase(index);

	index = ret.rfind(" ");
	if (index != string::npos)
		ret.erase(0, index + 1);

	return ret;
}


//======================================================================================================================
// msgPrefix                                                                                                           =
//======================================================================================================================
ostream& msgPrefix(MsgType msgType, const char* file, int line, const char* func)
{
	// select c stream
	ostream* cs;

	switch(msgType)
	{
		case MT_ERROR:
		case MT_WARNING:
		case MT_FATAL:
		case MT_DEBUG_ERR:
			cs = &cerr;
			break;

		case MT_INFO:
			cs = &cout;
			break;

		default:
			break;
	}


	// print terminal color
	if(app && app->isTerminalColoringEnabled())
	{
		(*cs) << terminalColors[msgType];
	}


	// print message info
	switch(msgType)
	{
		case MT_ERROR:
			(*cs) << "Error";
			break;

		case MT_WARNING:
			(*cs) << "Warning";
			break;

		case MT_FATAL:
			(*cs) << "Fatal";
			break;

		case MT_DEBUG_ERR:
			(*cs) << "Debug Err";
			break;

		case MT_INFO:
			(*cs) << "Info";
			break;

		default:
			break;
	}

	// print caller info
	(*cs) << " (" << filesystem::path(file).filename() << ":" << line << " " << getFunctionFromPrettyFunction(func) <<
	         "): ";

	return (*cs);
}


//======================================================================================================================
// msgSuffix                                                                                                           =
//======================================================================================================================
ostream& msgSuffix(ostream& cs)
{
	if(app && app->isTerminalColoringEnabled())
		cs << terminalColors[MT_NUM];

	cs << endl;
	return cs;
}


//======================================================================================================================
// msgSuffixFatal                                                                                                      =
//======================================================================================================================
ostream& msgSuffixFatal(ostream& cs)
{
	msgSuffix(cs);
	::exit(1);
	return cs;
}


//======================================================================================================================
// msgGlError                                                                                                          =
//======================================================================================================================
bool msgGlError(const char* file, int line, const char* func)
{
	GLenum errId = glGetError();
	if(errId == GL_NO_ERROR) return true;
	msgPrefix(MT_ERROR, file, line, func) << "OpenGL Err: " << gluErrorString(errId) << msgSuffix;
	return false;
}

