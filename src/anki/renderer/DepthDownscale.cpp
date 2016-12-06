// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>

namespace anki
{

HalfDepth::~HalfDepth()
{
}

Error HalfDepth::init(const ConfigSet&)
{
	GrManager& gr = getGrManager();
	U width = m_r->getWidth() / 2;
	U height = m_r->getHeight() / 2;

	// Create RT
	m_r->createRenderTarget(width,
		height,
		MS_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		1,
		m_depthRt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectMask::DEPTH;

	m_fb = gr.newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void HalfDepth::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_depthRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void HalfDepth::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_depthRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void HalfDepth::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_parent->m_prog);
	cmdb->bindTexture(0, 0, m_r->getMs().m_depthRt);

	cmdb->setViewport(0, 0, m_r->getWidth() / 2, m_r->getHeight() / 2);
	cmdb->setDepthCompareFunction(CompareOperation::ALWAYS);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();

	// Restore state
	cmdb->setDepthCompareFunction(CompareOperation::LESS);
}

QuarterDepth::~QuarterDepth()
{
}

Error QuarterDepth::init(const ConfigSet&)
{
	GrManager& gr = getGrManager();
	U width = m_r->getWidth() / 4;
	U height = m_r->getHeight() / 4;

	// Create RT
	m_r->createRenderTarget(width,
		height,
		MS_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		1,
		m_depthRt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectMask::DEPTH;

	m_fb = gr.newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void QuarterDepth::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_depthRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void QuarterDepth::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_depthRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void QuarterDepth::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_parent->m_prog);
	cmdb->bindTexture(0, 0, m_parent->m_hd.m_depthRt);

	cmdb->setViewport(0, 0, m_r->getWidth() / 4, m_r->getHeight() / 4);
	cmdb->setDepthCompareFunction(CompareOperation::ALWAYS);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();

	// Restore state
	cmdb->setDepthCompareFunction(CompareOperation::LESS);
}

DepthDownscale::~DepthDownscale()
{
}

Error DepthDownscale::initInternal(const ConfigSet& cfg)
{
	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/DepthDownscale.frag.glsl", m_frag));

	// Create prog
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	ANKI_CHECK(m_hd.init(cfg));
	ANKI_CHECK(m_qd.init(cfg));
	return ErrorCode::NONE;
}

Error DepthDownscale::init(const ConfigSet& cfg)
{
	ANKI_LOGI("Initializing depth downscale passes");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_LOGE("Failed to initialize depth downscale passes");
	}

	return err;
}

} // end namespace anki
