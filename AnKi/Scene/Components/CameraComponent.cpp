// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Core/App.h>

namespace anki {

CameraComponent::CameraComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	// Init main frustum
	m_frustum.init(FrustumType::kPerspective);
	m_frustum.setWorldTransform(node->getWorldTransform());
	m_frustum.update();
}

CameraComponent::~CameraComponent()
{
}

Error CameraComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(info.m_node->movedThisFrame())
	{
		m_frustum.setWorldTransform(info.m_node->getWorldTransform());
	}

	updated = m_frustum.update();

	return Error::kNone;
}

} // end namespace anki
