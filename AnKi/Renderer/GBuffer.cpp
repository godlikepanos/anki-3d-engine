// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

GBuffer::~GBuffer()
{
}

Error GBuffer::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize g-buffer pass");
	}

	return err;
}

Error GBuffer::initInternal()
{
	ANKI_R_LOGV("Initializing GBuffer. Resolution %ux%u", m_r->getInternalResolution().x(),
				m_r->getInternalResolution().y());

	// RTs
	static const Array<const char*, 2> depthRtNames = {{"GBuffer depth #0", "GBuffer depth #1"}};
	for(U32 i = 0; i < 2; ++i)
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
			m_r->getInternalResolution().x(), m_r->getInternalResolution().y(), m_r->getDepthNoStencilFormat(),
			TextureUsageBit::ALL_SAMPLED | TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, depthRtNames[i]);

		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		m_depthRts[i] = m_r->createAndClearRenderTarget(texinit);
	}

	static const Array<const char*, GBUFFER_COLOR_ATTACHMENT_COUNT> rtNames = {
		{"GBuffer rt0", "GBuffer rt1", "GBuffer rt2", "GBuffer rt3"}};
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		m_colorRtDescrs[i] =
			m_r->create2DRenderTargetDescription(m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
												 GBUFFER_COLOR_ATTACHMENT_PIXEL_FORMATS[i], rtNames[i]);
		m_colorRtDescrs[i].bake();
	}

	// FB descr
	AttachmentLoadOperation loadop = AttachmentLoadOperation::DONT_CARE;
#if ANKI_EXTRA_CHECKS
	loadop = AttachmentLoadOperation::CLEAR;
#endif

	m_fbDescr.m_colorAttachmentCount = GBUFFER_COLOR_ATTACHMENT_COUNT;
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		m_fbDescr.m_colorAttachments[i].m_loadOperation = loadop;
		m_fbDescr.m_colorAttachments[i].m_clearValue.m_colorf = {1.0f, 0.0f, 1.0f, 0.0f};
	}

	m_fbDescr.m_colorAttachments[3].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.m_colorAttachments[3].m_clearValue.m_colorf = {1.0f, 1.0f, 1.0f, 1.0f};

	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;

	if(getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs())
	{
		m_fbDescr.m_shadingRateAttachmentTexelWidth = m_r->getVrsSriGeneration().getSriTexelDimension();
		m_fbDescr.m_shadingRateAttachmentTexelHeight = m_r->getVrsSriGeneration().getSriTexelDimension();
	}

	m_fbDescr.bake();

	return Error::NONE;
}

void GBuffer::runInThread(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const
{
	ANKI_TRACE_SCOPED_EVENT(R_MS);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U32 threadId = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	const U32 threadCount = rgraphCtx.m_secondLevelCommandBufferCount;

	// Get some stuff
	const U32 earlyZCount = ctx.m_renderQueue->m_earlyZRenderables.getSize();
	const U32 problemSize = ctx.m_renderQueue->m_renderables.getSize() + earlyZCount;
	U32 start, end;
	splitThreadedProblem(threadId, threadCount, problemSize, start, end);
	ANKI_ASSERT(end != start);

	// Set some state, leave the rest to default
	cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

	const I32 earlyZStart = max(I32(start), 0);
	const I32 earlyZEnd = min(I32(end), I32(earlyZCount));
	const I32 colorStart = max(I32(start) - I32(earlyZCount), 0);
	const I32 colorEnd = I32(end) - I32(earlyZCount);

	cmdb->setRasterizationOrder(RasterizationOrder::RELAXED);

	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	if(enableVrs)
	{
		// Just set some low value, the attachment will take over
		cmdb->setVrsRate(VrsRate::_1x1);
	}

	// First do early Z (if needed)
	if(earlyZStart < earlyZEnd)
	{
		for(U32 i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			cmdb->setColorChannelWriteMask(i, ColorBit::NONE);
		}

		ANKI_ASSERT(earlyZStart < earlyZEnd && earlyZEnd <= I32(earlyZCount));
		m_r->getSceneDrawer().drawRange(Pass::EZ, ctx.m_matrices.m_view, ctx.m_matrices.m_viewProjectionJitter,
										ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection, cmdb,
										m_r->getSamplers().m_trilinearRepeatAnisoResolutionScalingBias,
										ctx.m_renderQueue->m_earlyZRenderables.getBegin() + earlyZStart,
										ctx.m_renderQueue->m_earlyZRenderables.getBegin() + earlyZEnd);

		// Restore state for the color write
		if(colorStart < colorEnd)
		{
			for(U32 i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
			{
				cmdb->setColorChannelWriteMask(i, ColorBit::ALL);
			}
		}
	}

	// Do the color writes
	if(colorStart < colorEnd)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS_EQUAL);

		ANKI_ASSERT(colorStart < colorEnd && colorEnd <= I32(ctx.m_renderQueue->m_renderables.getSize()));
		m_r->getSceneDrawer().drawRange(Pass::GB, ctx.m_matrices.m_view, ctx.m_matrices.m_viewProjectionJitter,
										ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection, cmdb,
										m_r->getSamplers().m_trilinearRepeatAnisoResolutionScalingBias,
										ctx.m_renderQueue->m_renderables.getBegin() + colorStart,
										ctx.m_renderQueue->m_renderables.getBegin() + colorEnd);
	}
}

void GBuffer::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_MS);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	const Bool fbDescrHasVrs = m_fbDescr.m_shadingRateAttachmentTexelWidth > 0;

	if(enableVrs != fbDescrHasVrs)
	{
		// Re-bake the FB descriptor if the VRS state has changed

		if(enableVrs)
		{
			m_fbDescr.m_shadingRateAttachmentTexelWidth = m_r->getVrsSriGeneration().getSriTexelDimension();
			m_fbDescr.m_shadingRateAttachmentTexelHeight = m_r->getVrsSriGeneration().getSriTexelDimension();
		}
		else
		{
			m_fbDescr.m_shadingRateAttachmentTexelWidth = 0;
			m_fbDescr.m_shadingRateAttachmentTexelHeight = 0;
		}

		m_fbDescr.bake();
	}

	// Create RTs
	Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS> rts;
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		m_runCtx.m_colorRts[i] = rgraph.newRenderTarget(m_colorRtDescrs[i]);
		rts[i] = m_runCtx.m_colorRts[i];
	}

	if(ANKI_LIKELY(m_runCtx.m_crntFrameDepthRt.isValid()))
	{
		// Already imported once
		m_runCtx.m_crntFrameDepthRt =
			rgraph.importRenderTarget(m_depthRts[m_r->getFrameCount() & 1], TextureUsageBit::NONE);
		m_runCtx.m_prevFrameDepthRt = rgraph.importRenderTarget(m_depthRts[(m_r->getFrameCount() + 1) & 1]);
	}
	else
	{
		m_runCtx.m_crntFrameDepthRt =
			rgraph.importRenderTarget(m_depthRts[m_r->getFrameCount() & 1], TextureUsageBit::NONE);
		m_runCtx.m_prevFrameDepthRt =
			rgraph.importRenderTarget(m_depthRts[(m_r->getFrameCount() + 1) & 1], TextureUsageBit::SAMPLED_FRAGMENT);
	}

	RenderTargetHandle sriRt;
	if(enableVrs)
	{
		sriRt = m_r->getVrsSriGeneration().getSriRt();
	}

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GBuffer");

	pass.setFramebufferInfo(m_fbDescr, ConstWeakArray<RenderTargetHandle>(&rts[0], GBUFFER_COLOR_ATTACHMENT_COUNT),
							m_runCtx.m_crntFrameDepthRt, sriRt);
	pass.setWork(computeNumberOfSecondLevelCommandBuffers(ctx.m_renderQueue->m_earlyZRenderables.getSize()
														  + ctx.m_renderQueue->m_renderables.getSize()),
				 [this, &ctx](RenderPassWorkContext& rgraphCtx) {
					 runInThread(ctx, rgraphCtx);
				 });

	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		pass.newDependency(RenderPassDependency(m_runCtx.m_colorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));
	}

	TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
	pass.newDependency(
		RenderPassDependency(m_runCtx.m_crntFrameDepthRt, TextureUsageBit::ALL_FRAMEBUFFER_ATTACHMENT, subresource));

	if(enableVrs)
	{
		pass.newDependency(RenderPassDependency(sriRt, TextureUsageBit::FRAMEBUFFER_SHADING_RATE));
	}
}

} // end namespace anki
