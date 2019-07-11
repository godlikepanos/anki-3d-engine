// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/DebugDrawer.h>
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

	ModelResourcePtr m_model; ///< The resource

	Obb m_obb;
	U64 m_mergeKey = 0;
	U32 m_modelPatchIdx = 0;

	DebugDrawer2 m_dbgDrawer;

	void onMoveComponentUpdate(const MoveComponent& move);

	void draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) const;
};
/// @}

} // end namespace anki
