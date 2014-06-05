// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_EXTENSION_H
#define ANKI_RESOURCE_EXTENSION_H

namespace anki {

/// Extension resource
class Extension
{
public:
	Extension();
	~Extension();
	void load(const char* filename);
	/*template<typename Type>
	int FooBar(Type* ptr) { DEBUG_ERR(foobarPtr==NULL);
		return (*foobarPtr)(reinterpret_cast<Type*>(ptr)); }*/

private:
	void* libHandle = nullptr;
	int(*foobarPtr)(void*) = nullptr;
};

} // end namespace

#endif
