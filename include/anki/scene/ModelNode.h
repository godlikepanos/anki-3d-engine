// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/SceneNode.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/resource/Model.h"
#include "anki/collision/Obb.h"

namespace anki {

// Forward
class ObbSpatialComponent;
class BodyComponent;

/// @addtogroup scene
/// @{

/// A fragment of the ModelNode
class ModelPatchNode: public SceneNode
{
	friend class ModelNode;
	friend class ModelPatchRenderComponent;

public:
	ModelPatchNode(SceneGraph* scene);

	~ModelPatchNode();

	ANKI_USE_RESULT Error create(
		const CString& name, const ModelPatch* modelPatch);

private:
	Obb m_obb; ///< In world space
	const ModelPatch* m_modelPatch; ///< The resource
	DArray<ObbSpatialComponent*> m_spatials;

	void updateInstanceSpatials(
		const MoveComponent* instanceMoves[],
		U32 instanceMovesCount);

	ANKI_USE_RESULT Error buildRendering(RenderingBuildData& data) const;

	void getRenderWorldTransform(U index, Transform& trf) const;
};

/// The model scene node
class ModelNode: public SceneNode
{
	friend class ModelPatchNode;
	friend class ModelMoveFeedbackComponent;

public:
	ModelNode(SceneGraph* scene);

	~ModelNode();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& modelFname);

	const Model& getModel() const
	{
		return *m_model;
	}

	/// Override SceneNode::frameUpdate
	ANKI_USE_RESULT Error frameUpdate(F32, F32) override;

private:
	ModelResourcePtr m_model; ///< The resource
	DArray<ModelPatchNode*> m_modelPatches;
	DArray<Transform> m_transforms; ///< Cache the transforms of instances
	Timestamp m_transformsTimestamp;

	void onMoveComponentUpdate(MoveComponent& move);
};
/// @}

} // end namespace anki

