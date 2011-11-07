#include "anki/resource/ResourceManager.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <iostream>


namespace anki {


//==============================================================================
template<typename Type>
void ResourceManager<Type>::allocAndLoadRsrc(
	const char* filename, Type*& newInstance)
{
	newInstance = NULL;

	// Alloc
	try
	{
		newInstance = new Type();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Constructor failed for \"" + filename +
			"\": " + e.what());
	}

	// Load
	try
	{
		newInstance->load(filename);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot load \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
template<typename Type>
typename ResourceManager<Type>::Hook& ResourceManager<Type>::load(
	const char* filename)
{
	Iterator it = find(filename);

	// If already loaded
	if(it != hooks.end())
	{
		++it->referenceCounter;
		return *it;
	}
	// else create new, load it and update the container
	else
	{
		Hook* hook = NULL;
		hook = new Hook;
		hook->uuid = filename;
		hook->referenceCounter = 1;

		try
		{
			allocAndLoadRsrc(filename, hook->resource);
		}
		catch(std::exception& e)
		{
			if(hook != NULL)
			{
				delete hook;
			}

			throw ANKI_EXCEPTION("Cannot load \"" +
				filename + "\": " + e.what());
		}

		hooks.push_back(hook);
		return *hook;
	}
}


//==============================================================================
template<typename Type>
void ResourceManager<Type>::deallocRsrc(Type* rsrc)
{
	typedef char TypeMustBeComplete[sizeof(Type) ? 1 : -1];
    (void) sizeof(TypeMustBeComplete);

	delete rsrc;
}


//==============================================================================
template<typename Type>
void ResourceManager<Type>::unload(const Hook& hook)
{
	// Find
	Iterator it = find(hook.uuid.c_str());

	// If not found
	if(it == hooks.end())
	{
		throw ANKI_EXCEPTION("Resource hook incorrect (\"" +
			hook.uuid + "\")");
	}

	assert(*it == hook);

	--it->referenceCounter;

	// Delete the resource
	if(it->referenceCounter == 0)
	{
		deallocRsrc(it->resource);
		hooks.erase(it);
	}
}


//==============================================================================
template<typename Type>
typename ResourceManager<Type>::Iterator ResourceManager<Type>::find(
	const char* filename)
{
	Iterator it = hooks.begin();
	for(; it != hooks.end(); it++)
	{
		if(it->uuid == filename)
		{
			break;
		}
	}

	return it;
}


} // end namespace
