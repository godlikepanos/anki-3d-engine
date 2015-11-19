// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Sslr.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Renderer.h>
#include <anki/misc/ConfigSet.h>

namespace anki {

//==============================================================================
Error Sslr::initInternal(const ConfigSet& config)
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

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_reflectionFrag,
		"shaders/PpsSslr.frag.glsl", pps.toCString(), "r_"));

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(
		m_reflectionFrag->getGrShader(), colorState, m_reflectionPpline);

	// Blit
	ANKI_CHECK(getResourceManager().loadResource(
		"shaders/Blit.frag.glsl", m_blitFrag));

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
	FramebufferInitializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// Create resource groups
	ResourceGroupInitializer rcInit;
	rcInit.m_textures[0].m_texture = m_r->getIs().getRt();
	rcInit.m_textures[1].m_texture = m_r->getMs().getDepthRt();
	rcInit.m_textures[2].m_texture = m_r->getMs().getRt1();
	rcInit.m_textures[3].m_texture = m_r->getMs().getRt2();

	rcInit.m_uniformBuffers[0].m_buffer =
		m_r->getPps().getSsao().getUniformBuffer();

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	ResourceGroupInitializer rcInitBlit;
	rcInitBlit.m_textures[0].m_texture = m_rt;

	m_rcGroupBlit = getGrManager().newInstance<ResourceGroup>(rcInitBlit);

	// Create IS FB
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::LOAD;
	m_isFb = getGrManager().newInstance<Framebuffer>(fbInit);

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Sslr::init(const ConfigSet& config)
{
	Error err = initInternal(config);

	if(err)
	{
		ANKI_LOGE("Failed to init PPS SSLR");
	}

	return err;
}

//==============================================================================
void Sslr::run(CommandBufferPtr& cmdBuff)
{
	ANKI_ASSERT(m_enabled);

	// Compute the reflection
	//
	cmdBuff->bindFramebuffer(m_fb);
	cmdBuff->setViewport(0, 0, m_width, m_height);
	cmdBuff->bindPipeline(m_reflectionPpline);
	cmdBuff->bindResourceGroup(m_rcGroup, 0, nullptr);

	m_r->drawQuad(cmdBuff);

	// Write the reflection back to IS RT
	//
	cmdBuff->bindFramebuffer(m_isFb);
	cmdBuff->bindPipeline(m_blitPpline);
	cmdBuff->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdBuff->bindResourceGroup(m_rcGroupBlit, 0, nullptr);

	m_r->drawQuad(cmdBuff);
}

} // end namespace anki

