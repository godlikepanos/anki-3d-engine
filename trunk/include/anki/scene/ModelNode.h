#ifndef ANKI_SCENE_MODEL_NODE_H
#define ANKI_SCENE_MODEL_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/resource/Resource.h"
#include "anki/resource/Model.h"
#include "anki/collision/Obb.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/scoped_ptr.hpp>

namespace anki {

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode, public Movable, public Renderable,
	public Spatial
{
public:
	/// @name Constructors/Destructor
	/// @{
	ModelPatchNode(const ModelPatch* modelPatch_,
		const char* name, Scene* scene, // Scene
		uint movableFlags, Movable* movParent); // Movable
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
	/// @}

	// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
		obb = modelPatch->getMeshBase().getBoundingShape().getTransformed(
			getWorldTransform());
	}
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getModelPatchBase() const
	{
		return *modelPatch;
	}

	/// Implements  Renderable::getMaterial
	const Material& getMaterial() const
	{
		return modelPatch->getMaterial();
	}

	/// Overrides Renderable::getRenderableWorldTransform
	const Transform* getRenderableWorldTransform() const
	{
		return &getWorldTransform();
	}
	/// @}

private:
	Obb obb; ///< In world space
	const ModelPatch* modelPatch; ///< The resource
};

/// The model scene node
class ModelNode: public SceneNode, public Movable, public Spatial
{
public:
	typedef boost::ptr_vector<ModelPatchNode> ModelPatchNodes;

	typedef boost::iterator_range<ModelPatchNodes::const_iterator>
		ConstRangeModelPatchNodes;

	typedef boost::iterator_range<ModelPatchNodes::iterator>
		MutableRangeModelPatchNodes;

	/// @name Constructors/Destructor
	/// @{
	ModelNode(const char* modelFname,
		const char* name, Scene* scene, // SceneNode
		uint movableFlags, Movable* movParent); // Movable

	virtual ~ModelNode();
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
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update collision shape
	void movableUpdate()
	{
		obb = model->getVisibilityShape().getTransformed(
			getWorldTransform());
	}
	/// @}

private:
	ModelResourcePointer model; ///< The resource
	ModelPatchNodes patches;
	Obb obb;
};

} // end namespace

#endif
