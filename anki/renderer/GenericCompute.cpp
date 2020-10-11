// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GenericCompute.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

GenericCompute::~GenericCompute()
{
}

void GenericCompute::populateRenderGraph(RenderingContext& ctx)
{
	if(ctx.m_renderQueue->m_genericGpuComputeJobs.getSize() == 0)
	{
		return;
	}

	m_runCtx.m_ctx = &ctx;

	ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Generic compute");

	pass.setWork(
		[](RenderPassWorkContext& rgraphCtx) {
			GenericCompute* const self = static_cast<GenericCompute*>(rgraphCtx.m_userData);
			self->run(rgraphCtx);
		},
		this, 0);

	pass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

void GenericCompute::run(RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_runCtx.m_ctx->m_renderQueue->m_genericGpuComputeJobs.getSize() > 0);

	GenericGpuComputeJobQueueElementContext elementCtx;
	elementCtx.m_commandBuffer = rgraphCtx.m_commandBuffer;
	elementCtx.m_stagingGpuAllocator = &m_r->getStagingGpuMemoryManager();
	elementCtx.m_viewMatrix = m_runCtx.m_ctx->m_matrices.m_view;
	elementCtx.m_viewProjectionMatrix = m_runCtx.m_ctx->m_matrices.m_viewProjection;
	elementCtx.m_projectionMatrix = m_runCtx.m_ctx->m_matrices.m_projection;
	elementCtx.m_previousViewProjectionMatrix = m_runCtx.m_ctx->m_prevMatrices.m_viewProjection;
	elementCtx.m_cameraTransform = m_runCtx.m_ctx->m_matrices.m_cameraTransform;

	// Bind some state
	rgraphCtx.bindTexture(0, 0, m_r->getDepthDownscale().getHiZRt(), TextureSubresourceInfo());

	for(const GenericGpuComputeJobQueueElement& element : m_runCtx.m_ctx->m_renderQueue->m_genericGpuComputeJobs)
	{
		ANKI_ASSERT(element.m_callback);
		element.m_callback(elementCtx, element.m_userData);
	}
}

} // end namespace anki