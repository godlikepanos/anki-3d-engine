// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/FsUpscale.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Fs.h>
#include <anki/renderer/Ssao.h>
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
		ANKI_LOGE("Failed to initialize forward shading upscale");
	}

	return err;
}

Error FsUpscale::initInternal(const ConfigSet& config)
{
	ANKI_LOGI("Initializing forward shading upscale");

	GrManager& gr = getGrManager();

	SamplerInitInfo sinit;
	sinit.m_repeat = false;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	m_nearestSampler = gr.newInstance<Sampler>(sinit);

	// Shader
	StringAuto pps(getFrameAllocator());
	pps.sprintf("#define SRC_SIZE uvec2(%uu, %uu)\n"
				"#define SSAO_ENABLED %u\n",
		m_r->getWidth() / FS_FRACTION,
		m_r->getHeight() / FS_FRACTION,
		1);

	ANKI_CHECK(m_r->createShader("shaders/FsUpscale.frag.glsl", m_frag, pps.toCString()));

	ANKI_CHECK(m_r->createShader("shaders/Quad.vert.glsl", m_vert, pps.toCString()));

	// Prog
	m_prog = gr.newInstance<ShaderProgram>(m_vert->getGrShader(), m_frag->getGrShader());

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
	const Frustum& fr = ctx.m_frustumComponent->getFrustum();
	computeLinearizeDepthOptimal(fr.getNear(), fr.getFar(), linearDepth->x(), linearDepth->y());

	cmdb->bindTexture(0, 0, m_r->getMs().m_depthRt);
	cmdb->bindTextureAndSampler(0, 1, m_r->getDepthDownscale().m_hd.m_depthRt, m_nearestSampler);
	cmdb->bindTexture(0, 2, m_r->getFs().getRt());
	cmdb->bindTexture(0, 3, m_r->getSsao().getRt());

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
