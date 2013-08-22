#include "anki/resource/ResourceManager.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
template<typename Type>
void TypeResourceManager<Type>::
	allocAndLoadRsrc(const char* filename, Type*& newInstance)
{
	newInstance = nullptr;
	std::string newFname;

	// Alloc
	try
	{
		newInstance = new Type();
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Constructor failed for: " + filename) << e;
	}

	// Load
	try
	{
		newFname = 
			ResourceManagerSingleton::get().fixResourcePath(filename);

		newInstance->load(newFname.c_str());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot load: " + newFname) << e;
	}
}

//==============================================================================
template<typename Type>
typename TypeResourceManager<Type>::Hook& TypeResourceManager<Type>::
	load(const char* filename)
{
	Iterator it = find(filename);

	// If already loaded
	if(it != hooks.end())
	{
		++(*it)->referenceCounter;
		return *(*it);
	}
	// else create new, load it and update the container
	else
	{
		Hook* hook = nullptr;
		hook = new Hook;
		hook->uuid = filename;
		hook->referenceCounter = 1;

		try
		{
			allocAndLoadRsrc(filename, hook->resource);
		}
		catch(std::exception& e)
		{
			delete hook;
			throw ANKI_EXCEPTION("Cannot load: " + filename) << e;
		}

		hooks.push_back(hook);
		return *hook;
	}
}

//==============================================================================
template<typename Type>
void TypeResourceManager<Type>::deallocRsrc(Type* rsrc)
{
	propperDelete(rsrc);
}

//==============================================================================
template<typename Type>
void TypeResourceManager<Type>::unload(const Hook& hook)
{
	// Find
	Iterator it = find(hook.uuid.c_str());

	// If not found
	if(it == hooks.end())
	{
		throw ANKI_EXCEPTION("Resource hook not found: " + hook.uuid);
	}

	ANKI_ASSERT(*(*it) == hook);
	ANKI_ASSERT((*it)->referenceCounter > 0);

	--(*it)->referenceCounter;

	// Delete the resource
	if((*it)->referenceCounter == 0)
	{
		deallocRsrc((*it)->resource);
		hooks.erase(it);
	}
}

//==============================================================================
template<typename Type>
typename TypeResourceManager<Type>::Iterator TypeResourceManager<Type>::
	find(const char* filename)
{
	Iterator it = hooks.begin();
	for(; it != hooks.end(); it++)
	{
		if((*it)->uuid == filename)
		{
			break;
		}
	}

	return it;
}

} // end namespace anki
