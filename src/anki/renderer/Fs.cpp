// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Fs.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Sm.h>
#include <anki/renderer/Volumetric.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

Fs::~Fs()
{
}

Error Fs::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing forward shading");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize forward shading");
	}

	return err;
}

Error Fs::initInternal(const ConfigSet&)
{
	m_width = m_r->getWidth() / FS_FRACTION;
	m_height = m_r->getHeight() / FS_FRACTION;

	// Create RT
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_width,
		m_height,
		FS_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR));

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_colorAttachments[0].m_clearValue.m_colorf = {{0.0, 0.0, 0.0, 1.0}};
	fbInit.m_depthStencilAttachment.m_texture = m_r->getDepthDownscale().m_hd.m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	ANKI_CHECK(initVol());

	return ErrorCode::NONE;
}

Error Fs::initVol()
{
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_vol.m_noiseTex));

	ANKI_CHECK(m_r->createShaderf("shaders/VolumetricUpscale.frag.glsl",
		m_vol.m_frag,
		"#define SRC_SIZE vec2(float(%u), float(%u))\n"
		"#define FB_SIZE vec2(float(%u), float(%u))\n"
		"#define NOISE_TEX_SIZE %u\n",
		m_r->getWidth() / VOLUMETRIC_FRACTION,
		m_r->getHeight() / VOLUMETRIC_FRACTION,
		m_width,
		m_height,
		m_vol.m_noiseTex->getWidth()));

	m_r->createDrawQuadShaderProgram(m_vol.m_frag->getGrShader(), m_vol.m_prog);

	SamplerInitInfo sinit;
	sinit.m_repeat = false;
	sinit.m_mipmapFilter = SamplingFilter::NEAREST;
	m_vol.m_nearestSampler = getGrManager().newInstance<Sampler>(sinit);

	return ErrorCode::NONE;
}

void Fs::drawVolumetric(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	cmdb->bindShaderProgram(m_vol.m_prog);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setDepthWrite(false);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);

	Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	computeLinearizeDepthOptimal(ctx.m_near, ctx.m_far, unis->x(), unis->y());

	cmdb->informTextureSurfaceCurrentUsage(
		m_r->getDepthDownscale().m_qd.m_depthRt, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureSurfaceCurrentUsage(
		m_r->getVolumetric().m_main.m_rt, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);

	cmdb->bindTextureAndSampler(0, 0, m_r->getDepthDownscale().m_hd.m_depthRt, m_vol.m_nearestSampler);
	cmdb->bindTextureAndSampler(0, 1, m_r->getDepthDownscale().m_qd.m_depthRt, m_vol.m_nearestSampler);
	cmdb->bindTexture(0, 2, m_r->getVolumetric().m_main.m_rt);
	cmdb->bindTexture(0, 3, m_vol.m_noiseTex->getGrTexture());

	m_r->drawQuad(cmdb);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setDepthWrite(true);
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
}

Error Fs::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	U problemSize = ctx.m_visResults->getCount(VisibilityGroupType::RENDERABLES_FS);
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		// Early exit
		return ErrorCode::NONE;
	}

	// Create the command buffer and set some state
	CommandBufferInitInfo cinf;
	cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
	cinf.m_framebuffer = m_fb;
	CommandBufferPtr cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);
	ctx.m_fs.m_commandBuffers[threadId] = cmdb;

	cmdb->bindTexture(1, 0, m_r->getDepthDownscale().m_hd.m_depthRt);
	cmdb->bindTexture(1, 1, m_r->getSm().m_spotTexArray);
	cmdb->bindTexture(1, 2, m_r->getSm().m_omniTexArray);
	bindUniforms(cmdb, 1, 0, ctx.m_is.m_commonToken);
	bindUniforms(cmdb, 1, 1, ctx.m_is.m_pointLightsToken);
	bindUniforms(cmdb, 1, 2, ctx.m_is.m_spotLightsToken);
	bindStorage(cmdb, 1, 0, ctx.m_is.m_clustersToken);
	bindStorage(cmdb, 1, 1, ctx.m_is.m_lightIndicesToken);

	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setBlendFactors(
		0, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendFactor::SRC_ALPHA, BlendFactor::DST_ALPHA, BlendFactor::ZERO);
	cmdb->setBlendOperation(0, BlendOperation::ADD);
	cmdb->setDepthWrite(false);

	// Start drawing
	Error err = m_r->getSceneDrawer().drawRange(Pass::MS_FS,
		ctx.m_viewMat,
		ctx.m_viewProjMat,
		cmdb,
		ctx.m_visResults->getBegin(VisibilityGroupType::RENDERABLES_FS) + start,
		ctx.m_visResults->getBegin(VisibilityGroupType::RENDERABLES_FS) + end);

	return err;
}

void Fs::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Fs::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void Fs::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_fs.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_fs.m_commandBuffers[i]);
		}
	}

	cmdb->endRenderPass();
}

} // end namespace anki
