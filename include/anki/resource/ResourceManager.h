#ifndef ANKI_RESOURCE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_RESOURCE_MANAGER_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <string>

namespace anki {

/// Holds information about a resource
template<typename Type>
struct ResourceHook
{
	std::string uuid; ///< Unique identifier
	int referenceCounter;
	Type* resource;

	bool operator==(const ResourceHook& b) const
	{
		return uuid == b.uuid 
			&& referenceCounter == b.referenceCounter 
			&& resource == b.resource;
	}
};

/// XXX
template<typename Type>
class ResourceManager
{
public:
	typedef ResourceManager<Type> Self;
	typedef ResourceHook<Type> Hook;
	typedef boost::ptr_vector<Hook> Container;
	typedef typename Container::iterator Iterator;
	typedef typename Container::const_iterator ConstIterator;

	virtual ~ResourceManager()
	{}

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
	/// Apparently the compiler is to dump to decide
	virtual void deallocRsrc(Type* rsrc);
};

} // end namespace

#include "anki/resource/ResourceManager.inl.h"

#endif
