// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/util/Logger.h"
#include "anki/misc/ConfigSet.h"
#include "anki/resource/ImageLoader.h"

namespace anki {

//==============================================================================
Pps::Pps(Renderer* r)
:	RenderingPass(r), 
	m_hdr(r), 
	m_ssao(r), 
	m_bl(r), 
	m_lf(r),
	m_sslr(r)
{}

//==============================================================================
Pps::~Pps()
{}

//==============================================================================
Error Pps::initInternal(const ConfigSet& config)
{
	m_enabled = config.get("pps.enabled");
	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	ANKI_ASSERT("Initializing PPS");

	ANKI_CHECK(m_ssao.init(config));
	ANKI_CHECK(m_hdr.init(config));
	ANKI_CHECK(m_lf.init(config));
	ANKI_CHECK(m_sslr.init(config));

	// FBO
	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager()));

	ANKI_CHECK(
		m_r->createRenderTarget(
		m_r->getWidth(), m_r->getHeight(), 
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM), 
		1, SamplingFilter::LINEAR, 1, m_rt));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	ANKI_CHECK(m_fb.create(cmdBuff, fbInit));

	// SProg
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define SSAO_ENABLED %u\n"
		"#define HDR_ENABLED %u\n"
		"#define SHARPEN_ENABLED %u\n"
		"#define GAMMA_CORRECTION_ENABLED %u\n"
		"#define FBO_WIDTH %u\n"
		"#define FBO_HEIGHT %u\n",
		m_ssao.getEnabled(), 
		m_hdr.getEnabled(), 
		static_cast<U>(config.get("pps.sharpen")),
		static_cast<U>(config.get("pps.gammaCorrection")),
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(m_frag.loadToCache(&getResourceManager(),
		"shaders/Pps.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(m_frag->getGrShader(), m_ppline));

	// LUT
	ANKI_CHECK(loadColorGradingTexture("engine_data/default_lut.ankitex"));

	cmdBuff.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Pps::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init PPS");
	}

	return err;
}
//==============================================================================
Error Pps::loadColorGradingTexture(CString filename)
{
	/*StringAuto lutFname(getAllocator());
	m_r->_getResourceManager().fixResourceFilename(
		"engine_data/default_lut.ankitex", lutFname);

	ImageLoader loader(getAllocator());
	ANKI_CHECK(loader.load(lutFname.toCString(), MAX_U32, true));*/

	return ErrorCode::NONE;
}

//==============================================================================
Error Pps::run(CommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	// First SSAO because it depends on MS where HDR depends on IS
	if(m_ssao.getEnabled())
	{
		ANKI_CHECK(m_ssao.run(cmdBuff));
	}

	// Then SSLR because HDR depends on it
	if(m_sslr.getEnabled())
	{
		ANKI_CHECK(m_sslr.run(cmdBuff));
	}

	if(m_hdr.getEnabled())
	{
		ANKI_CHECK(m_hdr.run(cmdBuff));
	}

	if(m_lf.getEnabled())
	{
		ANKI_CHECK(m_lf.run(cmdBuff));
	}

	Bool drawToDefaultFbo = 
		!m_r->getDbg().getEnabled() 
		&& !m_r->getIsOffscreen()
		&& m_r->getRenderingQuality() == 1.0;

	if(drawToDefaultFbo)
	{
		m_r->getDefaultFramebuffer().bind(cmdBuff);
		cmdBuff.setViewport(0, 0, 
			m_r->getDefaultFramebufferWidth(), 
			m_r->getDefaultFramebufferHeight());
	}
	else
	{
		m_fb.bind(cmdBuff);
		cmdBuff.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}

	m_ppline.bind(cmdBuff);

	m_r->getIs()._getRt().bind(cmdBuff, 0);

	if(m_ssao.getEnabled())
	{
		m_ssao._getRt().bind(cmdBuff, 1);
	}

	if(m_lf.getEnabled())
	{
		m_lf._getRt().bind(cmdBuff, 2);
	}
	else if(m_hdr.getEnabled())
	{
		m_hdr._getRt().bind(cmdBuff, 2);
	}

	m_r->drawQuad(cmdBuff);

	return ErrorCode::NONE;
}

} // end namespace anki
