// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/TextureView.h>

namespace anki {

NumericCVar<U32> g_reflectionProbeResolutionCVar(CVarSubsystem::kScene, "ReflectionProbeResolution", 128, 8, 2048,
												 "The resolution of the reflection probe's reflection");

ReflectionProbeComponent::ReflectionProbeComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_worldPos = node->getWorldTransform().getOrigin().xyz();
	m_gpuSceneProbe.allocate();

	TextureInitInfo texInit("ReflectionProbe");
	texInit.m_format =
		(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR16G16B16_Sfloat : Format::kR16G16B16A16_Sfloat;
	texInit.m_width = g_reflectionProbeResolutionCVar.get();
	texInit.m_height = texInit.m_width;
	texInit.m_mipmapCount = U8(computeMaxMipmapCount2d(texInit.m_width, texInit.m_height, 8));
	texInit.m_type = TextureType::kCube;
	texInit.m_usage = TextureUsageBit::kAllSampled | TextureUsageBit::kUavComputeWrite | TextureUsageBit::kUavComputeRead
					  | TextureUsageBit::kAllFramebuffer | TextureUsageBit::kGenerateMipmaps;

	m_reflectionTex = GrManager::getSingleton().newTexture(texInit);

	TextureViewInitInfo viewInit(m_reflectionTex.get(), "ReflectionProbe");
	m_reflectionView = GrManager::getSingleton().newTextureView(viewInit);

	m_reflectionTexBindlessIndex = m_reflectionView->getOrCreateBindlessTextureIndex();
}

ReflectionProbeComponent::~ReflectionProbeComponent()
{
}

Error ReflectionProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
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
		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		// Update the UUID
		m_uuid = (m_reflectionNeedsRefresh) ? SceneGraph::getSingleton().getNewUuid() : 0;

		// Upload to the GPU scene
		GpuSceneReflectionProbe gpuProbe;
		gpuProbe.m_position = m_worldPos;
		gpuProbe.m_cubeTexture = m_reflectionTexBindlessIndex;

		const Aabb aabbWorld(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		gpuProbe.m_aabbMin = aabbWorld.getMin().xyz();
		gpuProbe.m_aabbMax = aabbWorld.getMax().xyz();

		gpuProbe.m_uuid = m_uuid;
		gpuProbe.m_componentArrayIndex = getArrayIndex();
		m_gpuSceneProbe.uploadToGpuScene(gpuProbe);
	}

	return Error::kNone;
}

F32 ReflectionProbeComponent::getRenderRadius() const
{
	F32 effectiveDistance = max(m_halfSize.x(), m_halfSize.y());
	effectiveDistance = max(effectiveDistance, m_halfSize.z());
	effectiveDistance = max(effectiveDistance, g_probeEffectiveDistanceCVar.get());
	return effectiveDistance;
}

F32 ReflectionProbeComponent::getShadowsRenderRadius() const
{
	return min(getRenderRadius(), g_probeShadowEffectiveDistanceCVar.get());
}

} // end namespace anki
