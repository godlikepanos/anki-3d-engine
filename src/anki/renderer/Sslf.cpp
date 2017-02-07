// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Sslf.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

Error Sslf::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing screen space lens flare");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to init screen space lens flare");
	}

	return err;
}

Error Sslf::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(m_r->createShaderf("shaders/Sslf.frag.glsl",
		m_frag,
		"#define TEX_DIMENSIONS vec2(%u.0, %u.0)\n",
		m_r->getBloom().m_extractExposure.m_width,
		m_r->getBloom().m_extractExposure.m_height));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	ANKI_CHECK(getResourceManager().loadResource("engine_data/LensDirt.ankitex", m_lensDirtTex));

	return ErrorCode::NONE;
}

void Sslf::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Draw to the SSLF FB
	cmdb->bindShaderProgram(m_prog);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->bindTexture(0, 0, m_r->getBloom().m_extractExposure.m_rt);
	cmdb->bindTexture(0, 1, m_lensDirtTex->getGrTexture());

	m_r->drawQuad(cmdb);

	// Retore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
