#ifndef ANKI_SCENE_MODEL_NODE_H
#define ANKI_SCENE_MODEL_NODE_H

#include "anki/scene/SceneNode.h"
#include "anki/resource/Resource.h"
#include "anki/scene/Renderable.h"
#include "anki/resource/Model.h"
#include "anki/collision/Obb.h"
#include "anki/scene/MaterialRuntime.h"
#include <boost/array.hpp>
#include <vector>
#include <boost/range/iterator_range.hpp>
#include <boost/scoped_ptr.hpp>


namespace anki {


/// A fragment of the ModelNode
class ModelPatchNode: public Renderable, public SceneNode
{
public:
	ModelPatchNode(const ModelPatch* modelPatch_, SceneNode* parent)
		: SceneNode(SNT_RENDERABLE_NODE, parent->getScene(),
			SNF_INHERIT_PARENT_TRANSFORM, parent), modelPatch(modelPatch_)
	{
		mtlr.reset(new MaterialRuntime(modelPatch->getMaterial()));
	}

	/// Implements SceneNode::getVisibilityCollisionShapeWorldSpace
	const CollisionShape*
		getVisibilityCollisionShapeWorldSpace() const
	{
		return &visibilityShapeWSpace;
	}

	void init(const char*)
	{}

	/// Re-implements SceneNode::moveUpdate
	void moveUpdate()
	{
		visibilityShapeWSpace =
			modelPatch->getMesh().getVisibilityShape().getTransformed(
			getParent()->getWorldTransform());
	}

	/// Implements Renderable::getVao
	const Vao& getVao(const PassLevelKey& k)
	{
		return modelPatch->getVao(k);
	}

	/// Implements Renderable::getVertexIdsNum
	uint getVertexIdsNum(const PassLevelKey& k)
	{
		return modelPatch->getMesh().getVertsNum();
	}

	/// Implements Renderable::getMaterialRuntime
	MaterialRuntime& getMaterialRuntime()
	{
		return *mtlr;
	}

	/// Implements Renderable::getWorldTransform
	const Transform& getWorldTransform(const PassLevelKey&)
	{
		return SceneNode::getWorldTransform();
	}

	/// Implements Renderable::getPreviousWorldTransform
	const Transform& getPreviousWorldTransform(
		const PassLevelKey& k)
	{
		return SceneNode::getPrevWorldTransform();
	}

private:
	Obb visibilityShapeWSpace;
	const ModelPatch* modelPatch;
	boost::scoped_ptr<MaterialRuntime> mtlr; ///< Material runtime
};


/// The model scene node
class ModelNode: public SceneNode
{
public:
	typedef boost::iterator_range<std::vector<ModelPatchNode*>::
		const_iterator> ConstRangeModelPatchNodes;

	typedef boost::iterator_range<std::vector<ModelPatchNode*>::
		iterator> MutableRangeModelPatchNodes;

	ModelNode(Scene& scene, ulong flags, SceneNode* parent);
	virtual ~ModelNode();

	/// @name Accessors
	/// @{
	ConstRangeModelPatchNodes getModelPatchNodes() const
	{
		return ConstRangeModelPatchNodes(patches.begin(), patches.end());
	}
	MutableRangeModelPatchNodes getModelPatchNodes()
	{
		return MutableRangeModelPatchNodes(patches.begin(), patches.end());
	}

	const Model& getModel() const
	{
		return *model;
	}

	const CollisionShape*
		getVisibilityCollisionShapeWorldSpace() const
	{
		return &visibilityShapeWSpace;
	}
	/// @}

	/// Initialize the node
	/// - Load the resource
	void init(const char* filename);

	/// Update the bounding shape
	void moveUpdate();

private:
	ModelResourcePointer model;
	std::vector<ModelPatchNode*> patches;
	Obb visibilityShapeWSpace;
};


} // end namespace


#endif
