// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

GlobalIlluminationProbeComponent::GlobalIlluminationProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_spatial(this)
{
	for(U32 i = 0; i < 6; ++i)
	{
		m_frustums[i].init(FrustumType::kPerspective, &node->getMemoryPool());
		m_frustums[i].setPerspective(kClusterObjectFrustumNearPlane, 100.0f, kPi / 2.0f, kPi / 2.0f);
		m_frustums[i].setWorldTransform(
			Transform(node->getWorldTransform().getOrigin(), Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));
		m_frustums[i].setShadowCascadeCount(1);
		m_frustums[i].update();
	}
}

GlobalIlluminationProbeComponent::~GlobalIlluminationProbeComponent()
{
}

void GlobalIlluminationProbeComponent::onDestroy(SceneNode& node)
{
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());
}

Error GlobalIlluminationProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = info.m_node->movedThisFrame() || m_shapeDirty;

	if(updated) [[unlikely]]
	{
		m_shapeDirty = false;

		// Set a new UUID to force the renderer to update the probe
		m_uuid = info.m_node->getSceneGraph().getNewUuid();

		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		const Aabb aabb(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		m_spatial.setBoundingShape(aabb);
	}

	if(m_markedForRendering) [[unlikely]]
	{
		updated = true;

		F32 effectiveDistance = max(m_halfSize.x(), m_halfSize.y());
		effectiveDistance = max(effectiveDistance, m_halfSize.z());
		effectiveDistance =
			max(effectiveDistance, getExternalSubsystems(*info.m_node).m_config->getSceneProbeEffectiveDistance());

		const F32 shadowCascadeDistance = min(
			effectiveDistance, getExternalSubsystems(*info.m_node).m_config->getSceneProbeShadowEffectiveDistance());

		for(U32 i = 0; i < 6; ++i)
		{
			m_frustums[i].setWorldTransform(
				Transform(m_renderPosition.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));

			m_frustums[i].setFar(effectiveDistance);
			m_frustums[i].setShadowCascadeDistance(0, shadowCascadeDistance);

			// Add something really far to force LOD 0 to be used. The importing tools create LODs with holes some times
			// and that causes the sky to bleed to GI rendering
			m_frustums[i].setLodDistances({effectiveDistance - 3.0f * kEpsilonf, effectiveDistance - 2.0f * kEpsilonf,
										   effectiveDistance - 1.0f * kEpsilonf});
		}
	}

	for(U32 i = 0; i < 6; ++i)
	{
		const Bool frustumUpdated = m_frustums[i].update();
		updated = updated || frustumUpdated;
	}

	const Bool spatialUpdated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
	updated = updated || spatialUpdated;

	return Error::kNone;
}

} // end namespace anki
