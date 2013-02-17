#ifndef ANKI_SCENE_MODEL_NODE_H
#define ANKI_SCENE_MODEL_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/resource/Resource.h"
#include "anki/resource/Model.h"
#include "anki/collision/Obb.h"

namespace anki {

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode, public Movable, public Renderable,
	public Spatial
{
public:
	/// @name Constructors/Destructor
	/// @{
	ModelPatchNode(const ModelPatchBase* modelPatch_,
		const char* name, Scene* scene, // Scene
		U32 movableFlags, Movable* movParent); // Movable
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::getSpatial()
	Spatial* getSpatial()
	{
		return this;
	}

	/// Override SceneNode::getRenderable
	Renderable* getRenderable()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
	}
	/// @}

	// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
		obb = modelPatch->getBoundingShape().getTransformed(
			getWorldTransform());
		spatialMarkForUpdate();
	}
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase() const
	{
		return *modelPatch;
	}

	/// Implements  Renderable::getMaterial
	const Material& getRenderableMaterial() const
	{
		return modelPatch->getMaterial();
	}

	/// Overrides Renderable::getRenderableWorldTransforms
	const Transform* getRenderableWorldTransforms() const
	{
		return &getWorldTransform();
	}
	/// @}

private:
	Obb obb; ///< In world space
	const ModelPatchBase* modelPatch; ///< The resource
};

/// The model scene node
class ModelNode: public SceneNode, public Movable
{
public:
	typedef SceneVector<ModelPatchNode*> ModelPatchNodes;

	/// @name Constructors/Destructor
	/// @{
	ModelNode(const char* modelFname,
		const char* name, Scene* scene, // SceneNode
		uint movableFlags, Movable* movParent); // Movable

	virtual ~ModelNode();
	/// @}

	/// @name Accessors
	/// @{
	const Model& getModel() const
	{
		return *model;
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate
	void frameUpdate(float prevUpdateTime, float crntTime, int frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
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

private:
	ModelResourcePointer model; ///< The resource
	ModelPatchNodes patches;
};

} // end namespace anki

#endif
