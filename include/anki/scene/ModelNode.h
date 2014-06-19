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

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32, F32, SceneNode::UpdateType uptype) override;

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
};

/// The model scene node
class ModelNode: public SceneNode, public MoveComponent
{
	friend class ModelPatchNode;

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
		return *m_model;
	}
	/// @}

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32, F32, SceneNode::UpdateType uptype) override;

private:
	ModelResourcePointer m_model; ///< The resource
	SceneVector<Transform> m_transforms; ///< Cache the transforms of instances
	Timestamp m_transformsTimestamp;
};

/// @}

} // end namespace anki

#endif
