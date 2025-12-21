// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Gr/Texture.h>

namespace anki {

ReflectionProbeComponent::ReflectionProbeComponent(SceneNode* node, U32 uuid)
	: SceneComponent(node, kClassType, uuid)
{
	m_worldPos = node->getWorldTransform().getOrigin().xyz;
	m_gpuSceneProbe.allocate();

	TextureInitInfo texInit("ReflectionProbe");
	texInit.m_format =
		(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR16G16B16_Sfloat : Format::kR16G16B16A16_Sfloat;
	texInit.m_width = g_cvarRenderProbeReflectionsResolution;
	texInit.m_height = texInit.m_width;
	texInit.m_mipmapCount = computeMaxMipmapCount2d(texInit.m_width, texInit.m_height, 8);
	texInit.m_type = TextureType::kCube;
	texInit.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kUavCompute | TextureUsageBit::kAllRtvDsv;

	m_reflectionTex = GrManager::getSingleton().newTexture(texInit);

	m_reflectionTexBindlessIndex = m_reflectionTex->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
}

ReflectionProbeComponent::~ReflectionProbeComponent()
{
}

void ReflectionProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool moved = info.m_node->movedThisFrame();
	updated = moved || m_dirty;
	m_dirty = false;

	if(moved) [[unlikely]]
	{
		m_reflectionNeedsRefresh = true;
	}

	if(updated) [[unlikely]]
	{
		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz;
		m_halfSize = info.m_node->getWorldTransform().getScale().xyz;

		// Upload to the GPU scene
		GpuSceneReflectionProbe gpuProbe;
		gpuProbe.m_position = m_worldPos;
		gpuProbe.m_cubeTexture = m_reflectionTexBindlessIndex;

		const Aabb aabbWorld(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		gpuProbe.m_aabbMin = aabbWorld.getMin().xyz;
		gpuProbe.m_aabbMax = aabbWorld.getMax().xyz;

		gpuProbe.m_componentUuid = getUuid();
		gpuProbe.m_componentArrayIndex = getArrayIndex();
		gpuProbe.m_cpuFeedback = m_reflectionNeedsRefresh;
		gpuProbe.m_sceneNodeUuid = info.m_node->getUuid();
		m_gpuSceneProbe.uploadToGpuScene(gpuProbe);
	}
}

F32 ReflectionProbeComponent::getRenderRadius() const
{
	F32 effectiveDistance = max(m_halfSize.x, m_halfSize.y);
	effectiveDistance = max(effectiveDistance, m_halfSize.z);
	effectiveDistance = max<F32>(effectiveDistance, g_cvarSceneProbeEffectiveDistance);
	return effectiveDistance;
}

F32 ReflectionProbeComponent::getShadowsRenderRadius() const
{
	return min<F32>(getRenderRadius(), g_cvarSceneProbeShadowEffectiveDistance);
}

Error ReflectionProbeComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_halfSize, 1);
	return Error::kNone;
}

} // end namespace anki
