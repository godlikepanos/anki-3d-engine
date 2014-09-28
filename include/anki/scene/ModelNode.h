// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
class ObbSpatialComponent;

/// @addtogroup scene
/// @{

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode, 
	public RenderComponent, public SpatialComponent
{
	friend class ModelNode;

public:
	ModelPatchNode(
		const CString& name, SceneGraph* scene, // Scene
		const ModelPatchBase* modelPatch); // Self

	~ModelPatchNode();

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::buildRendering
	void buildRendering(RenderingBuildData& data);

	/// Implements  RenderComponent::getMaterial
	const Material& getMaterial()
	{
		return m_modelPatch->getMaterial();
	}

	/// Overrides RenderComponent::getRenderComponentWorldTransform
	void getRenderWorldTransform(U index, Transform& trf) override;

	Bool getHasWorldTransforms() override
	{
		return true;
	}
	/// @}

	/// Implement SpatialComponent::getSpatialCollisionShape
	const CollisionShape& getSpatialCollisionShape()
	{
		return m_obb;
	}

	/// Implement SpatialComponent::getSpatialOrigin
	Vec4 getSpatialOrigin()
	{
		return m_obb.getCenter();
	}

private:
	Obb m_obb; ///< In world space
	const ModelPatchBase* m_modelPatch; ///< The resource
	SceneVector<ObbSpatialComponent*> m_spatials;

	void updateInstanceSpatials(
		const SceneFrameVector<MoveComponent*>& instanceMoves);
};

/// The model scene node
class ModelNode: public SceneNode, public MoveComponent
{
	friend class ModelPatchNode;

public:
	ModelNode(
		const CString& name, SceneGraph* scene, // SceneNode
		const CString& modelFname); // Self

	virtual ~ModelNode();

	const Model& getModel() const
	{
		return *m_model;
	}

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32, F32) override;

	/// Override MoveComponent::onMoveComponentUpdate
	void onMoveComponentUpdate(
		SceneNode& node, F32 prevTime, F32 crntTime) override;

private:
	ModelResourcePointer m_model; ///< The resource
	SceneVector<ModelPatchNode*> m_modelPatches;
	SceneVector<Transform> m_transforms; ///< Cache the transforms of instances
	Timestamp m_transformsTimestamp;
};

/// @}

} // end namespace anki

#endif
