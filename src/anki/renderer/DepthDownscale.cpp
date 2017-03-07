// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	m_depthRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(width,
		height,
		MS_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR));

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	ANKI_CHECK(getResourceManager().loadResource("shaders/DepthDownscaleHalf.frag.glsl", m_frag));
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

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
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_r->getMs().m_depthRt);

	cmdb->setViewport(0, 0, m_r->getWidth() / 2, m_r->getHeight() / 2);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();

	// Restore state
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
}

QuarterDepth::~QuarterDepth()
{
}

Error QuarterDepth::init(const ConfigSet&)
{
	GrManager& gr = getGrManager();
	U width = m_r->getWidth() / 4;
	U height = m_r->getHeight() / 4;

	m_depthRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(width,
		height,
		PixelFormat(ComponentFormat::R32, TransformFormat::FLOAT),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR));

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachments[0].m_texture = m_depthRt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachmentCount = 1;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	ANKI_CHECK(getResourceManager().loadResource("shaders/DepthDownscaleQuarter.frag.glsl", m_frag));
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

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
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void QuarterDepth::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_parent->m_hd.m_depthRt);

	cmdb->setViewport(0, 0, m_r->getWidth() / 4, m_r->getHeight() / 4);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();
}

DepthDownscale::~DepthDownscale()
{
}

Error DepthDownscale::initInternal(const ConfigSet& cfg)
{
	ANKI_CHECK(m_hd.init(cfg));
	ANKI_CHECK(m_qd.init(cfg));
	return ErrorCode::NONE;
}

Error DepthDownscale::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing depth downscale passes");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize depth downscale passes");
	}

	return err;
}

} // end namespace anki
