#include "ResourceContainer.h"


//======================================================================================================================
// findByName                                                                                                          =
//======================================================================================================================
template<typename Type>
typename ResourceContainer<Type>::Iterator ResourceContainer<Type>::findByName(const char* name)
{
	Iterator it = BaseClass::begin();
	while(it != BaseClass::end())
	{
		if((*it).name == name)
			return it;
		++it;
	}

	return it;
}


//======================================================================================================================
// findByNameAndPath                                                                                                   =
//======================================================================================================================
template<typename Type>
typename ResourceContainer<Type>::Iterator ResourceContainer<Type>::findByNameAndPath(const char* name,
                                                                                      const char* path)
{
	Iterator it = BaseClass::begin();
	while(it != BaseClass::end())
	{
		if((*it)->name == name && (*it)->path == path)
			return it;
		++it;
	}

	return it;
}


//======================================================================================================================
// findByPtr                                                                                                           =
//======================================================================================================================
template<typename Type>
typename ResourceContainer<Type>::Iterator ResourceContainer<Type>::findByPtr(Type* ptr)
{
	Iterator it = BaseClass::begin();
	while(it != BaseClass::end())
	{
		if(ptr == (*it))
			return it;
		++it;
	}

	return it;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
template<typename Type>
Type* ResourceContainer<Type>::load(const char* fname)
{
	char* name = Util::cutPath(fname);
	string path = Util::getPath(fname);
	Iterator it = findByNameAndPath(name, path.c_str());

	// if already loaded then inc the users and return the pointer
	if(it != BaseClass::end())
	{
		++ (*it)->usersNum;
		return (*it);
	}

	// else create new, loaded and update the container
	Type* newInstance = new Type();
	newInstance->name = name;
	newInstance->path = path;
	newInstance->usersNum = 1;

	if(!newInstance->load(fname))
	{
		ERROR("Cannot load \"" << fname << '\"');
		delete newInstance;
		return NULL;
	}
	BaseClass::push_back(newInstance);

	return newInstance;
}


//======================================================================================================================
// unload                                                                                                              =
//======================================================================================================================
template<typename Type>
void ResourceContainer<Type>::unload(Type* x)
{
	Iterator it = findByPtr(x);
	if(it == BaseClass::end())
	{
		ERROR("Cannot find resource with pointer 0x" << x);
		return;
	}

	Type* del_ = (*it);
	DEBUG_ERR(del_->usersNum < 1); // WTF?

	--del_->usersNum;

	// if no other users then call unload and update the container
	if(del_->usersNum == 0)
	{
		del_->unload();
		delete del_;
		BaseClass::erase(it);
	}
}
