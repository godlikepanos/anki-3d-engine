// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/resource/ModelResource.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

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
	class FeedbackToSpatialComponent;

	ModelResourcePtr m_model; ///< The resource

	Aabb m_aabbLocal;
	U64 m_mergeKey = 0;
	U32 m_modelPatchIdx = 0;

	DebugDrawer2 m_dbgDrawer;

	void updateSpatialComponent(const MoveComponent& move, const SkinComponent* skinc);

	void draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) const;

	static void setupRayTracingInstanceQueueElement(U32 lod, const void* userData, RayTracingInstanceQueueElement& el);
};
/// @}

} // end namespace anki
