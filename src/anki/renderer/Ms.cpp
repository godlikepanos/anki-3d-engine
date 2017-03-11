// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ms.h>
#include <anki/renderer/Renderer.h>
#include <anki/util/Logger.h>
#include <anki/util/ThreadPool.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/misc/ConfigSet.h>
#include <anki/core/Trace.h>

namespace anki
{

Ms::~Ms()
{
}

Error Ms::createRt()
{
	m_depthRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE
			| TextureUsageBit::GENERATE_MIPMAPS,
		SamplingFilter::NEAREST));

	m_rt0 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[0],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::NEAREST));

	m_rt1 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[1],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::NEAREST));

	m_rt2 = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
		m_r->getHeight(),
		MS_COLOR_ATTACHMENT_PIXEL_FORMATS[2],
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::GENERATE_MIPMAPS,
		SamplingFilter::NEAREST));

	AttachmentLoadOperation loadop = AttachmentLoadOperation::DONT_CARE;
#if ANKI_EXTRA_CHECKS
	loadop = AttachmentLoadOperation::CLEAR;
#endif

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = MS_COLOR_ATTACHMENT_COUNT;
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

Error Ms::init(const ConfigSet& initializer)
{
	ANKI_R_LOGI("Initializing g-buffer pass");

	Error err = initInternal(initializer);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize g-buffer pass");
	}

	return err;
}

Error Ms::initInternal(const ConfigSet& initializer)
{
	ANKI_CHECK(createRt());
	return ErrorCode::NONE;
}

Error Ms::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	// Get some stuff
	const VisibilityTestResults& vis = *ctx.m_visResults;

	U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start != end)
	{
		// Create the command buffer
		CommandBufferInitInfo cinf;
		cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
		cinf.m_framebuffer = m_fb;
		CommandBufferPtr cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);
		ctx.m_ms.m_commandBuffers[threadId] = cmdb;

		// Inform on RTs
		TextureSurfaceInfo surf(0, 0, 0, 0);
		cmdb->informTextureSurfaceCurrentUsage(m_rt0, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_rt1, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_rt2, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE);
		cmdb->informTextureSurfaceCurrentUsage(m_depthRt, surf, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);

		// Set some state, leave the rest to default
		cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

		// Start drawing
		ANKI_CHECK(m_r->getSceneDrawer().drawRange(Pass::MS_FS,
			ctx.m_viewMat,
			ctx.m_viewProjMat,
			cmdb,
			vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
			vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end));
	}

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
	return ErrorCode::NONE;
}

void Ms::run(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);

	// Set some state anyway because other stages may depend on it
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_ms.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_ms.m_commandBuffers[i]);
		}
	}

	cmdb->endRenderPass();

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

void Ms::setPreRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(m_rt0, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(m_rt1, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(m_rt2, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, surf);
	cmdb->setTextureSurfaceBarrier(
		m_depthRt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, surf);

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

void Ms::setPostRunBarriers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	TextureSurfaceInfo surf(0, 0, 0, 0);

	cmdb->setTextureSurfaceBarrier(
		m_rt0, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_rt1, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(
		m_rt2, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->setTextureSurfaceBarrier(m_depthRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
		surf);

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

} // end namespace anki
