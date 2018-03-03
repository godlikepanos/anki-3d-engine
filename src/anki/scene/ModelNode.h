// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/resource/ModelResource.h>
#include <anki/collision/Obb.h>

namespace anki
{

// Forward
class ObbSpatialComponent;
class BodyComponent;
class ModelNode;

/// @addtogroup scene
/// @{

/// A fragment of the ModelNode
class ModelPatchNode : public SceneNode
{
	friend class ModelNode;

public:
	ModelPatchNode(SceneGraph* scene, CString name);

	~ModelPatchNode();

	ANKI_USE_RESULT Error init(const ModelPatch* modelPatch, U idx, const ModelNode& parent);

private:
	class MRenderComponent;

	Obb m_obb; ///< In world space. ModelNode will update it.
	const ModelPatch* m_modelPatch = nullptr; ///< The resource
	U64 m_mergeKey = 0;

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		el.m_callback = drawCallback;
		el.m_userData = this;
		el.m_mergeKey = m_mergeKey;
	}

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};

/// The model scene node.
class ModelNode : public SceneNode
{
	friend class ModelPatchNode;

public:
	ModelNode(SceneGraph* scene, CString name);

	~ModelNode();

	ANKI_USE_RESULT Error init(const CString& modelFname);

	const ModelResource& getModel() const
	{
		return *m_model;
	}

private:
	class MoveFeedbackComponent;
	class MRenderComponent;

	ModelResourcePtr m_model; ///< The resource
	DynamicArray<ModelPatchNode*> m_modelPatches;

	Obb m_obb;
	U64 m_mergeKey = 0;

	ShaderProgramResourcePtr m_dbgProg;

	Bool isSinglePatch() const
	{
		return m_modelPatches.getSize() == 0;
	}

	void onMoveComponentUpdate(const MoveComponent& move);

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		ANKI_ASSERT(isSinglePatch());
		el.m_callback = drawCallback;
		el.m_userData = this;
		el.m_mergeKey = m_mergeKey;
	}
};
/// @}

} // end namespace anki
