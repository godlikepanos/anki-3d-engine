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

	ANKI_USE_RESULT Error init(const Vec4& aabbMinLSpace, const Vec4& aabbMaxLSpace);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;

	class CubeSide
	{
	public:
		Transform m_localTrf;
	};

	Array<CubeSide, 6> m_cubeSides;
	Vec3 m_aabbMinLSpace = Vec3(+1.0f);
	Vec3 m_aabbMaxLSpace = Vec3(-1.0f);
	Aabb m_spatialAabb;

	DebugDrawer2 m_dbgDrawer;
	TextureResourcePtr m_dbgTex;

	void onMoveUpdate(MoveComponent& move);

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};
/// @}

} // end namespace anki
