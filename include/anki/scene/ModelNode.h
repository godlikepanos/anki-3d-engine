#ifndef ANKI_SCENE_MODEL_NODE_H
#define ANKI_SCENE_MODEL_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/resource/Resource.h"
#include "anki/resource/Model.h"
#include "anki/collision/Obb.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode, 
	public RenderComponent, public SpatialComponent
{
public:
	/// @name Constructors/Destructor
	/// @{
	ModelPatchNode(
		const char* name, SceneGraph* scene, // Scene
		const ModelPatchBase* modelPatch); // Self

	~ModelPatchNode();
	/// @}

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::getRenderingData
	void getRenderingData(
		const PassLodKey& key, 
		const U32* subMeshIndicesArray, U subMeshIndicesCount,
		const Vao*& vao, const ShaderProgram*& prog,
		Drawcall& dracall);

	/// Implements  RenderComponent::getMaterial
	const Material& getMaterial()
	{
		return modelPatch->getMaterial();
	}

	/// Overrides RenderComponent::getRenderComponentWorldTransforms
	const Transform* getRenderWorldTransforms() override;
	/// @}

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32, F32, SceneNode::UpdateType uptype) override;

	/// Implement SpatialComponent::getSpatialCollisionShape
	const CollisionShape& getSpatialCollisionShape()
	{
		return obb;
	}

	/// Implement SpatialComponent::getSpatialOrigin
	Vec3 getSpatialOrigin()
	{
		return obb.getCenter();
	}

private:
	Obb obb; ///< In world space
	const ModelPatchBase* modelPatch; ///< The resource
};

/// The model scene node
class ModelNode: public SceneNode, public MoveComponent
{
public:
	/// @name Constructors/Destructor
	/// @{
	ModelNode(
		const char* name, SceneGraph* scene, // SceneNode
		const char* modelFname); // Self

	virtual ~ModelNode();
	/// @}

	/// @name Accessors
	/// @{
	const Model& getModel() const
	{
		return *model;
	}
	/// @}

private:
	ModelResourcePointer model; ///< The resource
	SceneVector<Transform> transforms;
};

/// @}

} // end namespace anki

#endif
