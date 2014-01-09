#ifndef ANKI_SCENE_PROPERTY_H
#define ANKI_SCENE_PROPERTY_H

#if 0

#include "anki/util/Observer.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Visitor.h"
#include "anki/Math.h"
#include "anki/resource/Resource.h"
#include "anki/resource/TextureResource.h"
#include "anki/Collision.h"
#include "anki/util/Dictionary.h"

namespace anki {

// Forward
class PropertyBase;

template<typename T>
class Property;

typedef VisitableCommonBase<
	PropertyBase, // Base
	// Misc
	Property<std::string>, Property<bool>, Property<float>,
	Property<TextureResourcePointer>,
	// Math
	Property<Vec2>, Property<Vec3>, Property<Vec4>, Property<Mat3>,
	Property<Mat4>, Property<Transform>,
	// Collision
	Property<OrthographicFrustum>, Property<PerspectiveFrustum>>
	PropertyVisitable;

/// Base class for property
class PropertyBase: public NonCopyable, public PropertyVisitable
{
public:
	/// @name Constructors/Destructor
	/// @{
	PropertyBase(const char* name_)
		: name(name_)
	{}

	virtual ~PropertyBase()
	{}
	/// @}

	/// @name Accessors
	/// @{
	const std::string& getName() const
	{
		return name;
	}

	/// Get the property value. Throws if the @a T is incorrect
	template<typename TValue>
	const TValue& getValue() const
	{
		checkType<TValue>();
		return static_cast<const Property<TValue>*>(this)->getValue();
	}

	/// Set the property value. Throws if the @a T is incorrect
	template<typename TValue>
	void setValue(const TValue& x)
	{
		checkType<TValue>();
		return static_cast<Property<TValue>*>(this)->setValue(x);
	}
	/// @}

	/// Upcast to property. It makes a runtime check
	template<typename TProp>
	TProp& upCast()
	{
		checkType<TProp::Value>();
		return static_cast<TProp&>(*this);
	}

private:
	std::string name;

	/// Runtime type checking
	template<typename TValue>
	void checkType() const
	{
		if(PropertyVisitable::getVariadicTypeId<Property<TValue>>() !=
			getVisitableTypeId())
		{
			throw ANKI_EXCEPTION("Types do not match: %s", name.c_str());
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

	/// @name Constructors/Destructor
	/// @{

	/// Read only property
	Property(const char* name)
		: PropertyBase(name)
	{
		setupVisitable(this);
	}
	/// @}

	/// @name Accessors
	/// @{
	virtual const Value& getValue() const
	{
		throw ANKI_EXCEPTION("Property is not readable: %s", getName().c_str());
	}

	/// Set the value and emit the signal valueChanged
	virtual void setValue(const Value& x)
	{
		(void)x;
		throw ANKI_EXCEPTION("Property is not writable: %s", getName().c_str());
	}
	/// @}

	/// Signal that it is emitted when the value gets changed
	ANKI_SIGNAL(const Value&, valueChanged)
};

/// Read only property
template<typename T>
class ReadProperty: public Property<T>
{
public:
	typedef T Value;
	typedef Property<T> Base;

	/// @name Constructors/Destructor
	/// @{
	ReadProperty(const char* name, const Value& initialValue)
		: Base(name), x(initialValue)
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
	ReadWriteProperty(const char* name, const Value& initialValue)
		: Base(name), x(initialValue)
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
	ReadPointerProperty(const char* name, const Value* ptr_)
		: Base(name), ptr(ptr_)
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
	const Value* ptr;
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
	Value* ptr;
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
		if(sptr.get())
		{
			return *sptr;
		}
		else
		{
			return *ptr;
		}
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
	std::unique_ptr<Value> sptr;
};

/// A set of properties
class PropertyMap
{
public:
	typedef Vector<PropertyBase*> Container;
	typedef Dictionary<PropertyBase*> NameToPropertyMap;

	/// Add new property to the map. The map gets the ownership of this
	/// property
	template<typename T>
	Property<T>& addNewProperty(Property<T>* newp)
	{
		if(propertyExists(newp->getName().c_str()))
		{
			throw ANKI_EXCEPTION("Property already exists: %s", 
				newp->getName().c_str());
		}

		props.push_back(newp);
		map[newp->getName().c_str()] = newp;

		return *newp;
	}

	/// Self explanatory
	const PropertyBase& findPropertyBaseByName(const char* name) const
	{
		NameToPropertyMap::const_iterator it = map.find(name);
		if(it == map.end())
		{
			throw ANKI_EXCEPTION("Property not found: %s", name);
		}
		return *(it->second);
	}

	/// Self explanatory
	PropertyBase& findPropertyBaseByName(const char* name)
	{
		NameToPropertyMap::iterator it = map.find(name);
		if(it == map.end())
		{
			throw ANKI_EXCEPTION("Property not found: %s", name);
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

	/// Set the value of a property. It may throw
	template<typename T>
	void setValue(const char* name, const T& v)
	{
		findPropertyBaseByName(name).setValue<T>(v);
	}

private:
	Container props;
	NameToPropertyMap map;
};

} // end namespace anki

#endif

#endif
