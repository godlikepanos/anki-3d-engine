// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sslr.h"
#include "anki/renderer/Ssao.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Error Sslr::init(const ConfigSet& config)
{
	m_enabled = config.getNumber("pps.sslr.enabled");

	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	// Size
	const F32 quality = config.getNumber("pps.sslr.renderingQuality");

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

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(
		m_reflectionFrag->getGrShader(), colorState, m_reflectionPpline);

	// Blit
	ANKI_CHECK(
		m_blitFrag.load("shaders/Blit.frag.glsl", &getResourceManager()));

	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
	colorState.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	colorState.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;

	m_r->createDrawQuadPipeline(
		m_blitFrag->getGrShader(), colorState, m_blitPpline);

	// Init FBOs
	m_r->createRenderTarget(m_width, m_height,
		Is::RT_PIXEL_FORMAT, 1, SamplingFilter::LINEAR, 1, m_rt);

	// Create FB
	FramebufferPtr::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::LOAD;
	m_fb.create(&getGrManager(), fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
void Sslr::run(CommandBufferPtr& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	// Compute the reflection
	//
	m_fb.bind(cmdBuff);
	cmdBuff.setViewport(0, 0, m_width, m_height);

	m_reflectionPpline.bind(cmdBuff);

	Array<TexturePtr, 4> tarr = {{
		m_r->getIs().getRt(),
		m_r->getMs().getDepthRt(),
		m_r->getMs().getRt1(),
		m_r->getMs().getRt2()}};
	cmdBuff.bindTextures(0	, tarr.begin(), tarr.getSize());

	m_r->getPps().getSsao().getUniformBuffer().bindShaderBuffer(cmdBuff, 0);

	m_r->drawQuad(cmdBuff);

	SamplerPtr::bindDefault(cmdBuff, 1); // Unbind the sampler

	// Write the reflection back to IS RT
	//
	m_r->getIs().m_fb.bind(cmdBuff);
	cmdBuff.setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	m_rt.bind(cmdBuff, 0);

	m_blitPpline.bind(cmdBuff);
	m_r->drawQuad(cmdBuff);
}

} // end namespace anki

