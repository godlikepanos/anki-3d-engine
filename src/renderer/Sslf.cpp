// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sslf.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Bloom.h"
#include "anki/misc/ConfigSet.h"

namespace anki {

//==============================================================================
Error Sslf::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to init screen space lens flare pass");
	}

	return err;
}

//==============================================================================
Error Sslf::initInternal(const ConfigSet& config)
{
	m_enabled = config.get("pps.sslf.enabled");
	if(!m_enabled)
	{
		return ErrorCode::NONE;
	}

	// Load program 1
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define TEX_DIMENSIONS vec2(%u.0, %u.0)\n",
		m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight());

	ANKI_CHECK(m_frag.loadToCache(&getResourceManager(),
		"shaders/PpsSslf.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(m_frag->getGrShader(), m_ppline));

	// Textures
	ANKI_CHECK(m_lensDirtTex.load(
		"engine_data/lens_dirt.ankitex", &getResourceManager()));

	// Create the render target and FB
	m_r->createRenderTarget(m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight(),
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM),
		1, SamplingFilter::LINEAR, 1, m_rt);

	FramebufferPtr::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation =
		AttachmentLoadOperation::DONT_CARE;
	m_fb.create(&getGrManager(), fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
void Sslf::run(CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(m_enabled);

	// Draw to the SSLF FB
	m_fb.bind(cmdb);
	cmdb.setViewport(0, 0, m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight());

	m_ppline.bind(cmdb);

	Array<TexturePtr, 2> tarr = {{
		m_r->getPps().getBloom().getRt(),
		m_lensDirtTex->getGlTexture()}};
	cmdb.bindTextures(0, tarr.begin(), tarr.getSize());

	m_r->drawQuad(cmdb);
}

} // end namespace anki

