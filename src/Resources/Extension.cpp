#include <dlfcn.h>
#include "Extension.h"
#include "Exception.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Extension::load(const char* /*filename*/)
{
	/*// load libary
	libHandle = dlopen(filename, RTLD_LAZY);
	if(libHandle == NULL)
	{
		throw EXCEPTION("File \"" + filename + "\": " + dlerror());
	}
	
	// get FooBar
	foobarPtr = (int(*)(void*))(dlsym(libHandle, "FooBar"));
	if(foobarPtr == NULL)
	{
		throw EXCEPTION("File \"" + filename + "\": \"FooBar\" entry symbol not found: " + dlerror());
	}*/
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
Extension::~Extension()
{
	/*//DEBUG_ERR(libHandle==NULL || foobarPtr==NULL);
	dlclose(libHandle);
	libHandle = NULL;
	foobarPtr = NULL;*/
}
