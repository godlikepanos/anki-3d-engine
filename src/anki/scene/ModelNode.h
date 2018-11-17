// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/resource/ModelResource.h>
#include <anki/collision/Obb.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

// Forward
class ObbSpatialComponent;
class BodyComponent;
class ModelNode;

/// @addtogroup scene
/// @{

/// The model scene node.
class ModelNode : public SceneNode
{
	friend class ModelPatchNode;

public:
	ModelNode(SceneGraph* scene, CString name);

	~ModelNode();

	ANKI_USE_RESULT Error init(const CString& modelFname);

	ANKI_USE_RESULT Error init(ModelResourcePtr resource, U32 modelPatchIdx);

private:
	class MoveFeedbackComponent;
	class MyRenderComponent;

	ModelResourcePtr m_model; ///< The resource

	Obb m_obb;
	U64 m_mergeKey = 0;
	U32 m_modelPatchIdx = 0;

	ShaderProgramResourcePtr m_dbgProg;

	void onMoveComponentUpdate(const MoveComponent& move);

	void draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) const;

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		const ModelNode& self = *static_cast<const ModelNode*>(userData[0]);
		self.draw(ctx, userData);
	}

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		el.m_callback = drawCallback;
		el.m_userData = this;
		el.m_mergeKey = m_mergeKey;
	}
};
/// @}

} // end namespace anki
