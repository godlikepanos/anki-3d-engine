#ifndef EXTENSION_H
#define EXTENSION_H

#include "Resource.h"


/// Extension @ref Resource resource
class Extension: public Resource
{
	private:
		void* libHandle;
		int(*foobarPtr)(void*);
	
	public:
		Extension();
		~Extension();
		void load(const char* filename);
		/*template<typename Type>
		int FooBar(Type* ptr) { DEBUG_ERR(foobarPtr==NULL); return (*foobarPtr)(reinterpret_cast<Type*>(ptr)); }*/
};


inline Extension::Extension():
	Resource(RT_EXTENSION),
	libHandle(NULL),
	foobarPtr(NULL)
{}

#endif
