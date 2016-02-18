// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Sslf.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

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

	pps.sprintf("#define TEX_DIMENSIONS vec2(%u.0, %u.0)\n",
		m_r->getBloom().getWidth(),
		m_r->getBloom().getHeight());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_frag, "shaders/Sslf.frag.glsl", pps.toCString(), "r_"));

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_srcBlendMethod = BlendMethod::ONE;
	colorState.m_attachments[0].m_dstBlendMethod = BlendMethod::ONE;

	m_r->createDrawQuadPipeline(m_frag->getGrShader(), colorState, m_ppline);

	// Textures
	ANKI_CHECK(getResourceManager().loadResource(
		"engine_data/LensDirt.ankitex", m_lensDirtTex));

	// Create the resource group
	ResourceGroupInitInfo rcInit;
	rcInit.m_textures[0].m_texture = m_r->getBloom().getRt1();
	rcInit.m_textures[1].m_texture = m_lensDirtTex->getGrTexture();

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
void Sslf::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Draw to the SSLF FB
	cmdb->bindPipeline(m_ppline);
	cmdb->bindResourceGroup(m_rcGroup, 0, nullptr);

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

} // end namespace anki
