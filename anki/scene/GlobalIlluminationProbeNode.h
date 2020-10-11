// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/collision/Aabb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Probe used in global illumination.
class GlobalIlluminationProbeNode : public SceneNode
{
public:
	GlobalIlluminationProbeNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~GlobalIlluminationProbeNode();

	ANKI_USE_RESULT Error init();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	Array<Transform, 6> m_cubeFaceTransforms;
	Aabb m_spatialAabb = Aabb(Vec3(-1.0f), Vec3(1.0f));
	Vec4 m_previousPosition = Vec4(0.0f);

	DebugDrawer2 m_dbgDrawer;
	TextureResourcePtr m_dbgTex;

	void onMoveUpdate(MoveComponent& move);
	void onShapeUpdateOrProbeNeedsRendering();

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};
/// @}

} // end namespace anki
