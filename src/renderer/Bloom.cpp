// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Bloom.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Tm.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Bloom::~Bloom()
{}

//==============================================================================
Error Bloom::initFb(FramebufferHandle& fb, TextureHandle& rt)
{
	// Set to bilinear because the blurring techniques take advantage of that
	ANKI_CHECK(m_r->createRenderTarget(m_width, m_height, 
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM), 
		1, SamplingFilter::LINEAR, 1, rt));

	// Create FB
	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = rt;
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	ANKI_CHECK(fb.create(&getGrManager(), fbInit));

	return ErrorCode::NONE;
}

//==============================================================================
Error Bloom::initInternal(const ConfigSet& initializer)
{
	m_enabled = initializer.get("pps.bloom.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	const F32 renderingQuality = initializer.get("pps.bloom.renderingQuality");

	m_width = renderingQuality * (F32)m_r->getWidth();
	alignRoundDown(16, m_width);
	m_height = renderingQuality * (F32)m_r->getHeight();
	alignRoundDown(16, m_height);

	m_threshold = initializer.get("pps.bloom.threshold");
	m_blurringDist = initializer.get("pps.bloom.blurringDist");
	m_blurringIterationsCount = 
		initializer.get("pps.bloom.blurringIterationsCount");

	ANKI_CHECK(initFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(initFb(m_vblurFb, m_vblurRt));

	// init shaders
	GrManager& gl = getGrManager();

	ANKI_CHECK(m_commonBuff.create(&gl, GL_UNIFORM_BUFFER, nullptr,
		sizeof(Vec4), GL_DYNAMIC_STORAGE_BIT));

	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));
	updateDefaultBlock(cmdb);

	cmdb.flush();

	ANKI_CHECK(m_toneFrag.loadToCache(&getResourceManager(),
		"shaders/PpsBloom.frag.glsl", 
		m_r->_getShadersPrependedSource().toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_toneFrag->getGrShader(), m_tonePpline));

	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	StringAuto pps(getAllocator());
	pps.sprintf(
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(1.1)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 17\n",
		m_height);

	ANKI_CHECK(m_hblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_hblurFrag->getGrShader(), m_hblurPpline));

	pps.destroy(getAllocator());
	pps.sprintf(
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(1.0)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES 15\n",
		m_width);

	ANKI_CHECK(m_vblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_vblurFrag->getGrShader(), m_vblurPpline));

	// Set timestamps
	m_parameterUpdateTimestamp = getGlobalTimestamp();
	m_commonUboUpdateTimestamp = getGlobalTimestamp();

	return ErrorCode::NONE;
}

//==============================================================================
Error Bloom::init(const ConfigSet& initializer)
{
	Error err = initInternal(initializer);
	if(err)
	{
		ANKI_LOGE("Failed to init PPS bloom");
	}

	return err;
}

//==============================================================================
Error Bloom::run(CommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);

	// For the passes it should be NEAREST_BASE
	//vblurFai.setFiltering(Texture::TFrustumType::NEAREST_BASE);

	// pass 0
	m_vblurFb.bind(cmdb);
	cmdb.setViewport(0, 0, m_width, m_height);
	m_tonePpline.bind(cmdb);

	if(m_parameterUpdateTimestamp > m_commonUboUpdateTimestamp)
	{
		updateDefaultBlock(cmdb);

		m_commonUboUpdateTimestamp = getGlobalTimestamp();
	}

	m_r->getIs()._getRt().bind(cmdb, 0);
	m_commonBuff.bindShaderBuffer(cmdb, 0);
	m_r->getPps().getTm().getAverageLuminanceBuffer().bindShaderBuffer(cmdb, 0);

	m_r->drawQuad(cmdb);

	// Blurring passes
	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		if(i == 0)
		{
			Array<TextureHandle, 2> arr = {{m_hblurRt, m_vblurRt}};
			cmdb.bindTextures(0, arr.begin(), arr.getSize());
		}

		// hpass
		m_hblurFb.bind(cmdb);
		m_hblurPpline.bind(cmdb);		
		m_r->drawQuad(cmdb);

		// vpass
		m_vblurFb.bind(cmdb);
		m_vblurPpline.bind(cmdb);
		m_r->drawQuad(cmdb);
	}

	// For the next stage it should be LINEAR though
	//vblurFai.setFiltering(Texture::TFrustumType::LINEAR);

	return ErrorCode::NONE;
}

//==============================================================================
void Bloom::updateDefaultBlock(CommandBufferHandle& cmdb)
{
	Vec4 uniform(m_threshold, 0.0, 0.0, 0.0);
	m_commonBuff.write(cmdb, &uniform, sizeof(uniform), 0, 0, sizeof(uniform));
}

} // end namespace anki
