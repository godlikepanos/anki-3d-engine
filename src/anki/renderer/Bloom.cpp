// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Bloom.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Tm.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

BloomExposure::~BloomExposure()
{
}

Error BloomExposure::init(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_width = m_r->getDownscaleBlur().getSmallPassWidth() * 2;
	m_height = m_r->getDownscaleBlur().getSmallPassHeight() * 2;

	m_threshold = config.getNumber("bloom.threshold");
	m_scale = config.getNumber("bloom.scale");

	// Create RT
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_width,
		m_height,
		BLOOM_RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR));

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// init shaders
	ANKI_CHECK(m_r->createShaderf("shaders/Bloom.frag.glsl",
		m_frag,
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n",
		m_r->getDownscaleBlur().getSmallPassWidth(),
		m_r->getDownscaleBlur().getSmallPassHeight()));

	// Init prog
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	return ErrorCode::NONE;
}

void BloomExposure::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
}

void BloomExposure::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void BloomExposure::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_r->getDownscaleBlur().getSmallPassTexture());

	Vec4* uniforms = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	*uniforms = Vec4(m_threshold, m_scale, 0.0, 0.0);

	cmdb->bindStorageBuffer(0, 0, m_r->getTm().m_luminanceBuff, 0, MAX_PTR_SIZE);

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

BloomUpscale::~BloomUpscale()
{
}

Error BloomUpscale::init(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_width = m_r->getWidth() / BLOOM_FRACTION;
	m_height = m_r->getHeight() / BLOOM_FRACTION;

	// Create RTs
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_width,
		m_height,
		BLOOM_RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR));

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// init shaders
	ANKI_CHECK(m_r->createShaderf("shaders/BloomUpscale.frag.glsl",
		m_frag,
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n",
		m_width,
		m_height));

	// Init prog
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	return ErrorCode::NONE;
}

void BloomUpscale::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
}

void BloomUpscale::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void BloomUpscale::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_r->getBloom().m_extractExposure.m_rt);
	m_r->drawQuad(cmdb);

	m_r->getBloom().m_sslf.run(ctx);
	cmdb->endRenderPass();
}

} // end namespace anki
