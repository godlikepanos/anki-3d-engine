// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/SceneNode.h"
#include "anki/collision/Frustum.h"
#include "anki/collision/Sphere.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Dummy component to define a type.
class ReflectionProbeComponent: public SceneComponent
{
public:
	ReflectionProbeComponent(SceneNode* node)
		: SceneComponent(SceneComponent::Type::REFLECTION_PROBE, node)
	{}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::REFLECTION_PROBE;
	}
};

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

private:
	class CubeSide
	{
	public:
		PerspectiveFrustum m_frustum;
		Transform m_localTrf;
		FramebufferPtr m_fb;
	};

	Array<CubeSide, 6> m_cubeSides;

	TexturePtr m_colorTex;
	U32 m_fbSize = 128;
	Sphere m_spatialSphere;

	void onMoveUpdate(MoveComponent& move);

	void createGraphics();
};
/// @}

} // end namespace anki

