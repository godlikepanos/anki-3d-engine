// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/PackVisibleClusteredObjects.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

namespace anki {

Error PackVisibleClusteredObjects::init()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/PackVisibleClusteredObjects.ankiprogbin",
															m_packProg));

	U32 maxWaveSize = GrManager::getSingleton().getDeviceCapabilities().m_maxSubgroupSize;
	if(maxWaveSize == 16 || maxWaveSize == 32)
	{
		m_threadGroupSize = maxWaveSize;
	}
	else
	{
		m_threadGroupSize = 64;
	}

	for(ClusteredObjectType type = ClusteredObjectType::kFirst; type < ClusteredObjectType::kCount; ++type)
	{
		const ShaderProgramResourceVariant* variant;
		ShaderProgramResourceVariantInitInfo variantInit(m_packProg);
		variantInit.addMutation("OBJECT_TYPE", U32(type));
		variantInit.addMutation("THREAD_GROUP_SIZE", m_threadGroupSize);
		m_packProg->getOrCreateVariant(variantInit, variant);
		m_grProgs[type] = variant->getProgram();
	}

	U32 offset = 0;
	for(ClusteredObjectType type = ClusteredObjectType::kFirst; type < ClusteredObjectType::kCount; ++type)
	{
		alignRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_storageBufferBindOffsetAlignment, offset);
		m_structureBufferOffsets[type] = offset;
		offset += kMaxVisibleClusteredObjects[type] * kClusteredObjectSizes[type];
	}

	BufferInitInfo buffInit("ClusterObjects");
	buffInit.m_size = offset;
	buffInit.m_usage = BufferUsageBit::kAllCompute | BufferUsageBit::kAllGraphics;
	m_allClustererObjects = GrManager::getSingleton().newBuffer(buffInit);

	return Error::kNone;
}

template<typename TClustererType, ClusteredObjectType kType, typename TRenderQueueElement>
void PackVisibleClusteredObjects::dispatchType(WeakArray<TRenderQueueElement> array, const RenderQueue& rqueue,
											   CommandBufferPtr& cmdb)
{
	if(array.getSize() == 0)
	{
		return;
	}

	RebarGpuMemoryToken token;
	U32* indices = allocateStorage<U32*>(array.getSize() * sizeof(U32), token);

	RebarGpuMemoryToken extrasToken;
	PointLightExtra* plightExtras = nullptr;
	SpotLightExtra* slightExtras = nullptr;
	if constexpr(std::is_same_v<TClustererType, PointLight>)
	{
		plightExtras = allocateStorage<PointLightExtra*>(array.getSize() * sizeof(PointLightExtra), extrasToken);
	}
	else if constexpr(std::is_same_v<TClustererType, SpotLight>)
	{
		slightExtras = allocateStorage<SpotLightExtra*>(array.getSize() * sizeof(SpotLightExtra), extrasToken);
	}

	// Write ReBAR
	for(const auto& el : array)
	{
		*indices = el.m_index;
		++indices;

		if constexpr(std::is_same_v<TClustererType, PointLight>)
		{
			if(el.m_shadowRenderQueues[0] == nullptr)
			{
				plightExtras->m_shadowAtlasTileScale = -1.0f;
				plightExtras->m_shadowLayer = kMaxU32;
			}
			else
			{
				plightExtras->m_shadowLayer = el.m_shadowLayer;
				plightExtras->m_shadowAtlasTileScale = el.m_shadowAtlasTileSize;
				for(U32 f = 0; f < 6; ++f)
				{
					plightExtras->m_shadowAtlasTileOffsets[f] = Vec4(el.m_shadowAtlasTileOffsets[f], 0.0f, 0.0f);
				}
			}

			++plightExtras;
		}
		else if constexpr(std::is_same_v<TClustererType, SpotLight>)
		{
			slightExtras->m_shadowLayer = el.m_shadowLayer;
			slightExtras->m_textureMatrix = el.m_textureMatrix;

			++slightExtras;
		}
	}

	cmdb->bindStorageBuffer(0, 0, GpuSceneMemoryPool::getSingleton().getBuffer(),
							rqueue.m_clustererObjectsArrayOffsets[kType], rqueue.m_clustererObjectsArrayRanges[kType]);

	cmdb->bindStorageBuffer(0, 1, m_allClustererObjects, m_structureBufferOffsets[kType],
							array.getSize() * sizeof(TClustererType));

	cmdb->bindStorageBuffer(0, 2, RebarStagingGpuMemoryPool::getSingleton().getBuffer(), token.m_offset, token.m_range);

	if constexpr(std::is_same_v<TClustererType, PointLight> || std::is_same_v<TClustererType, SpotLight>)
	{
		cmdb->bindStorageBuffer(0, 3, RebarStagingGpuMemoryPool::getSingleton().getBuffer(), extrasToken.m_offset,
								extrasToken.m_range);
	}

	cmdb->bindShaderProgram(m_grProgs[kType]);

	const UVec4 pc(array.getSize());
	cmdb->setPushConstants(&pc, sizeof(pc));

	dispatchPPCompute(cmdb, m_threadGroupSize, 1, 1, array.getSize(), 1, 1);
}

void PackVisibleClusteredObjects::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("PackClusterObjects");

	pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kStorageComputeRead);

	m_allClustererObjectsHandle = rgraph.importBuffer(m_allClustererObjects, BufferUsageBit::kNone);
	pass.newBufferDependency(m_allClustererObjectsHandle, BufferUsageBit::kStorageComputeWrite);

	pass.setWork([&ctx, this](RenderPassWorkContext& rgraphCtx) {
		const RenderQueue& rqueue = *ctx.m_renderQueue;
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		dispatchType<PointLight, ClusteredObjectType::kPointLight>(rqueue.m_pointLights, rqueue, cmdb);
		dispatchType<SpotLight, ClusteredObjectType::kSpotLight>(rqueue.m_spotLights, rqueue, cmdb);
		dispatchType<Decal, ClusteredObjectType::kDecal>(rqueue.m_decals, rqueue, cmdb);
		dispatchType<FogDensityVolume, ClusteredObjectType::kFogDensityVolume>(rqueue.m_fogDensityVolumes, rqueue,
																			   cmdb);
		dispatchType<ReflectionProbe, ClusteredObjectType::kReflectionProbe>(rqueue.m_reflectionProbes, rqueue, cmdb);
		dispatchType<GlobalIlluminationProbe, ClusteredObjectType::kGlobalIlluminationProbe>(rqueue.m_giProbes, rqueue,
																							 cmdb);
	});
}

} // end namespace anki
