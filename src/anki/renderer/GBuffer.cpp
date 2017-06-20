// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/util/Logger.h>
#include <anki/util/ThreadPool.h>
#include <anki/misc/ConfigSet.h>
#include <anki/core/Trace.h>

namespace anki
{

GBuffer::~GBuffer()
{
}

Error GBuffer::createRt()
{
	m_depthRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE
			| TextureUsageBit::GENERATE_MIPMAPS,
		SamplingFilter::NEAREST,
		1,
		"gbuffdepth"));

	m_rt0 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[0],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::NEAREST,
		1,
		"gbuffrt0"));

	m_rt1 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[1],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::NEAREST,
		1,
		"gbuffrt1"));

	m_rt2 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[2],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::GENERATE_MIPMAPS,
		SamplingFilter::NEAREST,
		1,
		"gbuffrt2"));

	AttachmentLoadOperation loadop = AttachmentLoadOperation::DONT_CARE;
#if ANKI_EXTRA_CHECKS
	loadop = AttachmentLoadOperation::CLEAR;
#endif

	FramebufferInitInfo fbInit("gbuffer");
	fbInit.m_colorAttachmentCount = GBUFFER_COLOR_ATTACHMENT_COUNT;
	fbInit.m_colorAttachments[0].m_texture = m_rt0;
	fbInit.m_colorAttachments[0].m_loadOperation = loadop;
	fbInit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[1].m_texture = m_rt1;
	fbInit.m_colorAttachments[1].m_loadOperation = loadop;
	fbInit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0, 1.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[2].m_texture = m_rt2;
	fbInit.m_colorAttachments[2].m_loadOperation = loadop;
	fbInit.m_colorAttachments[2].m_clearValue.m_colorf = {{0.0, 0.0, 1.0, 0.0}};
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
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
	ANKI_CHECK(createRt());
	return ErrorCode::NONE;
}

void GBuffer::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_MS);

	// Get some stuff
	const PtrSize earlyZCount = ctx.m_renderQueue->m_earlyZRenderables.getSize();
	const U problemSize = ctx.m_renderQueue->m_renderables.getSize() + earlyZCount;
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start != end)
	{
		// Create the command buffer
		CommandBufferInitInfo cinf;
		cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
		if(end - start < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
		{
			cinf.m_flags |= CommandBufferFlag::SMALL_BATCH;
		}
		cinf.m_framebuffer = m_fb;
		CommandBufferPtr cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);
		ctx.m_gbuffer.m_commandBuffers[threadId] = cmdb;

		// Inform on RTs
		TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->informTextureSurfaceCurrentUsage(m_rt0, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_rt1, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_rt2, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_depthRt, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

		// Set some state, leave the rest to default
		cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

		const I32 earlyZStart = max(I32(start), 0);
		const I32 earlyZEnd = min(I32(end), I32(earlyZCount));
		const I32 colorStart = max(I32(start) - I32(earlyZCount), 0);
		const I32 colorEnd = I32(end) - I32(earlyZCount);

		// First do early Z (if needed)
		if(earlyZStart < earlyZEnd)
		{
			for(U i = 0; i < GBUFFER_COLOR_ATTACHMENT_COUNT; ++i)
			{
				cmdb->setColorChannelWriteMask(i, ColorBit::NONE);
			}

			ANKI_ASSERT(earlyZStart < earlyZEnd && earlyZEnd <= I32(earlyZCount));
			m_r->getSceneDrawer().drawRange(Pass::SM,
				ctx.m_renderQueue->m_viewMatrix,
				ctx.m_viewProjMatJitter,
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
			m_r->getSceneDrawer().drawRange(Pass::GB_FS,
				ctx.m_renderQueue->m_viewMatrix,
				ctx.m_viewProjMatJitter,
				cmdb,
				ctx.m_renderQueue->m_renderables.getBegin() + colorStart,
				ctx.m_renderQueue->m_renderables.getBegin() + colorEnd);
		}
	}
}

void GBuffer::run(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);

	// Set some state anyway because other stages may depend on it
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_gbuffer.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_gbuffer.m_commandBuffers[i]);
		}
	}

	cmdb->endRenderPass();
}

void GBuffer::setPreRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(m_rt0, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(m_rt1, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(m_rt2, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(
		m_depthRt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, surf);
}

void GBuffer::setPostRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(
		m_rt0, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_rt1, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_rt2, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_depthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);
}

} // end namespace anki
