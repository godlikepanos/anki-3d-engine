#include <cstdio>
#include <cstring>
#include "Exception.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Exception::init(const char* err_, const char* file, int line, const char* func)
{
	char tmpStr[1024];
	sprintf(tmpStr, "(%s:%d %s) Exception: %s", file, line, func, err_);
	//sprintf(tmpStr, "%s", err_);
	err = tmpStr;
}
