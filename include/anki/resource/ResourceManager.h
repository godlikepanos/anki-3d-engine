#ifndef ANKI_RESOURCE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_RESOURCE_MANAGER_H

#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Singleton.h"
#include "anki/util/Functions.h"
#include <string>

namespace anki {

/// Holds information about a resource
template<typename Type>
struct ResourceHook
{
	std::string uuid; ///< Unique identifier
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
	ResourceManager();

	const std::string& getDataPath() const
	{
		return dataPath;
	}

	std::string fixResourcePath(const char* filename) const;

private:
	std::string dataPath;
};

/// The singleton of resource manager
typedef Singleton<ResourceManager> ResourceManagerSingleton;

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

} // end namespace anki

#include "anki/resource/ResourceManager.inl.h"

#endif
