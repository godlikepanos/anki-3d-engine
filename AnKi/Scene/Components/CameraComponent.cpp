// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Core/App.h>
#include <AnKi/Renderer/Renderer.h>

namespace anki {

CameraComponent::CameraComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
	// Init main frustum
	m_frustum.init(FrustumType::kPerspective);
	m_frustum.setWorldTransform(init.m_node->getWorldTransform());
	m_frustum.update();
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(info.m_node->movedThisFrame())
	{
		m_frustum.setWorldTransform(info.m_node->getWorldTransform());
	}

	updated = m_frustum.update();
}

Error CameraComponent::serialize(SceneSerializer& serializer)
{
	F32 near = m_frustum.getNear();
	ANKI_SERIALIZE(near, 1);

	F32 far = m_frustum.getFar();
	ANKI_SERIALIZE(far, 1);

	F32 fovX = m_frustum.getFovX();
	ANKI_SERIALIZE(fovX, 1);

	F32 fovY = m_frustum.getFovY();
	ANKI_SERIALIZE(fovY, 1);

	if(serializer.isInReadMode())
	{
		const Bool valid = near > kEpsilonf && far > near && far < kMaxF32 && fovX > 0.0f && fovX < kPi && fovY > 0.0f && fovY < kPi;
		if(!valid)
		{
			ANKI_SCENE_LOGE("Camera frustum values are out of range");
			return Error::kUserData;
		}

		// The ctor has already init()ed the frustum as perspective and set its world transform
		m_frustum.setPerspective(near, far, fovX, fovY);
	}

	return Error::kNone;
}

} // end namespace anki
