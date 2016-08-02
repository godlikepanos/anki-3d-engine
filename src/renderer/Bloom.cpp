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

//==============================================================================
const PixelFormat Bloom::RT_PIXEL_FORMAT(
	ComponentFormat::R8G8B8, TransformFormat::UNORM);

//==============================================================================
Bloom::~Bloom()
{
}

//==============================================================================
Error Bloom::initInternal(const ConfigSet& config)
{
	GrManager& gr = getGrManager();

	m_upscale.m_width = m_r->getWidth() / BLOOM_FRACTION;
	m_upscale.m_height = m_r->getHeight() / BLOOM_FRACTION;

	m_extractExposure.m_width =
		m_r->getWidth() >> (m_r->getIs().getRtMipmapCount() - 2);
	m_extractExposure.m_height =
		m_r->getHeight() >> (m_r->getIs().getRtMipmapCount() - 2);

	m_threshold = config.getNumber("bloom.threshold");
	m_scale = config.getNumber("bloom.scale");

	// Create RTs
	m_r->createRenderTarget(m_extractExposure.m_width,
		m_extractExposure.m_height,
		RT_PIXEL_FORMAT,
		1,
		SamplingFilter::LINEAR,
		1,
		m_extractExposure.m_rt);

	m_r->createRenderTarget(m_upscale.m_width,
		m_upscale.m_height,
		RT_PIXEL_FORMAT,
		1,
		SamplingFilter::LINEAR,
		1,
		m_upscale.m_rt);

	// Create FBs
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_extractExposure.m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_extractExposure.m_fb = gr.newInstance<Framebuffer>(fbInit);

	fbInit.m_colorAttachments[0].m_texture = m_upscale.m_rt;
	m_upscale.m_fb = gr.newInstance<Framebuffer>(fbInit);

	// init shaders
	StringAuto pps(getAllocator());
	pps.sprintf("#define WIDTH %u\n"
				"#define HEIGHT %u\n"
				"#define MIPMAP %u.0\n",
		m_r->getWidth() >> (m_r->getIs().getRtMipmapCount() - 1),
		m_r->getHeight() >> (m_r->getIs().getRtMipmapCount() - 1),
		m_r->getIs().getRtMipmapCount() - 1);

	ANKI_CHECK(
		getResourceManager().loadResourceToCache(m_extractExposure.m_frag,
			"shaders/Bloom.frag.glsl",
			pps.toCString(),
			"r_"));

	pps.destroy();
	pps.sprintf("#define WIDTH %u\n"
				"#define HEIGHT %u\n",
		m_extractExposure.m_width,
		m_extractExposure.m_height);

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_upscale.m_frag,
		"shaders/BloomUpscale.frag.glsl",
		pps.toCString(),
		"r_"));

	// Init pplines
	ColorStateInfo colorInf;
	colorInf.m_attachmentCount = 1;
	colorInf.m_attachments[0].m_format = RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(m_extractExposure.m_frag->getGrShader(),
		colorInf,
		m_extractExposure.m_ppline);
	m_r->createDrawQuadPipeline(
		m_upscale.m_frag->getGrShader(), colorInf, m_upscale.m_ppline);

	// Set descriptors
	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_r->getIs().getRt();
		descInit.m_uniformBuffers[0].m_uploadedMemory = true;

		descInit.m_storageBuffers[0].m_buffer =
			m_r->getTm().getAverageLuminanceBuffer();

		m_extractExposure.m_rsrc = gr.newInstance<ResourceGroup>(descInit);
	}

	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_extractExposure.m_rt;
		m_upscale.m_rsrc = gr.newInstance<ResourceGroup>(descInit);
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Bloom::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init PPS bloom");
	}

	return err;
}

//==============================================================================
void Bloom::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// pass 0
	cmdb->beginRenderPass(m_extractExposure.m_fb);
	cmdb->setViewport(
		0, 0, m_extractExposure.m_width, m_extractExposure.m_height);
	cmdb->bindPipeline(m_extractExposure.m_ppline);

	TransientMemoryInfo dyn;
	Vec4* uniforms = static_cast<Vec4*>(
		getGrManager().allocateFrameTransientMemory(sizeof(Vec4),
			BufferUsageBit::UNIFORM_ANY_SHADER,
			dyn.m_uniformBuffers[0]));
	*uniforms = Vec4(m_threshold, m_scale, 0.0, 0.0);

	cmdb->bindResourceGroup(m_extractExposure.m_rsrc, 0, &dyn);

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// pass 1
	cmdb->setViewport(0, 0, m_upscale.m_width, m_upscale.m_height);
	cmdb->beginRenderPass(m_upscale.m_fb);
	cmdb->bindPipeline(m_upscale.m_ppline);
	cmdb->bindResourceGroup(m_upscale.m_rsrc, 0, nullptr);
	m_r->drawQuad(cmdb);

	if(!m_r->getSslfEnabled())
	{
		cmdb->endRenderPass();
	}
}

} // end namespace anki
