#ifndef _EXTENSION_H_
#define _EXTENSION_H_

#include "Common.h"
#include "Resource.h"


/// Extension @ref Resource resource
class Extension: public Resource
{
	private:
		void* libHandle;
		int(*foobarPtr)(void*);
	
	public:
		Extension(): libHandle(NULL), foobarPtr(NULL) {}
		~Extension() {}
		bool load(const char* filename);
		void unload();
		template<typename Type> int FooBar(Type* ptr) { DEBUG_ERR(foobarPtr==NULL); return (*foobarPtr)(reinterpret_cast<Type*>(ptr)); }
};


#endif
