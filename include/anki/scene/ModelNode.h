// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/RenderComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/resource/Model.h>
#include <anki/collision/Obb.h>

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
	Obb m_obb; ///< In world space. ModelNode will update it.
	const ModelPatch* m_modelPatch = nullptr; ///< The resource

	ANKI_USE_RESULT Error buildRendering(RenderingBuildInfo& data) const;
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

private:
	ModelResourcePtr m_model; ///< The resource
	DArray<ModelPatchNode*> m_modelPatches;

	void onMoveComponentUpdate(MoveComponent& move);
};
/// @}

} // end namespace anki

