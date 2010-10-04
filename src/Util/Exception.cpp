#include <cstdio>
#include <cstring>
#include "Exception.h"


using namespace std;


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Exception::init(const char* err_, const char* file, int line, const char* func)
{
	char tmpStr[1024];
	sprintf(tmpStr, "Exception (%s:%d %s): %s", file, line, func, err_);
	err = tmpStr;
}
