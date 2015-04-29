// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Sslf.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Bloom.h"

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
	// Load program 1
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define TEX_DIMENSIONS vec2(%u.0, %u.0)\n", 
		m_r->getPps().getBloom().getWidth(),
		m_r->getPps().getBloom().getHeight());

	ANKI_CHECK(m_frag.loadToCache(&getResourceManager(), 
		"shaders/PpsLfPseudoPass.frag.glsl", pps.toCString(), "r_"));

	ANKI_CHECK(m_r->createDrawQuadPipeline(m_frag->getGrShader(), m_ppline));

	// Textures
	ANKI_CHECK(m_lensDirtTex.load(
		"engine_data/lens_dirt.ankitex", &getResourceManager()));

	// Create the render target and FB
	ANKI_CHECK(m_r->createRenderTarget(m_r->getPps().getBloom().getWidth(), 
		m_r->getPps().getBloom().getHeight(), 
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM), 
		1, SamplingFilter::LINEAR, 1, m_rt));

	FramebufferHandle::Initializer fbInit;
	fbInit.m_colorAttachmentsCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = 
		AttachmentLoadOperation::DONT_CARE;
	ANKI_CHECK(m_fb.create(&getGrManager(), fbInit));

	return ErrorCode::NONE;
}

//==============================================================================
void Sslf::run(CommandBufferHandle& cmdb)
{
	m_fb.bind(cmdb);
	cmdb.setViewport(0, 0, m_r->getPps().getBloom().getWidth(), 
		m_r->getPps().getBloom().getHeight());

	m_ppline.bind(cmdb);

	Array<TextureHandle, 2> tarr = {{
		m_r->getPps().getBloom().getRt(), 
		m_lensDirtTex->getGlTexture()}};
	cmdb.bindTextures(0, tarr.begin(), tarr.getSize());

	m_r->drawQuad(cmdb);
}

} // end namespace anki

