#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"
#include <vector>
#include <boost/range/iterator_range.hpp>


namespace anki {


class ModelPatchBase;
class Material;
class MaterialVariable;


/// @addtogroup Scene
/// @{


/// XXX
template<typename T>
class MaterialVariableProperty: public ReadCowPointerProperty<T>
{
public:
	typedef T Value;
	typedef ReadCowPointerProperty<T> Base;

	/// @name Constructors/Destructor
	/// @{
	MaterialVariableProperty(const char* name, const Value* x, bool buildin_)
		: Base(name, x), buildin(buildin_)
	{}
	/// @}

	/// @name Accessors
	/// @{
	bool isBuildIn() const
	{
		return buildin;
	}
	/// @}

private:
	bool buildin;
};


/// Renderable interface
///
/// Implemented by renderable scene nodes
class Renderable
{
public:
	typedef std::vector<PropertyBase*> Properties;
	typedef boost::iterator_range<Properties::iterator> MutableRange;
	typedef boost::iterator_range<Properties::const_iterator> ConstRange;

	/// Access to VAOs
	virtual const ModelPatchBase& getModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getMaterial() const = 0;

	MutableRange getProperties()
	{
		return MutableRange(props.begin(), props.end());
	}

	ConstRange getProperties() const
	{
		return ConstRange(props.begin(), props.end());
	}

protected:
	void init(PropertyMap& pmap);

private:
	Properties props;
};
/// @}


} // end namespace


#endif
