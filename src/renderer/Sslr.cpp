// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sslr.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
void Sslr::init(const ConfigSet& config)
{
	m_enabled = config.get("pps.sslr.enabled");

	if(!m_enabled)
	{
		return;
	}

	// Size
	const F32 quality = config.get("pps.sslr.renderingQuality");
	m_blurringIterationsCount = 
		config.get("pps.sslr.blurringIterationsCount");

	m_width = quality * (F32)m_r->getWidth();
	alignRoundUp(16, m_width);
	m_height = quality * (F32)m_r->getHeight();
	alignRoundUp(16, m_height);

	// Programs
	String pps(getAllocator());

	pps.sprintf(
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n",
		m_width, m_height);

	m_reflectionFrag.loadToCache(&getResourceManager(),
		"shaders/PpsSslr.frag.glsl", pps.toCString(), "r_");

	m_reflectionPpline = m_r->createDrawQuadProgramPipeline(
		m_reflectionFrag->getGlProgram());

	// Sampler
	GlCommandBufferHandle cmdBuff;
	cmdBuff.create(&getGlDevice());
	m_depthMapSampler.create(cmdBuff);
	m_depthMapSampler.setFilter(cmdBuff, GlSamplerHandle::Filter::NEAREST);

	// Blit
	m_blitFrag.load("shaders/Blit.frag.glsl", &getResourceManager());
	m_blitPpline = m_r->createDrawQuadProgramPipeline(
		m_blitFrag->getGlProgram());

	// Init FBOs and RTs and blurring
	if(m_blurringIterationsCount > 0)
	{
		initBlurring(*m_r, m_width, m_height, 9, 0.0);
	}
	else
	{
		Direction& dir = m_dirs[(U)DirectionEnum::VERTICAL];

		m_r->createRenderTarget(m_width, m_height, GL_RGB8, GL_RGB, 
			GL_UNSIGNED_BYTE, 1, dir.m_rt);

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(cmdBuff, GlTextureHandle::Filter::LINEAR);

		// Create FB
		dir.m_fb.create(cmdBuff, {{dir.m_rt, GL_COLOR_ATTACHMENT0}});
	}

	cmdBuff.finish();
}

//==============================================================================
void Sslr::run(GlCommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	// Compute the reflection
	//
	m_dirs[(U)DirectionEnum::VERTICAL].m_fb.bind(cmdBuff, true);
	cmdBuff.setViewport(0, 0, m_width, m_height);

	m_reflectionPpline.bind(cmdBuff);

	Array<GlTextureHandle, 3> tarr = {{
		m_r->getIs()._getRt(),
		m_r->getMs()._getSmallDepthRt(),
		m_r->getMs()._getRt1()}};
	cmdBuff.bindTextures(0	, tarr.begin(), tarr.getSize()); 

	m_depthMapSampler.bind(cmdBuff, 1);
	m_r->getPps().getSsao().m_uniformsBuff.bindShaderBuffer(cmdBuff, 0);

	m_r->drawQuad(cmdBuff);

	GlSamplerHandle::bindDefault(cmdBuff, 1); // Unbind the sampler

	// Blurring
	//
	if(m_blurringIterationsCount > 0)
	{
		runBlurring(*m_r, cmdBuff);
	}

	// Write the reflection back to IS RT
	//
	m_r->getIs().m_fb.bind(cmdBuff, false);
	cmdBuff.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	cmdBuff.enableBlend(true);
	cmdBuff.setBlendFunctions(GL_ONE, GL_ONE);

	m_dirs[(U)DirectionEnum::VERTICAL].m_rt.bind(cmdBuff, 0);

	m_blitPpline.bind(cmdBuff);
	m_r->drawQuad(cmdBuff);

	cmdBuff.enableBlend(false);
}

} // end namespace anki

