#ifndef ANKI_RESOURCE_EXTENSION_H
#define ANKI_RESOURCE_EXTENSION_H

#include "anki/util/StdTypes.h"


namespace anki {


/// Extension @ref Resource resource
class Extension
{
private:
	void* libHandle;
	int(*foobarPtr)(void*);

public:
	Extension();
	~Extension();
	void load(const char* filename);
	/*template<typename Type>
	int FooBar(Type* ptr) { DEBUG_ERR(foobarPtr==NULL);
		return (*foobarPtr)(reinterpret_cast<Type*>(ptr)); }*/
};


inline Extension::Extension()
	: libHandle(NULL), foobarPtr(NULL)
{}


} // end namespace


#endif
