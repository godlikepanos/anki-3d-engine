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

// Forward
class ModelPatchNode;

/// @addtogroup Scene
/// @{

/// A model instance
class ModelPatchNodeInstance: public SceneNode, public MoveComponent, 
	public SpatialComponent
{
	friend class ModelPatchNode;
	friend class ModelNode;

public:
	ModelPatchNodeInstance(
		const char* name, SceneGraph* scene, // SceneNode
		ModelPatchNode* modelPatchNode); // Self

	/// @name MoveComponent virtuals
	/// @{

	/// Overrides MoveComponent::moveUpdate(). This does:
	/// - Update the collision shape
	/// - If it's the last instance update the parent's CS.
	void moveUpdate();
	/// @}

private:
	Obb obb; ///< In world space
	ModelPatchNode* modelPatchNode; ///< Keep the father here
};

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode, public MoveComponent, 
	public RenderComponent, public SpatialComponent
{
	friend class ModelPatchNodeInstance;
	friend class ModelNode;

public:
	/// @name Constructors/Destructor
	/// @{
	ModelPatchNode(
		const char* name, SceneGraph* scene, // Scene
		const ModelPatchBase* modelPatch, U instances); // Self

	~ModelPatchNode();
	/// @}

	/// @name MoveComponent virtuals
	/// @{

	/// Overrides MoveComponent::moveUpdate(). This does:
	/// - Update the collision shape
	void moveUpdate();
	/// @}

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::getRenderingData
	void getRenderingData(
		const PassLevelKey& key, 
		const Vao*& vao, const ShaderProgram*& prog,
		const U32* subMeshIndicesArray, U subMeshIndicesCount,
		Array<U32, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesCountArray,
		Array<const void*, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesOffsetArray, 
		U32& drawcallCount) const
	{
		modelPatch->getRenderingDataSub(key, vao, prog, 
			subMeshIndicesArray, subMeshIndicesCount, 
			indicesCountArray, indicesOffsetArray, drawcallCount);
	}

	/// Implements  RenderComponent::getMaterial
	const Material& getMaterial()
	{
		return modelPatch->getMaterial();
	}

	/// Overrides RenderComponent::getRenderComponentWorldTransforms
	const Transform* getRenderWorldTransforms();

	/// Overrides RenderComponent::getRenderComponentInstancesCount
	U32 getRenderInstancesCount()
	{
		// return this and the instances 
		return (transforms.size() > 0) ? transforms.size() : 1;
	}
	/// @}

private:
	Obb obb; ///< In world space.
	const ModelPatchBase* modelPatch; ///< The resource
	SceneVector<ModelPatchNodeInstance*> instances;
	SceneVector<Transform> transforms;

	/// This is called by the last of the instances on it's moveUpdate()
	void updateSpatialCs();
};

/// The model scene node
class ModelNode: public SceneNode, public MoveComponent
{
public:
	typedef SceneVector<ModelPatchNode*> ModelPatchNodes;

	/// @name Constructors/Destructor
	/// @{
	ModelNode(
		const char* name, SceneGraph* scene, // SceneNode
		const char* modelFname, U instances = 1); // Self

	virtual ~ModelNode();
	/// @}

	/// @name Accessors
	/// @{
	const Model& getModel() const
	{
		return *model;
	}
	/// @}

	/// Set the local transform of one instance
	void setInstanceLocalTransform(U instanceIndex, const Transform& trf);

private:
	ModelResourcePointer model; ///< The resource
	ModelPatchNodes patches;
};

/// @}

} // end namespace anki

#endif
