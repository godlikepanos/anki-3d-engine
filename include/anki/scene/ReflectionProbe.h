// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_REFLECTION_PROBE_H
#define ANKI_SCENE_REFLECTION_PROBE_H

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
	ReflectionProbe(SceneGraph* scene)
		: SceneNode(scene)
	{}

	~ReflectionProbe();

	ANKI_USE_RESULT Error create(const CString& name, F32 radius);

private:
	Array<PerspectiveFrustum, 6> m_frustums;
	TexturePtr m_colorTex;
	TexturePtr m_depthTex;
	Array<FramebufferPtr, 6> m_fbs;
	U32 m_fbSize = 128;
	Array<Transform, 6> m_localFrustumTrfs;
	Sphere m_spatialSphere;

	void onMoveUpdate(MoveComponent& move);

	void createGraphics();
};
/// @}

} // end namespace anki

#endif

