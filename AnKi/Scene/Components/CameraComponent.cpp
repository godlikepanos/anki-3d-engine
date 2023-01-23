// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

CameraComponent::CameraComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	m_frustum.init(FrustumType::kPerspective, &node->getMemoryPool());
}

CameraComponent::~CameraComponent()
{
}

Error CameraComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(m_moveComponent) [[likely]]
	{
		updated = m_frustum.update(m_moveComponent->wasDirtyThisFrame(), m_moveComponent->getWorldTransform());
	}
	else
	{
		updated = m_frustum.update(true, Transform::getIdentity());
	}

	return Error::kNone;
}

void CameraComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	if(other->getClassId() != MoveComponent::getStaticClassId())
	{
		return;
	}

	MoveComponent* movec = static_cast<MoveComponent*>(other);
	if(added)
	{
		m_moveComponent = movec;
	}
	else if(m_moveComponent == movec)
	{
		m_moveComponent = nullptr;
	}
}

void CameraComponent::fillCoverage(void* userData, F32* depthValues, U32 width, U32 height)
{
	ANKI_ASSERT(userData && depthValues && width > 0 && height > 0);

	CameraComponent& self = *static_cast<CameraComponent*>(userData);
	self.m_frustum.setCoverageBuffer(depthValues, width, height);
}

} // end namespace anki
