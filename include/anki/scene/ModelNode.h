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
	ModelPatchNode(SceneGraph* scene);

	~ModelPatchNode();

	ANKI_USE_RESULT Error create(
		const CString& name, const ModelPatchBase* modelPatch);

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::buildRendering
	ANKI_USE_RESULT Error buildRendering(RenderingBuildData& data);

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
	SceneDArray<ObbSpatialComponent*> m_spatials;

	ANKI_USE_RESULT Error updateInstanceSpatials(
		const MoveComponent* instanceMoves[], 
		U32 instanceMovesCount);
};

/// The model scene node
class ModelNode: public SceneNode, public MoveComponent
{
	friend class ModelPatchNode;

public:
	ModelNode(SceneGraph* scene);

	virtual ~ModelNode();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& modelFname);

	const Model& getModel() const
	{
		return *m_model;
	}

	/// Override SceneNode::frameUpdate
	ANKI_USE_RESULT Error frameUpdate(F32, F32) override;

	/// Override MoveComponent::onMoveComponentUpdate
	ANKI_USE_RESULT Error onMoveComponentUpdate(
		SceneNode& node, F32 prevTime, F32 crntTime) override;

private:
	ModelResourcePointer m_model; ///< The resource
	SceneDArray<ModelPatchNode*> m_modelPatches;
	SceneDArray<Transform> m_transforms; ///< Cache the transforms of instances
	Timestamp m_transformsTimestamp;
};

/// @}

} // end namespace anki

#endif
