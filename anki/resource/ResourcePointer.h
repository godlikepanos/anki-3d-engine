#ifndef ANKI_RESOURCE_RESOURCE_POINTER_H
#define ANKI_RESOURCE_RESOURCE_POINTER_H

#include "anki/util/Assert.h"


namespace anki {


/// This is a special smart pointer that points to Resource derivatives. It
/// looks like auto_ptr but the main difference is that when its out of scope
/// it tries to unload the resource.
template<typename Type, typename ResourceManagerSingleton>
class ResourcePointer
{
public:
	typedef ResourcePointer<Type, ResourceManagerSingleton> Self;
	typedef typename ResourceManagerSingleton::ValueType::Hook Hook;

	/// Default constructor
	ResourcePointer()
		: hook(NULL)
	{}

	/// Copy constructor
	ResourcePointer(const Self& b)
		: hook(NULL)
	{
		copy(b);
	}

	/// Construct and load
	ResourcePointer(const char* filename)
		: hook(NULL)
	{
		load(filename);
	}

	~ResourcePointer()
	{
		unload();
	}

	/// @name Accessors
	/// @{
	Type& operator*() const
	{
		ANKI_ASSERT(hook != NULL);
		return *hook->resource;
	}

	Type* operator->() const
	{
		ANKI_ASSERT(hook != NULL);
		return hook->resource;
	}

	Type* get() const
	{
		return hook->resource;
	}

	const std::string& getResourceName() const
	{
		return hook->uuid;
	}
	/// @}

	/// Copy
	Self& operator=(const Self& b)
	{
		copy(b);
		return *this;
	}

	/// Load the resource using the resource manager
	void load(const char* filename)
	{
		ANKI_ASSERT(hook == NULL);
		hook = &ResourceManagerSingleton::get().load(filename);
	}

private:
	/// Points to an element located in a container in the resource manager
	Hook* hook;

	/// Unloads the resource @see loadRsrc
	void unload()
	{
		if(hook != NULL)
		{
			ResourceManagerSingleton::get().unload(*hook);
			hook = NULL;
		}
	}

	/// If this empty and @a b empty then unload. If @a b has something then
	/// unload this and load exactly what @b has. In everything else do nothing
	void copy(const Self& b)
	{
		if(b.hook == NULL)
		{
			if(hook != NULL)
			{
				unload();
			}
		}
		else
		{
			unload();
			load(b.hook->uuid.c_str());
		}
	}
};


} // end namespace


#endif
