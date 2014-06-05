// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Extension.h"
#include "anki/util/Exception.h"
#include <dlfcn.h>

namespace anki {

//==============================================================================
// load                                                                        =
//==============================================================================
void Extension::load(const char* /*filename*/)
{
	/*// load libary
	libHandle = dlopen(filename, RTLD_LAZY);
	if(libHandle == NULL)
	{
		throw ANKI_EXCEPTION("File \"" + filename + "\": " + dlerror());
	}
	
	// get FooBar
	foobarPtr = (int(*)(void*))(dlsym(libHandle, "FooBar"));
	if(foobarPtr == NULL)
	{
		throw ANKI_EXCEPTION("File \"" + filename +
			"\": \"FooBar\" entry symbol not found: " + dlerror());
	}*/
}

//==============================================================================
// Destructor                                                                  =
//==============================================================================
Extension::~Extension()
{
	/*//DEBUG_ERR(libHandle==NULL || foobarPtr==NULL);
	dlclose(libHandle);
	libHandle = NULL;
	foobarPtr = NULL;*/
}

} // end namespace
