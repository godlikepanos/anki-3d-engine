// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Frustum.h>
#include <anki/collision/Sphere.h>
#include <anki/Gr.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Probe used in realtime reflections.
class ReflectionProbeNode : public SceneNode
{
public:
	const F32 EFFECTIVE_DISTANCE = 256.0f;

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
		PerspectiveFrustum m_frustum;
		Transform m_localTrf;
	};

	Array<CubeSide, 6> m_cubeSides;
	Vec3 m_aabbMinLSpace = Vec3(+1.0f);
	Vec3 m_aabbMaxLSpace = Vec3(-1.0f);
	Aabb m_spatialAabb;

	void onMoveUpdate(MoveComponent& move);
};
/// @}

} // end namespace anki
