#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"
#include "anki/util/Vector.h"
#include "anki/gl/Ubo.h"

namespace anki {

class ModelPatchBase;
class Material;
class MaterialVariable;
class Transform;

/// @addtogroup Scene
/// @{

/// Material variable property. Its a layer on top of material variables
template<typename T>
class MaterialVariableProperty: public ReadCowPointerProperty<T>
{
public:
	typedef T Value;
	typedef ReadCowPointerProperty<T> Base;

	/// @name Constructors/Destructor
	/// @{
	MaterialVariableProperty(const char* name, const Value* x,
		const MaterialVariable* mvar_)
		: Base(name, x), mvar(mvar_)
	{}
	/// @}

	/// @name Accessors
	/// @{
	U32 getBuildinId() const
	{
		return buildinId;
	}
	void setBuildinId(U32 id)
	{
		buildinId = id;
	}

	const MaterialVariable& getMaterialVariable() const
	{
		return *mvar;
	}
	/// @}

private:
	U32 buildinId = 0; ///< The renderer sets it
	const MaterialVariable* mvar = nullptr;
};

/// Renderable interface. Implemented by renderable scene nodes
class Renderable
{
public:
	typedef Vector<PropertyBase*> MaterialVariableProperties;

	Renderable()
	{}

	virtual ~Renderable();

	/// Access to VAOs
	virtual const ModelPatchBase& getModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getMaterial() const = 0;

	/// Information for movables
	virtual const Transform* getRenderableWorldTransform() const
	{
		return nullptr;
	}

	/// @name Accessors
	/// @{
	MaterialVariableProperties::iterator getPropertiesBegin()
	{
		return props.begin();
	}
	MaterialVariableProperties::iterator getPropertiesEnd()
	{
		return props.end();
	}

	Ubo& getUbo()
	{
		return ubo;
	}
	/// @}

protected:
	void init(PropertyMap& pmap);

private:
	MaterialVariableProperties props;
	Ubo ubo;
};
/// @}

} // end namespace anki

#endif
