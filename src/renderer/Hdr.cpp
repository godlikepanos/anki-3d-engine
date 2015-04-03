// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Hdr.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Hdr::~Hdr()
{}

//==============================================================================
Error Hdr::initFb(FramebufferHandle& fb, TextureHandle& rt)
{
	ANKI_CHECK(m_r->createRenderTarget(m_width, m_height, GL_RGB8, 1, rt));

	// Set to bilinear because the blurring techniques take advantage of that
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&getGrManager()));

	rt.setFilter(cmdb, TextureHandle::Filter::LINEAR);

	// Create FB
	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = rt;
	ANKI_CHECK(fb.create(cmdb, fbInit));

	cmdb.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Hdr::initInternal(const ConfigSet& initializer)
{
	m_enabled = initializer.get("pps.hdr.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	const F32 renderingQuality = initializer.get("pps.hdr.renderingQuality");

	m_width = renderingQuality * (F32)m_r->getWidth();
	alignRoundDown(16, m_width);
	m_height = renderingQuality * (F32)m_r->getHeight();
	alignRoundDown(16, m_height);

	m_exposure = initializer.get("pps.hdr.exposure");
	m_blurringDist = initializer.get("pps.hdr.blurringDist");
	m_blurringIterationsCount = 
		initializer.get("pps.hdr.blurringIterationsCount");

	ANKI_CHECK(initFb(m_hblurFb, m_hblurRt));
	ANKI_CHECK(initFb(m_vblurFb, m_vblurRt));

	// init shaders
	GrManager& gl = getGrManager();
	CommandBufferHandle cmdb;
	ANKI_CHECK(cmdb.create(&gl));

	ANKI_CHECK(m_commonBuff.create(cmdb, GL_UNIFORM_BUFFER, nullptr,
		sizeof(Vec4), GL_DYNAMIC_STORAGE_BIT));

	updateDefaultBlock(cmdb);

	cmdb.flush();

	ANKI_CHECK(
		m_toneFrag.load("shaders/PpsHdr.frag.glsl", &getResourceManager()));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_toneFrag->getGrShader(), m_tonePpline));

	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	StringAuto pps(getAllocator());
	pps.sprintf(
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		m_blurringDist, m_height, 
		static_cast<U>(initializer.get("pps.hdr.samples")));

	ANKI_CHECK(m_hblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_hblurFrag->getGrShader(), m_hblurPpline));

	pps.destroy(getAllocator());
	pps.sprintf(
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		m_blurringDist, m_width, 
		static_cast<U>(initializer.get("pps.hdr.samples")));

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
Error Hdr::init(const ConfigSet& initializer)
{
	Error err = initInternal(initializer);

	if(err)
	{
		ANKI_LOGE("Failed to init PPS HDR");
	}

	return err;
}

//==============================================================================
Error Hdr::run(CommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);

	// For the passes it should be NEAREST
	//vblurFai.setFiltering(Texture::TFrustumType::NEAREST);

	// pass 0
	m_vblurFb.bind(cmdb, true);
	cmdb.setViewport(0, 0, m_width, m_height);
	m_tonePpline.bind(cmdb);

	if(m_parameterUpdateTimestamp > m_commonUboUpdateTimestamp)
	{
		updateDefaultBlock(cmdb);

		m_commonUboUpdateTimestamp = getGlobalTimestamp();
	}

	m_r->getIs()._getRt().bind(cmdb, 0);
	m_commonBuff.bindShaderBuffer(cmdb, 0);

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
		m_hblurFb.bind(cmdb, true);
		m_hblurPpline.bind(cmdb);		
		m_r->drawQuad(cmdb);

		// vpass
		m_vblurFb.bind(cmdb, true);
		m_vblurPpline.bind(cmdb);
		m_r->drawQuad(cmdb);
	}

	// For the next stage it should be LINEAR though
	//vblurFai.setFiltering(Texture::TFrustumType::LINEAR);

	return ErrorCode::NONE;
}

//==============================================================================
void Hdr::updateDefaultBlock(CommandBufferHandle& cmdb)
{
	Vec4 uniform(m_exposure, 0.0, 0.0, 0.0);
	m_commonBuff.write(cmdb, &uniform, sizeof(uniform), 0, 0, sizeof(uniform));
}

} // end namespace anki
