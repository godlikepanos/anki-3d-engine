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

/// The ID of a buildin material variable
enum BuildinMaterialVariableId
{
	BMV_NO_BUILDIN = 0,
	BMV_MODEL_VIEW_PROJECTION_MATRIX,
	BMV_MODEL_VIEW_MATRIX,
	BMV_NORMAL_MATRIX,
	BMV_BLURRING,
	BMV_INSTANCING_TRANSLATIONS,
	BMV_INSTANCING_MODEL_VIEW_PROJECTION_MATRICES,
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
	virtual const ModelPatchBase& getRenderableModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getRenderableMaterial() const = 0;

	/// Information for movables
	virtual const Transform* getRenderableWorldTransform() const
	{
		return nullptr;
	}

	/// @name Instancing methods
	/// @{

	virtual const Vec3* getRenderableInstancingTranslations() const
	{
		return nullptr;
	}

	virtual const Transform* getRenderableInstancingWorldTransforms() const
	{
		return nullptr;
	}

	virtual U32 getRenderableInstancesCount() const
	{
		return 0;
	}
	/// @}

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

	Ubo& getInstancingUbo()
	{
		return instancingUbo;
	}
	/// @}

protected:
	/// The derived class needs to call that
	void init(PropertyMap& pmap);

private:
	RenderableMaterialVariables vars;
	Ubo ubo;
	Ubo instancingUbo;
};
/// @}

} // end namespace anki

#endif
