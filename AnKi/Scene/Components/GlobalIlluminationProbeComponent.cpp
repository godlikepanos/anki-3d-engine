// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/CommandBuffer.h>

namespace anki {

GlobalIlluminationProbeComponent::GlobalIlluminationProbeComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneProbe.allocate();

	const Error err = ResourceManager::getSingleton().loadResource("ShaderBinaries/ClearTextureCompute.ankiprogbin", m_clearTextureProg);
	if(err)
	{
		ANKI_LOGF("Failed to load shader");
	}
}

GlobalIlluminationProbeComponent::~GlobalIlluminationProbeComponent()
{
}

Error GlobalIlluminationProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool moved = info.m_node->movedThisFrame();
	updated = moved || m_shapeDirty || m_refreshDirty;

	if(moved || m_shapeDirty)
	{
		m_cellsRefreshedCount = 0;
	}

	// (re-)create the volume texture
	if(m_shapeDirty) [[unlikely]]
	{
		TextureInitInfo texInit("GiProbe");
		texInit.m_format = (GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR16G16B16_Sfloat
																											: Format::kR16G16B16A16_Sfloat;
		texInit.m_width = m_cellCounts.x() * 6;
		texInit.m_height = m_cellCounts.y();
		texInit.m_depth = m_cellCounts.z();
		texInit.m_type = TextureType::k3D;
		texInit.m_usage = TextureUsageBit::kAllSampled | TextureUsageBit::kUavComputeWrite | TextureUsageBit::kUavComputeRead;

		m_volTex = GrManager::getSingleton().newTexture(texInit);

		TextureViewInitInfo viewInit(m_volTex.get(), "GiProbe");
		m_volView = GrManager::getSingleton().newTextureView(viewInit);

		m_volTexBindlessIdx = m_volView->getOrCreateBindlessTextureIndex();

		// Zero the texture
		const ShaderProgramResourceVariant* variant;
		ShaderProgramResourceVariantInitInfo variantInit(m_clearTextureProg);
		variantInit.addMutation("TEXTURE_DIMENSIONS", 3);
		variantInit.addMutation("COMPONENT_TYPE", 0);
		m_clearTextureProg->getOrCreateVariant(variantInit, variant);

		CommandBufferInitInfo cmdbInit("ClearGIVol");
		cmdbInit.m_flags = CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		TextureBarrierInfo texBarrier;
		texBarrier.m_previousUsage = TextureUsageBit::kNone;
		texBarrier.m_nextUsage = TextureUsageBit::kUavComputeWrite;
		texBarrier.m_texture = m_volTex.get();
		cmdb->setPipelineBarrier({&texBarrier, 1}, {}, {});

		cmdb->bindShaderProgram(&variant->getProgram());
		cmdb->bindUavTexture(0, 0, m_volView.get());

		const Vec4 clearColor(0.0f);
		cmdb->setPushConstants(&clearColor, sizeof(clearColor));

		UVec3 wgSize;
		wgSize.x() = (8 - 1 + m_volTex->getWidth()) / 8;
		wgSize.y() = (8 - 1 + m_volTex->getHeight()) / 8;
		wgSize.z() = (8 - 1 + m_volTex->getDepth()) / 8;
		cmdb->dispatchCompute(wgSize.x(), wgSize.y(), wgSize.z());

		texBarrier.m_previousUsage = TextureUsageBit::kUavComputeWrite;
		texBarrier.m_nextUsage = m_volTex->getTextureUsage();
		cmdb->setPipelineBarrier({&texBarrier, 1}, {}, {});

		cmdb->flush();
	}

	// Any update
	if(updated) [[unlikely]]
	{
		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		// Change the UUID
		if(m_cellsRefreshedCount == 0)
		{
			// Refresh starts over, get a new UUID
			m_uuid = SceneGraph::getSingleton().getNewUuid();
		}
		else if(m_cellsRefreshedCount < m_totalCellCount)
		{
			// In the middle of the refresh process
			ANKI_ASSERT(m_uuid != 0);
		}
		else
		{
			// Refresh it done
			m_uuid = 0;
		}

		// Upload to the GPU scene
		GpuSceneGlobalIlluminationProbe gpuProbe = {};

		const Aabb aabb(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		gpuProbe.m_aabbMin = aabb.getMin().xyz();
		gpuProbe.m_aabbMax = aabb.getMax().xyz();

		gpuProbe.m_volumeTexture = m_volTexBindlessIdx;
		gpuProbe.m_halfTexelSizeU = 1.0f / (F32(m_cellCounts.y()) * 6.0f) / 2.0f;
		gpuProbe.m_fadeDistance = m_fadeDistance;
		gpuProbe.m_uuid = m_uuid;
		gpuProbe.m_componentArrayIndex = getArrayIndex();
		m_gpuSceneProbe.uploadToGpuScene(gpuProbe);
	}

	m_shapeDirty = false;
	m_refreshDirty = false;

	return Error::kNone;
}

F32 GlobalIlluminationProbeComponent::getRenderRadius() const
{
	F32 effectiveDistance = max(m_halfSize.x(), m_halfSize.y());
	effectiveDistance = max(effectiveDistance, m_halfSize.z());
	effectiveDistance = max(effectiveDistance, g_probeEffectiveDistanceCVar.get());
	return effectiveDistance;
}

F32 GlobalIlluminationProbeComponent::getShadowsRenderRadius() const
{
	return min(getRenderRadius(), g_probeShadowEffectiveDistanceCVar.get());
}

} // end namespace anki
