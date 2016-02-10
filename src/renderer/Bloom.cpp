// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
Error Bloom::initFb(FramebufferPtr& fb, TexturePtr& rt)
{
	// Set to bilinear because the blurring techniques take advantage of that
	m_r->createRenderTarget(
		m_width, m_height, RT_PIXEL_FORMAT, 1, SamplingFilter::LINEAR, 1, rt);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Bloom::initInternal(const ConfigSet& config)
{
	m_width = m_r->getWidth() / BLOOM_FRACTION;
	m_height = m_r->getHeight() / BLOOM_FRACTION;

	m_threshold = config.getNumber("bloom.threshold");
	m_scale = config.getNumber("bloom.scale");
	m_blurringDist = config.getNumber("bloom.blurringDist");
	m_blurringIterationsCount =
		config.getNumber("bloom.blurringIterationsCount");

	ANKI_CHECK(initFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(initFb(m_vblurFb, m_vblurRt));

	// init shaders & pplines
	GrManager& gr = getGrManager();

	PipelineInitInfo ppinit;
	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = RT_PIXEL_FORMAT;
	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	StringAuto pps(getAllocator());
	pps.sprintf("#define ANKI_RENDERER_WIDTH %u\n"
				"#define ANKI_RENDERER_HEIGHT %u\n"
				"#define MIPMAP %u\n",
		m_r->getWidth(),
		m_r->getHeight(),
		IS_MIPMAP_COUNT - 1);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_quadVert, "shaders/Quad.vert.glsl", pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::VERTEX] = m_quadVert->getGrShader();

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_toneFrag, "shaders/Bloom.frag.glsl", pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = m_toneFrag->getGrShader();

	m_tonePpline = gr.newInstance<Pipeline>(ppinit);

	const char* SHADER_FILENAME = "shaders/GaussianBlurGeneric.frag.glsl";

	pps.destroy();
	pps.sprintf("#define HPASS\n"
				"#define COL_RGB\n"
				"#define TEXTURE_SIZE vec2(%f, %f)\n"
				"#define KERNEL_SIZE 15\n",
		F32(m_width),
		F32(m_height));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_hblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = m_hblurFrag->getGrShader();
	m_hblurPpline = gr.newInstance<Pipeline>(ppinit);

	pps.destroy();
	pps.sprintf("#define VPASS\n"
				"#define COL_RGB\n"
				"#define TEXTURE_SIZE vec2(%f, %f)\n"
				"#define KERNEL_SIZE 15\n",
		F32(m_width),
		F32(m_height));

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_vblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	ppinit.m_shaders[ShaderType::FRAGMENT] = m_vblurFrag->getGrShader();
	m_vblurPpline = gr.newInstance<Pipeline>(ppinit);

	// Set descriptors
	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_r->getIs().getRt();
		descInit.m_uniformBuffers[0].m_dynamic = true;

		descInit.m_storageBuffers[0].m_buffer =
			m_r->getTm().getAverageLuminanceBuffer();

		m_firstDescrGroup = gr.newInstance<ResourceGroup>(descInit);
	}

	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_vblurRt;
		m_hDescrGroup = gr.newInstance<ResourceGroup>(descInit);
	}

	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_hblurRt;
		m_vDescrGroup = gr.newInstance<ResourceGroup>(descInit);
	}

	getGrManager().finish();
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
	cmdb->bindFramebuffer(m_vblurFb);
	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->bindPipeline(m_tonePpline);

	DynamicBufferInfo dyn;
	Vec4* uniforms =
		static_cast<Vec4*>(getGrManager().allocateFrameHostVisibleMemory(
			sizeof(Vec4), BufferUsage::UNIFORM, dyn.m_uniformBuffers[0]));
	*uniforms = Vec4(m_threshold, m_scale, 0.0, 0.0);

	cmdb->bindResourceGroup(m_firstDescrGroup, 0, &dyn);

	m_r->drawQuad(cmdb);

	// Blurring passes
	for(U i = 0; i < m_blurringIterationsCount; i++)
	{
		// hpass
		cmdb->bindFramebuffer(m_hblurFb);
		cmdb->bindResourceGroup(m_hDescrGroup, 0, nullptr);
		cmdb->bindPipeline(m_hblurPpline);
		m_r->drawQuad(cmdb);

		// vpass
		cmdb->bindFramebuffer(m_vblurFb);
		cmdb->bindResourceGroup(m_vDescrGroup, 0, nullptr);
		cmdb->bindPipeline(m_vblurPpline);
		m_r->drawQuad(cmdb);
	}
}

} // end namespace anki
