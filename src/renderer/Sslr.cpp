// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sslr.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Error Sslr::init(const ConfigSet& config)
{
	Error err = ErrorCode::NONE;

	m_enabled = config.get("pps.sslr.enabled");

	if(!m_enabled)
	{
		return err;
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
	String pps;
	String::ScopeDestroyer ppsd(&pps, getAllocator());

	err = pps.sprintf(getAllocator(),
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n",
		m_width, m_height);
	if(err) return err;

	err = m_reflectionFrag.loadToCache(&getResourceManager(),
		"shaders/PpsSslr.frag.glsl", pps.toCString(), "r_");
	if(err) return err;

	err = m_r->createDrawQuadProgramPipeline(
		m_reflectionFrag->getGlProgram(), m_reflectionPpline);
	if(err) return err;

	// Sampler
	GlCommandBufferHandle cmdBuff;
	err = cmdBuff.create(&getGlDevice());
	if(err) return err;
	err = m_depthMapSampler.create(cmdBuff);
	if(err) return err;
	m_depthMapSampler.setFilter(cmdBuff, GlSamplerHandle::Filter::NEAREST);

	// Blit
	err = m_blitFrag.load("shaders/Blit.frag.glsl", &getResourceManager());
	if(err) return err;
	err = m_r->createDrawQuadProgramPipeline(
		m_blitFrag->getGlProgram(), m_blitPpline);
	if(err) return err;

	// Init FBOs and RTs and blurring
	if(m_blurringIterationsCount > 0)
	{
		err = initBlurring(*m_r, m_width, m_height, 9, 0.0);
		if(err) return err;
	}
	else
	{
		Direction& dir = m_dirs[(U)DirectionEnum::VERTICAL];

		err = m_r->createRenderTarget(m_width, m_height, GL_RGB8, GL_RGB, 
			GL_UNSIGNED_BYTE, 1, dir.m_rt);
		if(err) return err;

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(cmdBuff, GlTextureHandle::Filter::LINEAR);

		// Create FB
		err = dir.m_fb.create(cmdBuff, {{dir.m_rt, GL_COLOR_ATTACHMENT0}});
		if(err) return err;
	}

	cmdBuff.finish();

	return err;
}

//==============================================================================
Error Sslr::run(GlCommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);
	Error err = ErrorCode::NONE;

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
		err = runBlurring(*m_r, cmdBuff);
		if(err) return err;
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

	return err;
}

} // end namespace anki

