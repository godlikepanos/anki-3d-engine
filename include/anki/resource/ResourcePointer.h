// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_POINTER_H
#define ANKI_RESOURCE_RESOURCE_POINTER_H

#include "anki/util/Assert.h"

namespace anki {

/// Special smart pointer that points to resource classes.
///
/// It looks like auto_ptr but the main difference is that when its out of scope
/// it tries to unload the resource.
template<typename Type, typename TResourceManagerSingleton>
class ResourcePointer
{
public:
	typedef Type Value; ///< Resource type
	typedef ResourcePointer<Value, TResourceManagerSingleton> Self;
	typedef typename TResourceManagerSingleton::Value::Hook Hook;

	/// Default constructor
	ResourcePointer()
	{}

	/// Copy constructor
	ResourcePointer(const Self& b)
	{
		copy(b);
	}

	/// Move contructor
	ResourcePointer(Self&& b)
	{
		*this = std::move(b);
	}

	/// Construct and load
	ResourcePointer(const char* filename)
	{
		load(filename);
	}

	~ResourcePointer()
	{
		unload();
	}

	/// @name Accessors
	/// @{
	const Value& operator*() const
	{
		ANKI_ASSERT(hook != nullptr);
		return *hook->resource;
	}
	Value& operator*()
	{
		ANKI_ASSERT(hook != nullptr);
		return *hook->resource;
	}

	const Value* operator->() const
	{
		ANKI_ASSERT(hook != nullptr);
		return hook->resource;
	}
	Value* operator->()
	{
		ANKI_ASSERT(hook != nullptr);
		return hook->resource;
	}

	const Value* get() const
	{
		ANKI_ASSERT(hook != nullptr);
		return hook->resource;
	}
	Value* get()
	{
		ANKI_ASSERT(hook != nullptr);
		return hook->resource;
	}

	const std::string& getResourceName() const
	{
		ANKI_ASSERT(hook != nullptr);
		return hook->uuid;
	}
	/// @}

	/// Copy
	Self& operator=(const Self& b)
	{
		copy(b);
		return *this;
	}

	/// Move
	Self& operator=(Self&& b)
	{
		hook = b.hook;
		b.hook = nullptr;
		return *this;
	}

	/// Load the resource using the resource manager
	void load(const char* filename)
	{
		ANKI_ASSERT(hook == nullptr);
		ANKI_ASSERT(filename != nullptr);
		hook = &TResourceManagerSingleton::get().load(filename);
	}

	Bool isLoaded() const
	{
		return hook != nullptr;
	}

private:
	/// Points to an element located in a container in the resource manager
	Hook* hook = nullptr;

	/// Unloads the resource @see loadRsrc
	void unload()
	{
		if(hook != nullptr)
		{
			TResourceManagerSingleton::get().unload(*hook);
			hook = nullptr;
		}
	}

	/// If this empty and @a b empty then unload. If @a b has something then
	/// unload this and load exactly what @b has. In everything else do nothing
	void copy(const Self& b)
	{
		if(b.hook == nullptr)
		{
			if(hook != nullptr)
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

} // end namespace anki

#endif
