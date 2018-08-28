// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/LensFlare.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>
#include <anki/core/Trace.h>

namespace anki
{

GBuffer::~GBuffer()
{
}

Error GBuffer::init(const ConfigSet& initializer)
{
	ANKI_R_LOGI("Initializing g-buffer pass");

	Error err = initInternal(initializer);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize g-buffer pass");
	}

	return err;
}

Error GBuffer::initInternal(const ConfigSet& initializer)
{
	// RT descrs
	m_depthRtDescr = m_r->create2DRenderTargetDescription(
		m_r->getWidth(), m_r->getHeight(), GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT, "GBuffer depth");
	m_depthRtDescr.bake();

	static const Array<const char*, GBUFFER_COLOR_ATTACHMENT_COUNT> rtNames = {
		{"GBuffer rt0", "GBuffer rt1", "GBuffer rt2", "GBuffer rt3"}};
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		m_colorRtDescrs[i] = m_r->create2DRenderTargetDescription(
			m_r->getWidth(), m_r->getHeight(), MS_COLOR_ATTACHMENT_PIXEL_FORMATS[i], rtNames[i]);
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
		m_fbDescr.m_colorAttachments[i].m_clearValue.m_colorf = {{1.0, 0.0, 1.0, 0.0}};
	}
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;
	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fbDescr.bake();

	return Error::NONE;
}

void GBuffer::runInThread(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx) const
{
	ANKI_TRACE_SCOPED_EVENT(R_MS);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const U threadId = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	const U threadCount = rgraphCtx.m_secondLevelCommandBufferCount;

	// Get some stuff
	const PtrSize earlyZCount = ctx.m_renderQueue->m_earlyZRenderables.getSize();
	const U problemSize = ctx.m_renderQueue->m_renderables.getSize() + earlyZCount;
	PtrSize start, end;
	splitThreadedProblem(threadId, threadCount, problemSize, start, end);
	ANKI_ASSERT(end != start);

	// Set some state, leave the rest to default
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	const I32 earlyZStart = max(I32(start), 0);
	const I32 earlyZEnd = min(I32(end), I32(earlyZCount));
	const I32 colorStart = max(I32(start) - I32(earlyZCount), 0);
	const I32 colorEnd = I32(end) - I32(earlyZCount);

	cmdb->setRasterizationOrder(RasterizationOrder::RELAXED);

	// First do early Z (if needed)
	if(earlyZStart < earlyZEnd)
	{
		for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
		{
			cmdb->setColorChannelWriteMask(i, ColorBit::NONE);
		}

		ANKI_ASSERT(earlyZStart < earlyZEnd && earlyZEnd <= I32(earlyZCount));
		m_r->getSceneDrawer().drawRange(Pass::EZ,
			ctx.m_matrices.m_view,
			ctx.m_matrices.m_viewProjectionJitter,
			ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection,
			cmdb,
			ctx.m_renderQueue->m_earlyZRenderables.getBegin() + earlyZStart,
			ctx.m_renderQueue->m_earlyZRenderables.getBegin() + earlyZEnd);

		// Restore state for the color write
		if(colorStart < colorEnd)
		{
			for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
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
		m_r->getSceneDrawer().drawRange(Pass::GB,
			ctx.m_matrices.m_view,
			ctx.m_matrices.m_viewProjectionJitter,
			ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection,
			cmdb,
			ctx.m_renderQueue->m_renderables.getBegin() + colorStart,
			ctx.m_renderQueue->m_renderables.getBegin() + colorEnd);
	}
}

void GBuffer::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_MS);

	m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RTs
	Array<RenderTargetHandle, MAX_COLOR_ATTACHMENTS> rts;
	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		m_colorRts[i] = rgraph.newRenderTarget(m_colorRtDescrs[i]);
		rts[i] = m_colorRts[i];
	}
	m_depthRt = rgraph.newRenderTarget(m_depthRtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GBuffer");

	pass.setFramebufferInfo(m_fbDescr, rts, m_depthRt);
	pass.setWork(runCallback,
		this,
		computeNumberOfSecondLevelCommandBuffers(
			ctx.m_renderQueue->m_earlyZRenderables.getSize() + ctx.m_renderQueue->m_renderables.getSize()));

	for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
	{
		pass.newDependency({m_colorRts[i], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	TextureSubresourceInfo subresource(DepthStencilAspectBit::DEPTH);
	pass.newDependency({m_depthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
}

} // end namespace anki
