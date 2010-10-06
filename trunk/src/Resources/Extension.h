#ifndef ENSION_H_
#define ENSION_H_

#include "Common.h"
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
		template<typename Type>
		int FooBar(Type* ptr) { DEBUG_ERR(foobarPtr==NULL); return (*foobarPtr)(reinterpret_cast<Type*>(ptr)); }
};


inline Extension::Extension():
	Resource(RT_EXTENSION),
	libHandle(NULL),
	foobarPtr(NULL)
{}

#endif
