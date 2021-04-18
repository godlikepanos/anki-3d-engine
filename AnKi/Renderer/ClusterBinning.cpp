// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki
{

ClusterBinning::ClusterBinning(Renderer* r)
	: RendererObject(r)
{
}

ClusterBinning::~ClusterBinning()
{
}

Error ClusterBinning::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing clusterer binning");

	ANKI_CHECK(getResourceManager().loadResource("Shaders/ClusterBinning.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("TILE_SIZE", m_r->getTileSize());
	variantInitInfo.addConstant("TILE_COUNT_X", m_r->getTileCounts().x());
	variantInitInfo.addConstant("TILE_COUNT_Y", m_r->getTileCounts().y());
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("RENDERING_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	m_clusterCount = m_r->getTileCounts().x() * m_r->getTileCounts().y() * m_r->getZSplitCount();

	return Error::NONE;
}

void ClusterBinning::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cluster Binning");

	const RenderQueue& rqueue = *m_runCtx.m_ctx->m_renderQueue;
	if(ANKI_LIKELY(rqueue.m_pointLights.getSize() || rqueue.m_spotLights.getSize() || rqueue.m_decals.getSize()
				   || rqueue.m_reflectionProbes.getSize() || rqueue.m_fogDensityVolumes.getSize()
				   || rqueue.m_giProbes.getSize()))
	{
		pass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<ClusterBinning*>(rgraphCtx.m_userData)->run(rgraphCtx);
			},
			this, 0);
	}

	// Allocate clusters. Store to a 8byte aligned ptr. Maybe that will trick the compiler to memset faster
	U64* clusters = static_cast<U64*>(m_r->getStagingGpuMemoryManager().allocateFrame(
		sizeof(Cluster) * m_clusterCount, StagingGpuMemoryType::STORAGE, ctx.m_clustererGpuObjects.m_clusterersToken));

	// Zero the memory because atomics will happen
	memset(clusters, 0, sizeof(Cluster) * m_clusterCount);
}

void ClusterBinning::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const ClustererGpuObjects& tokens = m_runCtx.m_ctx->m_clustererGpuObjects;

	cmdb->bindShaderProgram(m_grProg);
	bindUniforms(cmdb, 0, 0, tokens.m_lightingUniformsToken);
	bindStorage(cmdb, 0, 1, tokens.m_clusterersToken);
	bindStorage(cmdb, 0, 2, tokens.m_pointLightsToken);

	const U32 sampleCount = 8;
	const U32 sizex = m_clusterCount * sampleCount;
	const RenderQueue& rqueue = *m_runCtx.m_ctx->m_renderQueue;
	U32 clusterObjectCounts = min(MAX_VISIBLE_POINT_LIGHTS, rqueue.m_pointLights.getSize());
	cmdb->dispatchCompute((sizex - 64 - 1) / 64, 1, clusterObjectCounts);
}

} // end namespace anki
