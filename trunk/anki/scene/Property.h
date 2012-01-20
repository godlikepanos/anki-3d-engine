#ifndef ANKI_SCENE_PROPERTY_H
#define ANKI_SCENE_PROPERTY_H

#include "anki/util/Observer.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>


namespace anki {


// Forward
template<typename T>
class Property;


/// Base class for property
class PropertyBase
{
public:
	/// Flags for property
	enum PropertyFlag
	{
		PF_NONE = 0,
		PF_READ = 1,
		PF_WRITE = 2,
		PF_READ_WRITE = PF_READ | PF_WRITE
	};

	/// @name Constructors/Destructor
	/// @{
	PropertyBase(const char* name_, uint tid_, uint flags_ = PF_NONE)
		: name(name_), tid(tid_), flags(flags_)
	{
		ANKI_ASSERT(tid != 0);
	}

	virtual ~PropertyBase()
	{}
	/// @}

	/// @name Accessors
	/// @{
	const std::string& getName() const
	{
		return name;
	}

	uint getTypeId() const
	{
		return tid;
	}

	template<typename T>
	const T& getValue() const
	{
		checkType<T>();
		return static_cast<const Property<T>*>(this)->getValue();
	}

	template<typename T>
	void setValue(const T& x)
	{
		checkType<T>();
		return static_cast<Property<T>*>(this)->setValue(x);
	}

	bool isFlagEnabled(PropertyFlag flag) const
	{
		return flags & flag;
	}
	/// @}

protected:
	void enableFlag(PropertyFlag flag, bool enable = true)
	{
		flags = enable ? flags | flag : flags & ~flag;
	}
	void disableFlag(PropertyFlag flag)
	{
		enableFlag(flag, false);
	}

private:
	std::string name;
	uint tid;
	uint flags;

	/// Runtime checking of type
	template<typename T>
	void checkType() const
	{
		if(Property<T>::TYPE_ID != getTypeId())
		{
			throw ANKI_EXCEPTION("Types do not match: " + name);
		}
	}
};


/// Property. It holds a pointer to a value
template<typename T>
class Property: public PropertyBase
{
public:
	typedef T Value;
	typedef Property<Value> Self;

	static const uint TYPE_ID; ///< Unique id for every type of property

	/// @name Constructors/Destructor
	/// @{

	/// Read only property
	Property(const char* name, const Value* x, uint flags = PF_NONE)
		: PropertyBase(name, TYPE_ID, flags), ptr(x)
	{
		disableFlag(PF_WRITE);
	}

	/// Read/write property
	Property(const char* name, Value* x, uint flags = PF_NONE)
		: PropertyBase(name, TYPE_ID, flags), ptr(x)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Value& getValue() const
	{
		if(!isFlagEnabled(PF_READ))
		{
			throw ANKI_EXCEPTION("Property is not readable: " + name);
		}

		return *ptr;
	}

	/// Set the value and emit the signal valueChanged
	void setValue(const Value& x)
	{
		if(!isFlagEnabled(PF_WRITE))
		{
			throw ANKI_EXCEPTION("Property is not writable: " + name);
		}

		*(const_cast<Value*>(ptr)) = x;
		ANKI_EMIT valueChanged(x);
	}
	/// @}

	ANKI_SIGNAL(const Value&, valueChanged)

private:
	const Value* ptr; ///< Have only one const pointer for size saving
};


template<typename T>
const uint Property<T>::TYPE_ID = 0;


/// A set of properties
class PropertyMap
{
public:
	typedef boost::ptr_vector<PropertyBase> Container;
	typedef boost::unordered_map<std::string, PropertyBase*> NameToPropertyMap;


	/// Create a new property
	template<typename T>
	Property<T>& addProperty(const char* name, T* x,
		uint flags = PropertyBase::PF_NONE)
	{
		if(propertyExists(name))
		{
			throw ANKI_EXCEPTION("Property already exists: " + name);
		}

		Property<T>* newp = new Property<T>(name, x, flags);

		props.push_back(newp);
		map[name] = newp;

		return *newp;
	}


	/// XXX
	const PropertyBase& findPropertyBaseByName(const char* name) const
	{
		NameToPropertyMap::const_iterator it = map.find(name);
		if(it == map.end())
		{
			throw ANKI_EXCEPTION("Property not found: " + name);
		}
		return *(it->second);
	}


	/// XXX
	PropertyBase& findPropertyBaseByName(const char* name)
	{
		NameToPropertyMap::iterator it = map.find(name);
		if(it == map.end())
		{
			throw ANKI_EXCEPTION("Property not found: " + name);
		}
		return *(it->second);
	}


	/// Alias for findPropertyBaseByName
	const PropertyBase& operator[](const char* name) const
	{
		return findPropertyBaseByName(name);
	}


	/// Alias for findPropertyBaseByName
	PropertyBase& operator[](const char* name)
	{
		return findPropertyBaseByName(name);
	}


	/// Return true if the property named @a name exists
	bool propertyExists(const char* name) const
	{
		return map.find(name) != map.end();
	}


	/// XXX
	template<typename T>
	void setValue(const char* name, const T& v)
	{
		findPropertyBaseByName(name).setValue<T>(v);
	}

private:
	Container props;
	NameToPropertyMap map;
};


} // namespace anki


#endif
