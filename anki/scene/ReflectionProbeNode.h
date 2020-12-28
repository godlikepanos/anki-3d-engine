// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Aabb.h>
#include <anki/scene/DebugDrawer.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Probe used in realtime reflections.
class ReflectionProbeNode : public SceneNode
{
public:
	ReflectionProbeNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~ReflectionProbeNode();

	ANKI_USE_RESULT Error init();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	class CubeSide
	{
	public:
		Transform m_localTrf;
	};

	Array<CubeSide, 6> m_cubeSides;

	DebugDrawer2 m_dbgDrawer;
	TextureResourcePtr m_dbgTex;

	void onMoveUpdate(MoveComponent& move);
	void onShapeUpdate(ReflectionProbeComponent& reflc);

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};
/// @}

} // end namespace anki
