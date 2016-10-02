// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Bloom.h>
#include <anki/renderer/Is.h>
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

	m_width = m_r->getWidth() >> (m_r->getIs().getRtMipmapCount() - 2);
	m_height = m_r->getHeight() >> (m_r->getIs().getRtMipmapCount() - 2);

	m_threshold = config.getNumber("bloom.threshold");
	m_scale = config.getNumber("bloom.scale");

	// Create RT
	m_r->createRenderTarget(m_width,
		m_height,
		BLOOM_RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// init shaders
	StringAuto pps(getAllocator());
	pps.sprintf("#define WIDTH %u\n"
				"#define HEIGHT %u\n"
				"#define MIPMAP %u.0\n",
		m_r->getWidth() >> (m_r->getIs().getRtMipmapCount() - 1),
		m_r->getHeight() >> (m_r->getIs().getRtMipmapCount() - 1),
		m_r->getIs().getRtMipmapCount() - 1);

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_frag, "shaders/Bloom.frag.glsl", pps.toCString(), "r_"));

	// Init pplines
	ColorStateInfo colorInf;
	colorInf.m_attachmentCount = 1;
	colorInf.m_attachments[0].m_format = BLOOM_RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorInf, m_ppline);

	// Set descriptors
	ResourceGroupInitInfo descInit;
	descInit.m_textures[0].m_texture = m_r->getIs().getRt();
	descInit.m_uniformBuffers[0].m_uploadedMemory = true;
	descInit.m_uniformBuffers[0].m_usage = BufferUsageBit::UNIFORM_FRAGMENT;
	descInit.m_storageBuffers[0].m_buffer = m_r->getTm().getAverageLuminanceBuffer();
	descInit.m_storageBuffers[0].m_usage = BufferUsageBit::STORAGE_FRAGMENT_READ;

	m_rsrc = gr.newInstance<ResourceGroup>(descInit);

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
	cmdb->bindPipeline(m_ppline);

	TransientMemoryInfo dyn;
	Vec4* uniforms = static_cast<Vec4*>(getGrManager().allocateFrameTransientMemory(
		sizeof(Vec4), BufferUsageBit::UNIFORM_ALL, dyn.m_uniformBuffers[0]));
	*uniforms = Vec4(m_threshold, m_scale, 0.0, 0.0);

	cmdb->bindResourceGroup(m_rsrc, 0, &dyn);

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
	m_r->createRenderTarget(m_width,
		m_height,
		BLOOM_RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1,
		m_rt);

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachments[0].m_usageInsideRenderPass = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// init shaders
	StringAuto pps(getAllocator());
	pps.sprintf("#define WIDTH %u\n"
				"#define HEIGHT %u\n",
		m_width,
		m_height);

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_frag, "shaders/BloomUpscale.frag.glsl", pps.toCString(), "r_"));

	// Init pplines
	ColorStateInfo colorInf;
	colorInf.m_attachmentCount = 1;
	colorInf.m_attachments[0].m_format = BLOOM_RT_PIXEL_FORMAT;
	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorInf, m_ppline);

	// Set descriptors
	ResourceGroupInitInfo descInit;
	descInit.m_textures[0].m_texture = m_r->getBloom().m_extractExposure.m_rt;
	m_rsrc = gr.newInstance<ResourceGroup>(descInit);

	return ErrorCode::NONE;
}

void BloomUpscale::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
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
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rsrc, 0, nullptr);
	m_r->drawQuad(cmdb);

	m_r->getBloom().m_sslf.run(ctx);
	cmdb->endRenderPass();
}

} // end namespace anki
