// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>

namespace anki
{

TemporalAA::TemporalAA(Renderer* r)
	: RenderingPass(r)
{
}

TemporalAA::~TemporalAA()
{
}

Error TemporalAA::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing TAA");
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init TAA");
	}

	return ErrorCode::NONE;
}

Error TemporalAA::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(m_r->getResourceManager().loadResource("programs/TemporalAAResolve.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	for(U i = 0; i < 2; ++i)
	{
		m_rts[i] = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
			m_r->getHeight(),
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR,
			1,
			"taa"));

		FramebufferInitInfo fbInit("taa");
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_rts[i];
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fbs[i] = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	return ErrorCode::NONE;
}

void TemporalAA::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rts[m_r->getFrameCount() & 1],
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void TemporalAA::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fbs[m_r->getFrameCount() & 1]);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	cmdb->bindShaderProgram(m_grProg);
	cmdb->bindTextureAndSampler(0, 0, m_r->getGBuffer().m_depthRt, m_r->getLinearSampler());
	cmdb->bindTextureAndSampler(0, 1, m_r->getLightShading().getRt(), m_r->getLinearSampler());
	cmdb->informTextureCurrentUsage(m_rts[(m_r->getFrameCount() + 1) & 1], TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTextureAndSampler(0, 2, m_rts[(m_r->getFrameCount() + 1) & 1], m_r->getLinearSampler());

	Mat4* unis = allocateAndBindUniforms<Mat4*>(sizeof(Mat4), cmdb, 0, 0);
	*unis = ctx.m_jitterMat * ctx.m_prevViewProjMat * ctx.m_viewProjMatJitter.getInverse();

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void TemporalAA::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rts[m_r->getFrameCount() & 1],
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

TexturePtr TemporalAA::getRt() const
{
	return m_rts[m_r->getFrameCount() & 1];
}

} // end namespace anki
