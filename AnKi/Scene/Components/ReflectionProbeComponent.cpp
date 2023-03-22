// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

ReflectionProbeComponent::ReflectionProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_spatial(this)
{
	m_worldPos = node->getWorldTransform().getOrigin().xyz();

	for(U32 i = 0; i < 6; ++i)
	{
		m_frustums[i].init(FrustumType::kPerspective, &node->getMemoryPool());
		m_frustums[i].setPerspective(kClusterObjectFrustumNearPlane, 100.0f, kPi / 2.0f, kPi / 2.0f);
		m_frustums[i].setWorldTransform(
			Transform(m_worldPos.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));
		m_frustums[i].setShadowCascadeCount(1);
		m_frustums[i].update();
	}

	m_gpuSceneIndex =
		node->getSceneGraph().getAllGpuSceneContiguousArrays().allocate(GpuSceneContiguousArrayType::kReflectionProbes);
}

ReflectionProbeComponent::~ReflectionProbeComponent()
{
}

Error ReflectionProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool moved = info.m_node->movedThisFrame();
	const Bool shapeUpdated = m_dirty;
	m_dirty = false;
	updated = moved || shapeUpdated;

	if(shapeUpdated && !m_reflectionTex) [[unlikely]]
	{
		TextureInitInfo texInit("ReflectionProbe");
		texInit.m_format =
			(getExternalSubsystems(*info.m_node).m_grManager->getDeviceCapabilities().m_unalignedBbpTextureFormats)
				? Format::kR16G16B16_Sfloat
				: Format::kR16G16B16A16_Sfloat;
		texInit.m_width = getExternalSubsystems(*info.m_node).m_config->getSceneReflectionProbeResolution();
		texInit.m_height = texInit.m_width;
		texInit.m_mipmapCount = U8(computeMaxMipmapCount2d(texInit.m_width, texInit.m_height, 8));
		texInit.m_type = TextureType::kCube;
		texInit.m_usage = TextureUsageBit::kAllSampled | TextureUsageBit::kImageComputeWrite
						  | TextureUsageBit::kImageComputeRead | TextureUsageBit::kAllFramebuffer
						  | TextureUsageBit::kGenerateMipmaps;

		m_reflectionTex = getExternalSubsystems(*info.m_node).m_grManager->newTexture(texInit);

		TextureViewInitInfo viewInit(m_reflectionTex, "ReflectionPRobe");
		m_reflectionView = getExternalSubsystems(*info.m_node).m_grManager->newTextureView(viewInit);

		m_reflectionTexBindlessIndex = m_reflectionView->getOrCreateBindlessTextureIndex();
	}

	if(updated) [[unlikely]]
	{
		m_reflectionNeedsRefresh = true;

		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		F32 effectiveDistance = max(m_halfSize.x(), m_halfSize.y());
		effectiveDistance = max(effectiveDistance, m_halfSize.z());
		effectiveDistance =
			max(effectiveDistance, getExternalSubsystems(*info.m_node).m_config->getSceneProbeEffectiveDistance());

		const F32 shadowCascadeDistance = min(
			effectiveDistance, getExternalSubsystems(*info.m_node).m_config->getSceneProbeShadowEffectiveDistance());

		for(U32 i = 0; i < 6; ++i)
		{
			m_frustums[i].setWorldTransform(
				Transform(m_worldPos.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));

			m_frustums[i].setFar(effectiveDistance);
			m_frustums[i].setShadowCascadeDistance(0, shadowCascadeDistance);

			// Add something really far to force LOD 0 to be used. The importing tools create LODs with holes some times
			// and that causes the sky to bleed to GI rendering
			m_frustums[i].setLodDistances({effectiveDistance - 3.0f * kEpsilonf, effectiveDistance - 2.0f * kEpsilonf,
										   effectiveDistance - 1.0f * kEpsilonf});
		}

		const Aabb aabbWorld(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		m_spatial.setBoundingShape(aabbWorld);

		// Upload to the GPU scene
		GpuSceneReflectionProbe gpuProbe;
		gpuProbe.m_position = m_worldPos;
		gpuProbe.m_cubeTexture = m_reflectionTexBindlessIndex;
		gpuProbe.m_aabbMin = aabbWorld.getMin().xyz();
		gpuProbe.m_aabbMax = aabbWorld.getMax().xyz();

		const PtrSize offset = m_gpuSceneIndex * sizeof(GpuSceneReflectionProbe)
							   + info.m_node->getSceneGraph().getAllGpuSceneContiguousArrays().getArrayBase(
								   GpuSceneContiguousArrayType::kReflectionProbes);
		getExternalSubsystems(*info.m_node)
			.m_gpuSceneMicroPatcher->newCopy(*info.m_framePool, offset, sizeof(gpuProbe), &gpuProbe);
	}

	// Update spatial and frustums
	const Bool spatialUpdated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
	updated = updated || spatialUpdated;

	for(U32 i = 0; i < 6; ++i)
	{
		const Bool frustumUpdated = m_frustums[i].update();
		updated = updated || frustumUpdated;
	}

	return Error::kNone;
}

void ReflectionProbeComponent::onDestroy(SceneNode& node)
{
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());

	node.getSceneGraph().getAllGpuSceneContiguousArrays().deferredFree(GpuSceneContiguousArrayType::kReflectionProbes,
																	   m_gpuSceneIndex);
}

} // end namespace anki
