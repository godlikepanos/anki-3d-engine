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

static NumericCVar<U8> g_shadowCascadeCountCVar(CVarSubsystem::kScene, "ShadowCascadeCount", (ANKI_PLATFORM_MOBILE) ? 2 : kMaxShadowCascades, 1,
												kMaxShadowCascades, "Max number of shadow cascades for directional lights");
static NumericCVar<F32> g_shadowCascade0DistanceCVar(CVarSubsystem::kScene, "ShadowCascade0Distance", 18.0, 1.0, kMaxF32,
													 "The distance of the 1st cascade");
static NumericCVar<F32> g_shadowCascade1DistanceCVar(CVarSubsystem::kScene, "ShadowCascade1Distance", 40.0, 1.0, kMaxF32,
													 "The distance of the 2nd cascade");
static NumericCVar<F32> g_shadowCascade2DistanceCVar(CVarSubsystem::kScene, "ShadowCascade2Distance", 80.0, 1.0, kMaxF32,
													 "The distance of the 3rd cascade");
static NumericCVar<F32> g_shadowCascade3DistanceCVar(CVarSubsystem::kScene, "ShadowCascade3Distance", 200.0, 1.0, kMaxF32,
													 "The distance of the 4th cascade");
static NumericCVar<F32> g_earyZDistanceCVar(CVarSubsystem::kScene, "EarlyZDistance", (ANKI_PLATFORM_MOBILE) ? 0.0f : 10.0f, 0.0f, kMaxF32,
											"Objects with distance lower than that will be used in early Z");
BoolCVar g_rayTracedShadowsCVar(CVarSubsystem::kScene, "RayTracedShadows", true, "Enable or not ray traced shadows. Ignored if RT is not supported");
static NumericCVar<F32>
	g_rayTracingExtendedFrustumDistanceCVar(CVarSubsystem::kScene, "RayTracingExtendedFrustumDistance", 100.0f, 10.0f, 10000.0f,
											"Every object that its distance from the camera is bellow that value will take part in ray tracing");

CameraComponent::CameraComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
	// Init main frustum
	m_frustum.init(FrustumType::kPerspective);

	m_frustum.setLodDistance(0, g_lod0MaxDistanceCVar.get());
	m_frustum.setLodDistance(1, g_lod1MaxDistanceCVar.get());
	m_frustum.setShadowCascadeCount(g_shadowCascadeCountCVar.get());

	static_assert(kMaxShadowCascades == 4);
	m_frustum.setShadowCascadeDistance(0, g_shadowCascade0DistanceCVar.get());
	m_frustum.setShadowCascadeDistance(1, g_shadowCascade1DistanceCVar.get());
	m_frustum.setShadowCascadeDistance(2, g_shadowCascade2DistanceCVar.get());
	m_frustum.setShadowCascadeDistance(3, g_shadowCascade3DistanceCVar.get());

	m_frustum.setWorldTransform(node->getWorldTransform());

	m_frustum.setEarlyZDistance(g_earyZDistanceCVar.get());

	m_frustum.update();

	// Init extended frustum
	m_usesExtendedFrustum = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled && g_rayTracedShadowsCVar.get();

	if(m_usesExtendedFrustum)
	{
		m_extendedFrustum.init(FrustumType::kOrthographic);

		const F32 dist = g_rayTracingExtendedFrustumDistanceCVar.get();

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
