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
	const F32 renderingQuality = config.getNumber("bloom.renderingQuality");

	m_width = renderingQuality * F32(m_r->getWidth());
	alignRoundDown(16, m_width);
	m_height = renderingQuality * F32(m_r->getHeight());
	alignRoundDown(16, m_height);

	m_threshold = config.getNumber("bloom.threshold");
	m_scale = config.getNumber("bloom.scale");
	m_blurringDist = config.getNumber("bloom.blurringDist");
	m_blurringIterationsCount =
		config.getNumber("bloom.blurringIterationsCount");

	ANKI_CHECK(initFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(initFb(m_vblurFb, m_vblurRt));

	// init shaders & pplines
	GrManager& gl = getGrManager();

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = RT_PIXEL_FORMAT;

	StringAuto pps(getAllocator());
	pps.sprintf("#define ANKI_RENDERER_WIDTH %u\n"
				"#define ANKI_RENDERER_HEIGHT %u\n",
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_toneFrag, "shaders/Bloom.frag.glsl", pps.toCString(), "r_"));

	m_r->createDrawQuadPipeline(
		m_toneFrag->getGrShader(), colorState, m_tonePpline);

	const char* SHADER_FILENAME =
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	pps.destroy();
	pps.sprintf("#define HPASS\n"
				"#define COL_RGB\n"
				"#define BLURRING_DIST float(1.1)\n"
				"#define IMG_DIMENSION %u\n"
				"#define SAMPLES 17\n",
		m_height);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_hblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	m_r->createDrawQuadPipeline(
		m_hblurFrag->getGrShader(), colorState, m_hblurPpline);

	pps.destroy();
	pps.sprintf("#define VPASS\n"
				"#define COL_RGB\n"
				"#define BLURRING_DIST float(1.0)\n"
				"#define IMG_DIMENSION %u\n"
				"#define SAMPLES 15\n",
		m_width);

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_vblurFrag, SHADER_FILENAME, pps.toCString(), "r_"));

	m_r->createDrawQuadPipeline(
		m_vblurFrag->getGrShader(), colorState, m_vblurPpline);

	// Set descriptors
	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_r->getIs().getRt();
		descInit.m_uniformBuffers[0].m_dynamic = true;

		descInit.m_storageBuffers[0].m_buffer =
			m_r->getTm().getAverageLuminanceBuffer();

		m_firstDescrGroup = gl.newInstance<ResourceGroup>(descInit);
	}

	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_vblurRt;
		m_hDescrGroup = gl.newInstance<ResourceGroup>(descInit);
	}

	{
		ResourceGroupInitInfo descInit;
		descInit.m_textures[0].m_texture = m_hblurRt;
		m_vDescrGroup = gl.newInstance<ResourceGroup>(descInit);
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
void Bloom::run(CommandBufferPtr& cmdb)
{
	// For the passes it should be NEAREST_BASE
	// vblurFai.setFiltering(Texture::TFrustumType::NEAREST_BASE);

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
