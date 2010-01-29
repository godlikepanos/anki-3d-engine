#ifndef _EXTENSION_H_
#define _EXTENSION_H_

#include "common.h"
#include "resource.h"


/// Extension resource
class extension_t: public resource_t
{
	private:
		void* lib_handle;
		int(*foobar_ptr)(void*);
	
	public:
		extension_t(): lib_handle(NULL), foobar_ptr(NULL) {}
		~extension_t() {}
		bool Load( const char* filename );
		void Unload();
		template<typename type_t> int FooBar( type_t* ptr ) { DEBUG_ERR(foobar_ptr==NULL); return (*foobar_ptr)( reinterpret_cast<type_t*>(ptr) ); }
};


#endif
