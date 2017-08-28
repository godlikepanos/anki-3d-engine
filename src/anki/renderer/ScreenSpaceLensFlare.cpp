// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ScreenSpaceLensFlare.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

Error ScreenSpaceLensFlare::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing screen space lens flare");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to init screen space lens flare");
	}

	return err;
}

Error ScreenSpaceLensFlare::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(getResourceManager().loadResource("engine_data/LensDirt.ankitex", m_lensDirtTex));

	ANKI_CHECK(getResourceManager().loadResource("programs/ScreenSpaceLensFlare.ankiprog", m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_prog);
	consts.add(
		"INPUT_TEX_SIZE", UVec2(m_r->getBloom().m_extractExposure.m_width, m_r->getBloom().m_extractExposure.m_height));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void ScreenSpaceLensFlare::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Draw to the SSLF FB
	cmdb->bindShaderProgram(m_grProg);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->bindTexture(0, 0, m_r->getBloom().m_extractExposure.m_rt);
	cmdb->bindTexture(0, 1, m_lensDirtTex->getGrTexture());

	m_r->drawQuad(cmdb);

	// Retore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
