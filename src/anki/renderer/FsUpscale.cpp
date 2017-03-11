// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/FsUpscale.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Fs.h>
#include <anki/renderer/Volumetric.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

Error FsUpscale::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize forward shading upscale");
	}

	return err;
}

Error FsUpscale::initInternal(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing forward shading upscale");

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_noiseTex));

	GrManager& gr = getGrManager();

	SamplerInitInfo sinit;
	sinit.m_repeat = false;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	m_nearestSampler = gr.newInstance<Sampler>(sinit);

	// Shader
	ANKI_CHECK(m_r->createShaderf("shaders/FsUpscale.frag.glsl",
		m_frag,
		"#define FB_SIZE uvec2(%uu, %uu)\n"
		"#define SRC_SIZE uvec2(%uu, %uu)\n"
		"#define NOISE_TEX_SIZE %u\n",
		m_r->getWidth(),
		m_r->getHeight(),
		m_r->getWidth() / FS_FRACTION,
		m_r->getHeight() / FS_FRACTION,
		m_noiseTex->getWidth()));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	// Create FB
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_r->getIs().getRt();
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

void FsUpscale::run(RenderingContext& ctx)
{
	CommandBufferPtr cmdb = ctx.m_commandBuffer;

	Vec4* linearDepth = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	computeLinearizeDepthOptimal(ctx.m_near, ctx.m_far, linearDepth->x(), linearDepth->y());

	cmdb->bindTexture(0, 0, m_r->getMs().m_depthRt);
	cmdb->bindTextureAndSampler(0, 1, m_r->getDepthDownscale().m_hd.m_depthRt, m_nearestSampler);
	cmdb->bindTexture(0, 2, m_r->getFs().getRt());
	cmdb->bindTexture(0, 3, m_noiseTex->getGrTexture());

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA);

	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_prog);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
