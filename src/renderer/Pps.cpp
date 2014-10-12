// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Ssao.h"
#include "anki/util/Logger.h"
#include "anki/misc/ConfigSet.h"

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
void Pps::initInternal(const ConfigSet& config)
{
	m_enabled = config.get("pps.enabled");
	if(!m_enabled)
	{
		return;
	}

	ANKI_ASSERT("Initializing PPS");

	m_ssao.init(config);
	m_hdr.init(config);
	m_lf.init(config);
	m_sslr.init(config);

	// FBO
	GlCommandBufferHandle cmdBuff;
	cmdBuff.create(&getGlDevice());

	m_r->createRenderTarget(m_r->getWidth(), m_r->getHeight(), GL_RGB8, GL_RGB,
		GL_UNSIGNED_BYTE, 1, m_rt);

	m_fb.create(cmdBuff, {{m_rt, GL_COLOR_ATTACHMENT0}});

	// SProg
	String pps(getAllocator());

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

	m_frag.loadToCache(&getResourceManager(),
		"shaders/Pps.frag.glsl", pps.toCString(), "r_");

	m_ppline = m_r->createDrawQuadProgramPipeline(m_frag->getGlProgram());

	cmdBuff.finish();
}

//==============================================================================
void Pps::init(const ConfigSet& config)
{
	try
	{
		initInternal(config);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to init PPS") << e;
	}
}

//==============================================================================
void Pps::run(GlCommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	// First SSAO because it depends on MS where HDR depends on IS
	if(m_ssao.getEnabled())
	{
		m_ssao.run(cmdBuff);
	}

	// Then SSLR because HDR depends on it
	if(m_sslr.getEnabled())
	{
		m_sslr.run(cmdBuff);
	}

	if(m_hdr.getEnabled())
	{
		m_hdr.run(cmdBuff);
	}

	if(m_lf.getEnabled())
	{
		m_lf.run(cmdBuff);
	}

	Bool drawToDefaultFbo = 
		!m_r->getDbg().getEnabled() 
		&& !m_r->getIsOffscreen()
		&& m_r->getRenderingQuality() == 1.0;

	if(drawToDefaultFbo)
	{
		m_r->getDefaultFramebuffer().bind(cmdBuff, true);
		cmdBuff.setViewport(0, 0, m_r->getWindowWidth(), m_r->getWindowHeight());
	}
	else
	{
		m_fb.bind(cmdBuff, true);
		cmdBuff.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	}

	m_ppline.bind(cmdBuff);

	m_r->getIs()._getRt().bind(cmdBuff, 0);

	if(m_ssao.getEnabled())
	{
		m_ssao.getRt().bind(cmdBuff, 1);
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
}

} // end namespace anki
