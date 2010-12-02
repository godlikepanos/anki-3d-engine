#include <cstdio>
#include <cstring>
#include "Exception.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Exception::init(const char* err_, const char* file, int line, const char* func)
{
	char tmpStr[1024];
	sprintf(tmpStr, "\n(%s:%d %s) %s", file, line, func, err_);
	//sprintf(tmpStr, "%s", err_);
	err = tmpStr;
}
