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

	if(m_fovYDerivesByAspect)
	{
		const F32 desiredFovY = m_frustum.getFovX() / Renderer::getSingleton().getAspectRatio();
		if(m_frustum.getFovY() != desiredFovY)
		{
			m_frustum.setFovY(desiredFovY);
		}
	}

	updated = m_frustum.update();
}

Error CameraComponent::serialize(SceneSerializer& serializer)
{
	Frustum frustum;

	F32 near = m_frustum.getNear();
	ANKI_SERIALIZE(near, 1);
	frustum.setNear(near);

	F32 far = m_frustum.getFar();
	ANKI_SERIALIZE(far, 1);
	frustum.setFar(far);

	F32 fovX = m_frustum.getFovX();
	ANKI_SERIALIZE(fovX, 1);
	frustum.setFovX(fovX);

	F32 fovY = m_frustum.getFovY();
	ANKI_SERIALIZE(fovY, 1);
	frustum.setFovY(fovY);

	if(serializer.isInReadMode())
	{
		m_frustum = frustum;
	}

	return Error::kNone;
}

} // end namespace anki
