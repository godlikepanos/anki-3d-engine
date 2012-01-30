#ifndef ANKI_SCENE_PROPERTY_H
#define ANKI_SCENE_PROPERTY_H

#include "anki/util/Observer.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>


namespace anki {


// Forward
template<typename T>
class Property;


/// Base class for property
class PropertyBase: public boost::noncopyable
{
public:
	/// @name Constructors/Destructor
	/// @{
	PropertyBase(const char* name_, uint tid_)
		: name(name_), tid(tid_)
	{
		ANKI_ASSERT(tid != 0 && "Property::TYPE_ID not set");
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

private:
	std::string name;
	uint tid;

	/// Runtime type checking
	template<typename T>
	void checkType() const
	{
		if(Property<T>::TYPE_ID != getTypeId())
		{
			throw ANKI_EXCEPTION("Types do not match: " + name);
		}
	}
};


/// Property interface
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
	Property(const char* name)
		: PropertyBase(name, TYPE_ID)
	{}
	/// @}

	/// @name Accessors
	/// @{
	virtual const Value& getValue() const
	{
		throw ANKI_EXCEPTION("Property is not readable: " + getName());
	}

	/// Set the value and emit the signal valueChanged
	virtual void setValue(const Value& x)
	{
		throw ANKI_EXCEPTION("Property is not writable: " + getName());
	}
	/// @}

	ANKI_SIGNAL(const Value&, valueChanged)
};


template<typename T>
const uint Property<T>::TYPE_ID = 0;


/// Read only property
template<typename T>
class ReadProperty: public Property<T>
{
public:
	typedef T Value;
	typedef Property<T> Base;

	/// @name Constructors/Destructor
	/// @{
	ReadProperty(const char* name, const Value& x_)
		: Base(name), x(x_)
	{}
	/// @}

	/// @name Accessors
	/// @{

	/// Overrides Property::getValue()
	const Value& getValue() const
	{
		return x;
	}
	/// @}

private:
	Value x;
};


/// Read write property
template<typename T>
class ReadWriteProperty: public Property<T>
{
public:
	typedef T Value;
	typedef Property<T> Base;

	/// @name Constructors/Destructor
	/// @{
	ReadWriteProperty(const char* name, const Value& x_)
		: Base(name), x(x_)
	{}
	/// @}

	/// @name Accessors
	/// @{

	/// Overrides Property::getValue()
	const Value& getValue() const
	{
		return x;
	}

	/// Overrides Property::setValue()
	virtual void setValue(const Value& x_)
	{
		x = x_;
		ANKI_EMIT valueChanged(x);
	}
	/// @}

private:
	Value x;
};


/// Read only property that holds a reference to a value
template<typename T>
class ReadPointerProperty: public Property<T>
{
public:
	typedef T Value;
	typedef Property<T> Base;

	/// @name Constructors/Destructor
	/// @{
	ReadPointerProperty(const char* name, const Value* x)
		: Base(name), ptr(x)
	{}
	/// @}

	/// @name Accessors
	/// @{

	/// Overrides Property::getValue()
	const Value& getValue() const
	{
		return *ptr;
	}
	/// @}

private:
	const Value* ptr; ///< Have only one const pointer for size saving
};


/// Read write property that alters a reference to the value
template<typename T>
class ReadWritePointerProperty: public Property<T>
{
public:
	typedef T Value;
	typedef Property<T> Base;

	/// @name Constructors/Destructor
	/// @{
	ReadWritePointerProperty(const char* name, Value* x)
		: Base(name), ptr(x)
	{}
	/// @}

	/// @name Accessors
	/// @{

	/// Overrides Property::getValue()
	const Value& getValue() const
	{
		return *ptr;
	}

	/// Overrides Property::setValue()
	virtual void setValue(const Value& x)
	{
		*ptr = x;
		ANKI_EMIT valueChanged(x);
	}
	/// @}

private:
	Value* ptr; ///< Have only one const pointer for size saving
};


/// Read and Copy on Write property
template<typename T>
class ReadCowPointerProperty: public Property<T>
{
public:
	typedef T Value;
	typedef Property<T> Base;

	/// @name Constructors/Destructor
	/// @{
	ReadCowPointerProperty(const char* name, const Value* x)
		: Base(name), ptr(x)
	{}
	/// @}

	/// @name Accessors
	/// @{
	/// Overrides Property::getValue()
	const Value& getValue() const
	{
		return (sptr.get()) ? *sptr : *ptr;
	}

	/// Overrides Property::setValue()
	virtual void setValue(const Value& x)
	{
		if(sptr.get())
		{
			*sptr = x;
		}
		else
		{
			sptr.reset(new Value(x));
		}
		ANKI_EMIT valueChanged(x);
	}
	/// @}

private:
	const Value* ptr;
	boost::scoped_ptr sptr;
};


/// A set of properties
class PropertyMap
{
public:
	typedef boost::ptr_vector<PropertyBase> Container;
	typedef ConstCharPtrHashMap<PropertyBase*>::Type NameToPropertyMap;

	/// Create a new property
	template<typename T>
	Property<T>& addNewProperty(Property<T>* newp)
	{
		if(propertyExists(newp->getName().c_str()))
		{
			throw ANKI_EXCEPTION("Property already exists: " + newp->getName());
		}

		props.push_back(newp);
		map[newp->getName().c_str()] = newp;

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
