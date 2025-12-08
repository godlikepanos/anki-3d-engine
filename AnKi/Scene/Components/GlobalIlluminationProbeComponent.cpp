// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/CVarSet.h>
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

void GlobalIlluminationProbeComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool moved = info.m_node->movedThisFrame();

	if(!m_dirty && !moved) [[likely]]
	{
		return;
	}

	updated = true;

	const Vec3 halfSize = info.m_node->getWorldTransform().getScale().xyz();
	UVec3 newCellCounts = UVec3(2.0f * halfSize / m_cellSize);
	newCellCounts = newCellCounts.max(UVec3(1));

	const Bool shapeDirty = m_cellCounts != newCellCounts;

	if(shapeDirty)
	{
		m_volTex.reset(nullptr);
		m_cellCounts = newCellCounts;
		m_totalCellCount = m_cellCounts.x() * m_cellCounts.y() * m_cellCounts.z();
	}

	if(moved)
	{
		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();
		m_halfSize = halfSize;
	}

	if(moved || shapeDirty)
	{
		m_cellsRefreshedCount = 0;
	}

	// (re-)create the volume texture
	if(!m_volTex)
	{
		TextureInitInfo texInit("GiProbe");
		texInit.m_format = (GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR16G16B16_Sfloat
																											: Format::kR16G16B16A16_Sfloat;
		texInit.m_width = m_cellCounts.x() * 6;
		texInit.m_height = m_cellCounts.y();
		texInit.m_depth = m_cellCounts.z();
		texInit.m_type = TextureType::k3D;
		texInit.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kUavCompute;

		m_volTex = GrManager::getSingleton().newTexture(texInit);
		m_volTexBindlessIdx = m_volTex->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());

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
		texBarrier.m_nextUsage = TextureUsageBit::kUavCompute;
		texBarrier.m_textureView = TextureView(m_volTex.get(), TextureSubresourceDesc::all());
		cmdb->setPipelineBarrier({&texBarrier, 1}, {}, {});

		cmdb->bindShaderProgram(&variant->getProgram());
		cmdb->bindUav(0, 0, TextureView(m_volTex.get(), TextureSubresourceDesc::all()));

		const Vec4 clearColor(0.0f);
		cmdb->setFastConstants(&clearColor, sizeof(clearColor));

		UVec3 wgSize;
		wgSize.x() = (8 - 1 + m_volTex->getWidth()) / 8;
		wgSize.y() = (8 - 1 + m_volTex->getHeight()) / 8;
		wgSize.z() = (8 - 1 + m_volTex->getDepth()) / 8;
		cmdb->dispatchCompute(wgSize.x(), wgSize.y(), wgSize.z());

		texBarrier.m_previousUsage = TextureUsageBit::kUavCompute;
		texBarrier.m_nextUsage = TextureUsageBit::kAllSrv; // Put something random, the renderer will start from kNone
		cmdb->setPipelineBarrier({&texBarrier, 1}, {}, {});

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());
	}

	// Upload to GPU scene
	if(moved || shapeDirty || m_dirty)
	{
		Bool cpuFeedback = false;
		if(m_cellsRefreshedCount == 0)
		{
			// Refresh starts over
			cpuFeedback = true;
		}
		else if(m_cellsRefreshedCount < m_totalCellCount)
		{
			// In the middle of the refresh process
			cpuFeedback = true;
		}
		else
		{
			// Refresh it done
			cpuFeedback = false;
		}

		// Upload to the GPU scene
		GpuSceneGlobalIlluminationProbe gpuProbe = {};

		const Aabb aabb(-m_halfSize + m_worldPos, m_halfSize + m_worldPos);
		gpuProbe.m_aabbMin = aabb.getMin().xyz();
		gpuProbe.m_aabbMax = aabb.getMax().xyz();

		gpuProbe.m_volumeTexture = m_volTexBindlessIdx;
		gpuProbe.m_halfTexelSizeU = 1.0f / (F32(m_cellCounts.y()) * 6.0f) / 2.0f;
		gpuProbe.m_fadeDistance = m_fadeDistance;
		gpuProbe.m_uuid = getUuid();
		gpuProbe.m_componentArrayIndex = getArrayIndex();
		gpuProbe.m_cpuFeedback = cpuFeedback;
		m_gpuSceneProbe.uploadToGpuScene(gpuProbe);
	}

	m_dirty = false;
}

F32 GlobalIlluminationProbeComponent::getRenderRadius() const
{
	F32 effectiveDistance = max(m_halfSize.x(), m_halfSize.y());
	effectiveDistance = max(effectiveDistance, m_halfSize.z());
	effectiveDistance = max<F32>(effectiveDistance, g_cvarSceneProbeEffectiveDistance);
	return effectiveDistance;
}

F32 GlobalIlluminationProbeComponent::getShadowsRenderRadius() const
{
	return min<F32>(getRenderRadius(), g_cvarSceneProbeShadowEffectiveDistance);
}

Error GlobalIlluminationProbeComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_cellSize, 1);
	ANKI_SERIALIZE(m_fadeDistance, 1);
	ANKI_SERIALIZE(m_halfSize, 1);

	return Error::kNone;
}

} // end namespace anki
