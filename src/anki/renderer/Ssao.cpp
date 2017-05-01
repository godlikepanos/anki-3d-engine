// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssao.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/scene/SceneGraph.h>
#include <anki/util/Functions.h>
#include <anki/misc/ConfigSet.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

const PixelFormat Ssao::RT_PIXEL_FORMAT(ComponentFormat::R8, TransformFormat::UNORM);

TexturePtr SsaoMain::getRt() const
{
	return m_rt[m_r->getFrameCount() & 1];
}

TexturePtr SsaoMain::getPreviousRt() const
{
	return m_rt[(m_r->getFrameCount() + 1) & 1];
}

Error SsaoMain::init(const ConfigSet& config)
{
	for(U i = 0; i < 2; ++i)
	{
		// RT
		m_rt[i] = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_ssao->m_width,
			m_ssao->m_height,
			Ssao::RT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::CLEAR,
			SamplingFilter::LINEAR,
			1,
			"ssaomain"));

		// FB
		FramebufferInitInfo fbInit("ssaomain");
		fbInit.m_colorAttachmentCount = 1;
		fbInit.m_colorAttachments[0].m_texture = m_rt[i];
		fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fb[i] = getGrManager().newInstance<Framebuffer>(fbInit);
	}

	// Noise
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_noiseTex));

	// Shader
	ANKI_CHECK(m_r->createShaderf("shaders/Ssao.frag.glsl",
		m_frag,
		"#define NOISE_MAP_SIZE %u\n"
		"#define WIDTH %u\n"
		"#define HEIGHT %u\n"
		"#define RADIUS float(%f)\n",
		m_noiseTex->getWidth(),
		m_ssao->m_width,
		m_ssao->m_height,
		HEMISPHERE_RADIUS));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	return ErrorCode::NONE;
}

void SsaoMain::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void SsaoMain::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb[m_r->getFrameCount() & 1]);
	cmdb->setViewport(0, 0, m_ssao->m_width, m_ssao->m_height);
	cmdb->bindShaderProgram(m_prog);

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_qd.m_depthRt);
	cmdb->bindTextureAndSampler(0, 1, m_r->getMs().m_rt2, m_r->getLinearSampler());
	cmdb->bindTexture(0, 2, m_noiseTex->getGrTexture());
	cmdb->bindTexture(0, 3, m_rt[(m_r->getFrameCount() + 1) & 1]);

	struct Unis
	{
		Vec4 m_unprojectionParams;
		Vec4 m_projectionMat;
		Vec4 m_noiseLayerPad3;
		Mat4 m_prevViewProjMatMulInvViewProjMat;
	};

	Unis* unis = allocateAndBindUniforms<Unis*>(sizeof(Unis), cmdb, 0, 0);
	const Mat4& pmat = ctx.m_projMat;
	unis->m_unprojectionParams = ctx.m_unprojParams;
	unis->m_projectionMat = Vec4(pmat(0, 0), pmat(1, 1), pmat(2, 2), pmat(2, 3));
	unis->m_noiseLayerPad3 = Vec4(m_r->getFrameCount() % m_noiseTex->getLayerCount(), 0.0, 0.0, 0.0);
	unis->m_prevViewProjMatMulInvViewProjMat = ctx.m_prevViewProjMat * ctx.m_viewProjMat.getInverse();

	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void SsaoMain::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error SsaoHBlur::init(const ConfigSet& config)
{
	// RT
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_ssao->m_width,
		m_ssao->m_height,
		Ssao::RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		SamplingFilter::LINEAR,
		1,
		"ssaoblur"));

	// FB
	FramebufferInitInfo fbInit("ssaoblur");
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	// shader
	ANKI_CHECK(m_r->createShaderf("shaders/GaussianBlurGeneric.frag.glsl",
		m_frag,
		"#define HPASS\n"
		"#define COL_R\n"
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define KERNEL_SIZE 5\n",
		F32(m_ssao->m_width),
		F32(m_ssao->m_height)));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	return ErrorCode::NONE;
}

void SsaoHBlur::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(
		m_rt, TextureUsageBit::NONE, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, TextureSurfaceInfo(0, 0, 0, 0));
}

void SsaoHBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_ssao->m_width, m_ssao->m_height);
	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_ssao->m_main.m_rt[m_r->getFrameCount() & 1]);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void SsaoHBlur::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error SsaoVBlur::init(const ConfigSet& config)
{
	ANKI_CHECK(m_r->createShaderf("shaders/GaussianBlurGeneric.frag.glsl",
		m_frag,
		"#define VPASS\n"
		"#define COL_R\n"
		"#define TEXTURE_SIZE vec2(%f, %f)\n"
		"#define KERNEL_SIZE 5\n",
		F32(m_ssao->m_width),
		F32(m_ssao->m_height)));

	m_r->createDrawQuadShaderProgram(m_frag->getGrShader(), m_prog);

	return ErrorCode::NONE;
}

void SsaoVBlur::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_ssao->m_main.m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void SsaoVBlur::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_ssao->m_width, m_ssao->m_height);
	cmdb->beginRenderPass(m_ssao->m_main.m_fb[m_r->getFrameCount() & 1]);
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindTexture(0, 0, m_ssao->m_hblur.m_rt);
	m_r->drawQuad(cmdb);
	cmdb->endRenderPass();
}

void SsaoVBlur::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_ssao->m_main.m_rt[m_r->getFrameCount() & 1],
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

Error Ssao::init(const ConfigSet& config)
{
	m_width = m_r->getWidth() / SSAO_FRACTION;
	m_height = m_r->getHeight() / SSAO_FRACTION;

	ANKI_R_LOGI("Initializing SSAO. Size %ux%u", m_width, m_height);

	Error err = m_main.init(config);

	if(!err)
	{
		err = m_hblur.init(config);
	}

	if(!err)
	{
		err = m_vblur.init(config);
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to init PPS SSAO");
	}

	return err;
}

} // end namespace anki
