// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

CameraComponent::CameraComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	const ConfigSet& config = ConfigSet::getSingleton();

	// Init main frustum
	m_frustum.init(FrustumType::kPerspective, &node->getMemoryPool());

	m_frustum.setLodDistance(0, config.getLod0MaxDistance());
	m_frustum.setLodDistance(1, config.getLod1MaxDistance());
	m_frustum.setShadowCascadeCount(config.getSceneShadowCascadeCount());

	static_assert(kMaxShadowCascades == 4);
	m_frustum.setShadowCascadeDistance(0, config.getSceneShadowCascade0Distance());
	m_frustum.setShadowCascadeDistance(1, config.getSceneShadowCascade1Distance());
	m_frustum.setShadowCascadeDistance(2, config.getSceneShadowCascade2Distance());
	m_frustum.setShadowCascadeDistance(3, config.getSceneShadowCascade3Distance());

	m_frustum.setWorldTransform(node->getWorldTransform());

	m_frustum.setEarlyZDistance(config.getSceneEarlyZDistance());

	m_frustum.update();

	// Init extended frustum
	m_usesExtendedFrustum =
		GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled && config.getSceneRayTracedShadows();

	if(m_usesExtendedFrustum)
	{
		m_extendedFrustum.init(FrustumType::kOrthographic, &node->getMemoryPool());

		const F32 dist = config.getSceneRayTracingExtendedFrustumDistance();

		m_extendedFrustum.setOrthographic(0.1f, dist * 2.0f, dist, -dist, dist, -dist);
		m_extendedFrustum.setWorldTransform(computeExtendedFrustumTransform(node->getWorldTransform()));

		m_extendedFrustum.update();
	}
}

CameraComponent::~CameraComponent()
{
}

Error CameraComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(info.m_node->movedThisFrame())
	{
		m_frustum.setWorldTransform(info.m_node->getWorldTransform());

		if(m_usesExtendedFrustum)
		{
			m_extendedFrustum.setWorldTransform(computeExtendedFrustumTransform(info.m_node->getWorldTransform()));
		}
	}

	updated = m_frustum.update();

	if(m_usesExtendedFrustum)
	{
		const Bool extendedUpdated = m_extendedFrustum.update();
		updated = updated || extendedUpdated;
	}

	return Error::kNone;
}

Transform CameraComponent::computeExtendedFrustumTransform(const Transform& cameraTransform) const
{
	const F32 far = m_extendedFrustum.getFar();
	Transform extendedFrustumTransform = Transform::getIdentity();
	Vec3 newOrigin = cameraTransform.getOrigin().xyz();
	newOrigin.z() += far / 2.0f;
	extendedFrustumTransform.setOrigin(newOrigin.xyz0());

	return extendedFrustumTransform;
}

} // end namespace anki
