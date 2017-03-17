// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Taa.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>

namespace anki
{

class TaaUniforms
{
public:
	Mat4 m_prevViewProjMat;
	Mat4 m_invViewProjMat;
};

Taa::~Taa()
{
}

Error Taa::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing TAA");
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init TAA");
	}

	return ErrorCode::NONE;
}

Error Taa::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/Taa.frag.glsl", m_frag));
	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	for(U i = 0; i < 2; ++i)
	{
		m_rts[i] = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
			m_r->getHeight(),
			IS_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR));

		FramebufferInitInfo fbInit;
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_rts[i];
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fbs[i] = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	return ErrorCode::NONE;
}

void Taa::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rts[m_r->getFrameCount() & 1],
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Taa::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fbs[m_r->getFrameCount() & 1]);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTextureAndSampler(0, 0, m_r->getMs().m_depthRt, m_r->getNearestSampler());
	cmdb->bindTextureAndSampler(0, 1, m_r->getIs().getRt(), m_r->getNearestSampler());
	cmdb->bindTextureAndSampler(0, 2, m_rts[(m_r->getFrameCount() + 1) & 1], m_r->getLinearSampler());

	TaaUniforms* unis = allocateAndBindUniforms<TaaUniforms*>(sizeof(TaaUniforms), cmdb, 0, 0);
	unis->m_prevViewProjMat = ctx.m_jitterMat * ctx.m_prevViewProjMat;
	unis->m_invViewProjMat = ctx.m_viewProjMatJitter.getInverse();

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void Taa::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rts[m_r->getFrameCount() & 1],
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

TexturePtr Taa::getRt() const
{
	return m_rts[m_r->getFrameCount() & 1];
}

} // end namespace anki
