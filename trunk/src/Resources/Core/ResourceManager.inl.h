#include "Exception.h"
#include "Logger.h"
#include "Assert.h"


//======================================================================================================================
// allocAndLoadRsrc                                                                                                    =
//======================================================================================================================
template<typename Type>
void ResourceManager::allocAndLoadRsrc(const char* filename, Type*& newInstance)
{
	newInstance = NULL;

	try
	{
		newInstance = new Type();
		newInstance->load(filename);
	}
	catch(std::exception& e)
	{
		//ERROR("fuckkkkkkkkkk " << e.what());

		/*if(newInstance != NULL)
		{
			delete newInstance;
		}*/
		
		throw EXCEPTION("Cannot load \"" + filename + "\": " + e.what());
	}
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Hook& ResourceManager::load(const char* filename)
{
	// Chose container
	typename Types<Type>::Container& c = choseContainer<Type>();

	// Find
	typename Types<Type>::Iterator it = find<Type>(filename, c);

	// If already loaded
	if(it != c.end())
	{
		++it->referenceCounter;
	}
	// else create new, load it and update the container
	else
	{
		typename Types<Type>::Hook* hook = NULL;

		try
		{
			hook = new typename Types<Type>::Hook;
			hook->uuid = filename;
			hook->referenceCounter = 1;
			allocAndLoadRsrc<Type>(filename, hook->resource);

			c.push_back(hook);

			it = c.end();
			--it;
		}
		catch(std::exception& e)
		{
			if(hook != NULL)
			{
				delete hook;
			}

			throw EXCEPTION("Cannot load \"" + filename + "\": " + e.what());
		}
	}

	return *it;
}


//======================================================================================================================
// unload                                                                                                              =
//======================================================================================================================
template<typename Type>
void ResourceManager::unload(const typename Types<Type>::Hook& hook)
{
	// Chose container
	typename Types<Type>::Container& c = choseContainer<Type>();

	// Find
	typename Types<Type>::Iterator it = find<Type>(hook.uuid.c_str(), c);

	// If not found
	if(it == c.end())
	{
		throw EXCEPTION("Resource hook incorrect (\"" + hook.uuid + "\")");
	}

	if(it->uuid != hook.uuid)
	{
		INFO(it->uuid << " " << hook.uuid);
	}

	ERROR(it->uuid << " " << hook.uuid << " " << it->resource << " " << hook.resource << " " << dummyTex.get());
	ASSERT(it->uuid == hook.uuid);
	ASSERT(it->referenceCounter == hook.referenceCounter);

	--it->referenceCounter;

	// Delete the resource
	if(it->referenceCounter == 0)
	{
		deallocRsrc(it->resource);
		c.erase(it);
	}
}


//======================================================================================================================
// find [char*]                                                                                                        =
//======================================================================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Iterator
ResourceManager::find(const char* filename, typename Types<Type>::Container& container)
{
	typename Types<Type>::Iterator it = container.begin();
	for(; it != container.end(); it++)
	{
		if(it->uuid == filename)
		{
			break;
		}
	}

	return it;
}
