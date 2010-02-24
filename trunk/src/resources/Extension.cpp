#include <dlfcn.h>
#include "Extension.h"


bool Extension::load( const char* filename )
{
	// load libary
	libHandle = dlopen( filename, RTLD_LAZY );
	if( libHandle == NULL )
	{
		ERROR( "File \"" << filename << "\": " << dlerror() );
		return false;
	}
	
	// get FooBar
	foobarPtr = (int(*)(void*))( dlsym(libHandle, "FooBar") );
	if( foobarPtr == NULL )
	{
		ERROR( "File \"" << filename << "\": \"FooBar\" entry symbol not found: " << dlerror() );
		return false;
	}
	
	return true;
}


void Extension::unload()
{
	DEBUG_ERR( libHandle==NULL || foobarPtr==NULL );
	dlclose( libHandle );
	libHandle = NULL;
	foobarPtr = NULL;
}
