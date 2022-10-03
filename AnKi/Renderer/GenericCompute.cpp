// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GenericCompute.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

GenericCompute::~GenericCompute()
{
}

void GenericCompute::populateRenderGraph(RenderingContext& ctx)
{
	if(ctx.m_renderQueue->m_genericGpuComputeJobs.getSize() == 0)
	{
		return;
	}

	ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Generic compute");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});

	pass.newTextureDependency(m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::kSampledCompute);
}

void GenericCompute::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(ctx.m_renderQueue->m_genericGpuComputeJobs.getSize() > 0);

	GenericGpuComputeJobQueueElementContext elementCtx;
	elementCtx.m_commandBuffer = rgraphCtx.m_commandBuffer;
	elementCtx.m_stagingGpuAllocator = &m_r->getStagingGpuMemory();
	elementCtx.m_viewMatrix = ctx.m_matrices.m_view;
	elementCtx.m_viewProjectionMatrix = ctx.m_matrices.m_viewProjection;
	elementCtx.m_projectionMatrix = ctx.m_matrices.m_projection;
	elementCtx.m_previousViewProjectionMatrix = ctx.m_prevMatrices.m_viewProjection;
	elementCtx.m_cameraTransform = ctx.m_matrices.m_cameraTransform;

	// Bind some state
	rgraphCtx.bindColorTexture(0, 0, m_r->getDepthDownscale().getHiZRt());

	for(const GenericGpuComputeJobQueueElement& element : ctx.m_renderQueue->m_genericGpuComputeJobs)
	{
		ANKI_ASSERT(element.m_callback);
		element.m_callback(elementCtx, element.m_userData);
	}
}

} // end namespace anki
