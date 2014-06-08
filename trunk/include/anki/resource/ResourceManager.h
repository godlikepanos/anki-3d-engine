// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_RESOURCE_MANAGER_H

#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Singleton.h"
#include "anki/util/Functions.h"
#include "anki/util/String.h"

namespace anki {

// Forward
class ConfigSet;

/// @addtogroup resource
/// @{

/// Holds information about a resource
template<typename Type>
struct ResourceHook
{
	String uuid; ///< Unique identifier
	U32 referenceCounter = 0;
	Type* resource = nullptr;

	~ResourceHook()
	{}

	Bool operator==(const ResourceHook& b) const
	{
		return uuid == b.uuid 
			&& referenceCounter == b.referenceCounter 
			&& resource == b.resource;
	}
};

/// Resource manager. It holds a few global variables
class ResourceManager
{
public:
	ResourceManager(const ConfigSet& config);

	const String& getDataPath() const
	{
		return m_dataPath;
	}

	U32 getMaxTextureSize() const
	{
		return m_maxTextureSize;
	}

	U32 getTextureAnisotropy() const
	{
		return m_textureAnisotropy;
	}

	String fixResourcePath(const char* filename) const;

private:
	String m_dataPath;
	U32 m_maxTextureSize;
	U32 m_textureAnisotropy;
};

/// The singleton of resource manager
typedef SingletonInit<ResourceManager> ResourceManagerSingleton;

/// Convenience macro to sanitize resources path
#define ANKI_R(x_) \
	ResourceManagerSingleton::get().fixResourcePath(x_).c_str()

/// Manage resources of a certain type
template<typename Type>
class TypeResourceManager
{
public:
	typedef TypeResourceManager<Type> Self;
	typedef ResourceHook<Type> Hook;
	typedef Vector<Hook*> Container;
	typedef typename Container::iterator Iterator;
	typedef typename Container::const_iterator ConstIterator;

	virtual ~TypeResourceManager()
	{
		for(auto it : hooks)
		{
			propperDelete(it);
		}
	}

	Hook& load(const char* filename);

	void unload(const Hook& hook);

protected:
	Container hooks;

	Iterator find(const char* filename);

	/// Allocate and load a resource.
	/// This method allocates memory for a resource and loads it (calls the
	/// load method). Its been used by the load method. Its a separate
	/// method because we want to specialize it for async loaded resources
	virtual void allocAndLoadRsrc(const char* filename, Type*& ptr);

	/// Dealocate the resource. Its separate for two reasons:
	/// - Because we want to specialize it for the async loaded resources
	/// - Because we cannot have the operator delete in a template body.
	///   Apparently the compiler is to dump to decide
	virtual void deallocRsrc(Type* rsrc);
};

/// @}

} // end namespace anki

#include "anki/resource/ResourceManager.inl.h"

#endif
