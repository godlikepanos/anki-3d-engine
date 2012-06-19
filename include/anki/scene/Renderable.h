#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"
#include <vector>

namespace anki {

class ModelPatchBase;
class Material;
class MaterialVariable;
class Light;
class Transform;

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

/// Renderable interface. Implemented by renderable scene nodes
class Renderable
{
public:
	typedef std::vector<PropertyBase*> Properties;

	Renderable()
	{}

	virtual ~Renderable()
	{}

	/// Access to VAOs
	virtual const ModelPatchBase& getModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getMaterial() const = 0;

	/// Information for movables
	virtual const Transform* getRenderableWorldTransform() const
	{
		return nullptr;
	}

	const Properties& getProperties() const
	{
		return props;
	}

protected:
	void init(PropertyMap& pmap);

private:
	Properties props;
};


/// @}


} // end namespace


#endif
