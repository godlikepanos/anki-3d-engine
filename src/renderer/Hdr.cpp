// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
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
Error Hdr::initFb(GlFramebufferHandle& fb, GlTextureHandle& rt)
{
	Error err = ErrorCode::NONE;

	err = m_r->createRenderTarget(m_width, m_height, GL_RGB8, GL_RGB, 
		GL_UNSIGNED_BYTE, 1, rt);
	if(err) return err;

	// Set to bilinear because the blurring techniques take advantage of that
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&getGlDevice());
	if(err) return err;

	rt.setFilter(cmdb, GlTextureHandle::Filter::LINEAR);

	// Create FB
	err = fb.create(cmdb, {{rt, GL_COLOR_ATTACHMENT0}});
	if(err) return err;

	cmdb.finish();

	return err;
}

//==============================================================================
Error Hdr::initInternal(const ConfigSet& initializer)
{
	Error err = ErrorCode::NONE;

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

	err = initFb(m_hblurFb, m_hblurRt);
	if(err) return err;

	err = initFb(m_vblurFb, m_vblurRt);
	if(err) return err;

	// init shaders
	GlDevice& gl = getGlDevice();
	GlCommandBufferHandle cmdb;
	err = cmdb.create(&gl);
	if(err) return err;

	err = m_commonBuff.create(cmdb, GL_UNIFORM_BUFFER, 
		sizeof(Vec4), GL_DYNAMIC_STORAGE_BIT);
	if(err) return err;

	err = updateDefaultBlock(cmdb);
	if(err) return err;

	cmdb.flush();

	err = m_toneFrag.load("shaders/PpsHdr.frag.glsl", &getResourceManager());
	if(err) return err;

	err = m_r->createDrawQuadProgramPipeline(
		m_toneFrag->getGlProgram(), m_tonePpline);
	if(err) return err;

	const char* SHADER_FILENAME = 
		"shaders/VariableSamplingBlurGeneric.frag.glsl";

	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());
	err = pps.sprintf(getAllocator(),
		"#define HPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		m_blurringDist, m_height, 
		static_cast<U>(initializer.get("pps.hdr.samples")));
	if(err) return err;

	err = m_hblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadProgramPipeline(
		m_hblurFrag->getGlProgram(), m_hblurPpline);
	if(err) return err;

	pps.destroy(getAllocator());
	err = pps.sprintf(getAllocator(),
		"#define VPASS\n"
		"#define COL_RGB\n"
		"#define BLURRING_DIST float(%f)\n"
		"#define IMG_DIMENSION %u\n"
		"#define SAMPLES %u\n",
		m_blurringDist, m_width, 
		static_cast<U>(initializer.get("pps.hdr.samples")));
	if(err) return err;

	err = m_vblurFrag.loadToCache(&getResourceManager(),
		SHADER_FILENAME, pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadProgramPipeline(
		m_vblurFrag->getGlProgram(), m_vblurPpline);
	if(err) return err;

	// Set timestamps
	m_parameterUpdateTimestamp = getGlobTimestamp();
	m_commonUboUpdateTimestamp = getGlobTimestamp();

	return err;
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
Error Hdr::run(GlCommandBufferHandle& cmdb)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

	// For the passes it should be NEAREST
	//vblurFai.setFiltering(Texture::TFrustumType::NEAREST);

	// pass 0
	m_vblurFb.bind(cmdb, true);
	cmdb.setViewport(0, 0, m_width, m_height);
	m_tonePpline.bind(cmdb);

	if(m_parameterUpdateTimestamp > m_commonUboUpdateTimestamp)
	{
		err = updateDefaultBlock(cmdb);
		if(err)
		{
			return err;
		}

		m_commonUboUpdateTimestamp = getGlobTimestamp();
	}

	m_r->getIs()._getRt().bind(cmdb, 0);
	m_commonBuff.bindShaderBuffer(cmdb, 0);

	m_r->drawQuad(cmdb);

	// Blurring passes
	for(U32 i = 0; i < m_blurringIterationsCount; i++)
	{
		if(i == 0)
		{
			Array<GlTextureHandle, 2> arr = {{m_hblurRt, m_vblurRt}};
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
Error Hdr::updateDefaultBlock(GlCommandBufferHandle& cmdb)
{
	GlClientBufferHandle tempBuff;
	Error err = tempBuff.create(cmdb, sizeof(Vec4), nullptr);
	
	if(!err)
	{
		*((Vec4*)tempBuff.getBaseAddress()) = Vec4(m_exposure, 0.0, 0.0, 0.0);

		m_commonBuff.write(cmdb, tempBuff, 0, 0, tempBuff.getSize());
	}

	return err;
}

} // end namespace anki
