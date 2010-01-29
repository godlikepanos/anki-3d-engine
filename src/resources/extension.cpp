#include <dlfcn.h>
#include "extension.h"


bool extension_t::Load( const char* filename )
{
	// load libary
	lib_handle = dlopen( filename, RTLD_LAZY );
	if( lib_handle == NULL ) 
	{
		ERROR( "File \"" << filename << "\": " << dlerror() );
		return false;
	}
	
	// get FooBar
	foobar_ptr = (int(*)(void*))( dlsym(lib_handle, "FooBar") );
	if( foobar_ptr == NULL )  
	{
		ERROR( "File \"" << filename << "\": \"FooBar\" entry symbol not found: " << dlerror() );
		return false;
	}
	
	return true;
}


void extension_t::Unload()
{
	DEBUG_ERR( lib_handle==NULL || foobar_ptr==NULL );
	dlclose( lib_handle );
	lib_handle = NULL;
	foobar_ptr = NULL;
}
