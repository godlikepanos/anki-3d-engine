#ifndef ANKI_SCENE_MODEL_NODE_H
#define ANKI_SCENE_MODEL_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Movable.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/resource/Resource.h"
#include "anki/resource/Model.h"
#include "anki/collision/Obb.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// A model instance
class ModelPatchNodeInstance: public SceneNode, public Movable, 
	public SpatialComponent
{
	friend class ModelPatchNode;
	friend class ModelNode;

public:
	ModelPatchNodeInstance(
		const char* name, SceneGraph* scene, SceneNode* parent, // Scene
		U32 movableFlags, // Movable
		const ModelPatchBase* modelPatchResource); // Self

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	/// - If it's the last instance update the parent's CS.
	void movableUpdate();
	/// @}

private:
	Obb obb; ///< In world space
	const ModelPatchBase* modelPatch; ///< Keep the resource for tha OBB
};

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode, public Movable, public Renderable,
	public SpatialComponent
{
	friend class ModelPatchNodeInstance;
	friend class ModelNode;

public:
	/// @name Constructors/Destructor
	/// @{
	ModelPatchNode(
		const char* name, SceneGraph* scene, SceneNode* parent, // Scene
		U32 movableFlags, // Movable
		const ModelPatchBase* modelPatch, U instances); // Self

	~ModelPatchNode();
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate();
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase()
	{
		return *modelPatch;
	}

	/// Implements  Renderable::getMaterial
	const Material& getRenderableMaterial()
	{
		return modelPatch->getMaterial();
	}

	/// Overrides Renderable::getRenderableWorldTransforms
	const Transform* getRenderableWorldTransforms();

	/// Overrides Renderable::getRenderableInstancesCount
	U32 getRenderableInstancesCount()
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

	/// This is called by the last of the instances on it's movableUpdate()
	void updateSpatialCs();
};

/// The model scene node
class ModelNode: public SceneNode, public Movable
{
public:
	typedef SceneVector<ModelPatchNode*> ModelPatchNodes;

	/// @name Constructors/Destructor
	/// @{
	ModelNode(
		const char* name, SceneGraph* scene, SceneNode* node, // SceneNode
		U32 movableFlags, // Movable
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

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
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
