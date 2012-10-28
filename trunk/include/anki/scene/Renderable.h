#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"
#include "anki/util/Vector.h"
#include "anki/gl/Ubo.h"
#include "anki/resource/Material.h"

namespace anki {

class ModelPatchBase;
class Transform;

/// @addtogroup Scene
/// @{

/// XXX
enum BuildinMaterialVariableId
{
	BMV_NO_BUILDIN = 0,
	BMV_MODEL_VIEW_PROJECTION_MATRIX,
	BMV_MODEL_VIEW_MATRIX,
	BMV_NORMAL_MATRIX,
	BMV_BLURRING,
	BMV_COUNT
};

/// A wrapper on top of MaterialVariable
class RenderableMaterialVariable
{
public:
	RenderableMaterialVariable(const MaterialVariable* mvar_);

	/// @name Accessors
	/// @{
	BuildinMaterialVariableId getBuildinId() const
	{
		return buildinId;
	}

	const MaterialVariable& getMaterialVariable() const
	{
		return *mvar;
	}

	const std::string& getName() const
	{
		return mvar->getName();
	}
	/// @}

private:
	BuildinMaterialVariableId buildinId;
	const MaterialVariable* mvar = nullptr;
	PropertyBase* prop = nullptr;
};

/// Renderable interface. Implemented by renderable scene nodes
class Renderable
{
public:
	typedef PtrVector<RenderableMaterialVariable> RenderableMaterialVariables;

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
	RenderableMaterialVariables::iterator getVariablesBegin()
	{
		return vars.begin();
	}
	RenderableMaterialVariables::iterator getVariablesEnd()
	{
		return vars.end();
	}

	Ubo& getUbo()
	{
		return ubo;
	}
	/// @}

protected:
	void init(PropertyMap& pmap);

private:
	RenderableMaterialVariables vars;
	Ubo ubo;
};
/// @}

} // end namespace anki

#endif
