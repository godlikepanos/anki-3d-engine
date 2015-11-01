// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Frustum.h>
#include <anki/collision/Sphere.h>
#include <anki/Gr.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Probe used in realtime reflections.
class ReflectionProbe: public SceneNode
{
	friend class ReflectionProbeMoveFeedbackComponent;

public:
	const F32 FRUSTUM_NEAR_PLANE = 0.1 / 4.0;

	ReflectionProbe(SceneGraph* scene)
		: SceneNode(scene)
	{}

	~ReflectionProbe();

	ANKI_USE_RESULT Error create(const CString& name, F32 radius);

	U getCubemapArrayIndex() const
	{
		ANKI_ASSERT(m_cubemapArrayIdx < 0xFF);
		return m_cubemapArrayIdx;
	}

	void setCubemapArrayIndex(U cubemapArrayIdx)
	{
		ANKI_ASSERT(cubemapArrayIdx < 0xFF);
		m_cubemapArrayIdx = cubemapArrayIdx;
	}

private:
	class CubeSide
	{
	public:
		PerspectiveFrustum m_frustum;
		Transform m_localTrf;
	};

	Array<CubeSide, 6> m_cubeSides;
	Sphere m_spatialSphere;
	U8 m_cubemapArrayIdx = 0xFF; ///< Used by the renderer

	void onMoveUpdate(MoveComponent& move);
};
/// @}

} // end namespace anki

