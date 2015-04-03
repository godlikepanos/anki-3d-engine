// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
	m_enabled = config.get("pps.sslr.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
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
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n",
		m_width, m_height);

	ANKI_CHECK(m_reflectionFrag.loadToCache(&getResourceManager(),
		"shaders/PpsSslr.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(
		m_reflectionFrag->getGrShader(), m_reflectionPpline));

	// Sampler
	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(&getGrManager()));
	ANKI_CHECK(m_depthMapSampler.create(cmdBuff));
	m_depthMapSampler.setFilter(cmdBuff, SamplerHandle::Filter::NEAREST);

	// Blit
	ANKI_CHECK(
		m_blitFrag.load("shaders/Blit.frag.glsl", &getResourceManager()));
	ANKI_CHECK(
		m_r->createDrawQuadPipeline(m_blitFrag->getGrShader(), m_blitPpline));

	// Init FBOs and RTs and blurring
	if(m_blurringIterationsCount > 0)
	{
		ANKI_CHECK(initBlurring(*m_r, m_width, m_height, 9, 0.0));
	}
	else
	{
		Direction& dir = m_dirs[(U)DirectionEnum::VERTICAL];

		ANKI_CHECK(
			m_r->createRenderTarget(m_width, m_height, GL_RGB8, 1, dir.m_rt));

		// Set to bilinear because the blurring techniques take advantage of 
		// that
		dir.m_rt.setFilter(cmdBuff, TextureHandle::Filter::LINEAR);

		// Create FB
		FramebufferHandle::Initializer fbInit;
		fbInit.m_colorAttachmentsCount = 1;
		fbInit.m_colorAttachments[0].m_texture = dir.m_rt;
		ANKI_CHECK(dir.m_fb.create(cmdBuff, fbInit));
	}

	cmdBuff.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Sslr::run(CommandBufferHandle& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	// Compute the reflection
	//
	m_dirs[(U)DirectionEnum::VERTICAL].m_fb.bind(cmdBuff, true);
	cmdBuff.setViewport(0, 0, m_width, m_height);

	m_reflectionPpline.bind(cmdBuff);

	Array<TextureHandle, 3> tarr = {{
		m_r->getIs()._getRt(),
		m_r->getDp().getSmallDepthRt(),
		m_r->getMs()._getRt1()}};
	cmdBuff.bindTextures(0	, tarr.begin(), tarr.getSize()); 

	m_depthMapSampler.bind(cmdBuff, 1);
	m_r->getPps().getSsao().m_uniformsBuff.bindShaderBuffer(cmdBuff, 0);

	m_r->drawQuad(cmdBuff);

	SamplerHandle::bindDefault(cmdBuff, 1); // Unbind the sampler

	// Blurring
	//
	if(m_blurringIterationsCount > 0)
	{
		ANKI_CHECK(runBlurring(*m_r, cmdBuff));
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

	return ErrorCode::NONE;
}

} // end namespace anki

