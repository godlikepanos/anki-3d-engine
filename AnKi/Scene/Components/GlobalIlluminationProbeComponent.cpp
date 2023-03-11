// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

GlobalIlluminationProbeComponent::GlobalIlluminationProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
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

	m_gpuSceneOffset = U32(node->getSceneGraph().getAllGpuSceneContiguousArrays().allocate(
		GpuSceneContiguousArrayType::kGlobalIlluminationProbes));

	const Error err = getExternalSubsystems(*node).m_resourceManager->loadResource(
		"ShaderBinaries/ClearTextureCompute.ankiprogbin", m_clearTextureProg);
	if(err)
	{
		ANKI_LOGF("Failed to load shader");
	}
}

GlobalIlluminationProbeComponent::~GlobalIlluminationProbeComponent()
{
}

void GlobalIlluminationProbeComponent::onDestroy(SceneNode& node)
{
	m_spatial.removeFromOctree(node.getSceneGraph().getOctree());

	node.getSceneGraph().getAllGpuSceneContiguousArrays().deferredFree(
		GpuSceneContiguousArrayType::kGlobalIlluminationProbes, m_gpuSceneOffset);
}

Error GlobalIlluminationProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = info.m_node->movedThisFrame() || m_shapeDirty;

	if(m_shapeDirty) [[unlikely]]
	{
		TextureInitInfo texInit("GiProbe");
		texInit.m_format =
			(getExternalSubsystems(*info.m_node).m_grManager->getDeviceCapabilities().m_unalignedBbpTextureFormats)
				? Format::kR16G16B16_Sfloat
				: Format::kR16G16B16A16_Sfloat;
		texInit.m_width = m_cellCounts.x() * 6;
		texInit.m_height = m_cellCounts.y();
		texInit.m_depth = m_cellCounts.z();
		texInit.m_type = TextureType::k3D;
		texInit.m_usage =
			TextureUsageBit::kAllSampled | TextureUsageBit::kImageComputeWrite | TextureUsageBit::kImageComputeRead;

		m_volTex = getExternalSubsystems(*info.m_node).m_grManager->newTexture(texInit);

		TextureViewInitInfo viewInit(m_volTex, "GiProbe");
		m_volView = getExternalSubsystems(*info.m_node).m_grManager->newTextureView(viewInit);

		m_volTexBindlessIdx = m_volView->getOrCreateBindlessTextureIndex();

		// Zero the texture
		const ShaderProgramResourceVariant* variant;
		ShaderProgramResourceVariantInitInfo variantInit(m_clearTextureProg);
		variantInit.addMutation("TEXTURE_DIMENSIONS", 3);
		variantInit.addMutation("COMPONENT_TYPE", 0);
		m_clearTextureProg->getOrCreateVariant(variantInit, variant);

		CommandBufferInitInfo cmdbInit("ClearGIVol");
		cmdbInit.m_flags = CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = getExternalSubsystems(*info.m_node).m_grManager->newCommandBuffer(cmdbInit);

		TextureBarrierInfo texBarrier;
		texBarrier.m_previousUsage = TextureUsageBit::kNone;
		texBarrier.m_nextUsage = TextureUsageBit::kImageComputeWrite;
		texBarrier.m_texture = m_volTex.get();
		cmdb->setPipelineBarrier({&texBarrier, 1}, {}, {});

		cmdb->bindShaderProgram(variant->getProgram());
		cmdb->bindImage(0, 0, m_volView);

		const Vec4 clearColor(0.0f);
		cmdb->setPushConstants(&clearColor, sizeof(clearColor));

		UVec3 wgSize;
		wgSize.x() = (8 - 1 + m_volTex->getWidth()) / 8;
		wgSize.y() = (8 - 1 + m_volTex->getHeight()) / 8;
		wgSize.z() = (8 - 1 + m_volTex->getDepth()) / 8;
		cmdb->dispatchCompute(wgSize.x(), wgSize.y(), wgSize.z());

		texBarrier.m_previousUsage = TextureUsageBit::kImageComputeWrite;
		texBarrier.m_nextUsage = m_volTex->getTextureUsage();
		cmdb->setPipelineBarrier({&texBarrier, 1}, {}, {});

		cmdb->flush();
	}

	if(updated) [[unlikely]]
	{
		m_shapeDirty = false;
		m_cellIdxToRefresh = 0;

		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		const Aabb aabb(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		m_spatial.setBoundingShape(aabb);

		// Upload to the GPU scene
		GpuSceneGlobalIlluminationProbe gpuProbe;
		gpuProbe.m_aabbMin = aabb.getMin().xyz();
		gpuProbe.m_aabbMax = aabb.getMax().xyz();
		gpuProbe.m_volumeTexture = m_volTexBindlessIdx;
		gpuProbe.m_halfTexelSizeU = 1.0f / F32(m_cellCounts.y()) / 2.0f;
		gpuProbe.m_fadeDistance = m_fadeDistance;

		getExternalSubsystems(*info.m_node)
			.m_gpuSceneMicroPatcher->newCopy(*info.m_framePool, m_gpuSceneOffset, sizeof(gpuProbe), &gpuProbe);
	}

	if(needsRefresh()) [[unlikely]]
	{
		updated = true;

		// Compute the position of the cell
		const Aabb aabb(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		U32 x, y, z;
		unflatten3dArrayIndex(m_cellCounts.x(), m_cellCounts.y(), m_cellCounts.z(), m_cellIdxToRefresh, x, y, z);
		const Vec3 cellSize = ((m_halfSize * 2.0f) / Vec3(m_cellCounts));
		const Vec3 halfCellSize = cellSize / 2.0f;
		const Vec3 cellCenter = aabb.getMin().xyz() + halfCellSize + cellSize * Vec3(UVec3(x, y, z));

		F32 effectiveDistance = max(m_halfSize.x(), m_halfSize.y());
		effectiveDistance = max(effectiveDistance, m_halfSize.z());
		effectiveDistance =
			max(effectiveDistance, getExternalSubsystems(*info.m_node).m_config->getSceneProbeEffectiveDistance());

		const F32 shadowCascadeDistance = min(
			effectiveDistance, getExternalSubsystems(*info.m_node).m_config->getSceneProbeShadowEffectiveDistance());

		for(U32 i = 0; i < 6; ++i)
		{
			m_frustums[i].setWorldTransform(
				Transform(cellCenter.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));

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
