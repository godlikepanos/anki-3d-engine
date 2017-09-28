// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/ForwardShading.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Volumetric.h>
#include <anki/renderer/DepthDownscale.h>

namespace anki
{

ForwardShading::~ForwardShading()
{
}

Error ForwardShading::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing forward shading");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize forward shading");
	}

	return err;
}

Error ForwardShading::initInternal(const ConfigSet&)
{
	m_width = m_r->getWidth() / FS_FRACTION;
	m_height = m_r->getHeight() / FS_FRACTION;

	// Create RT
	m_rt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_width,
		m_height,
		FORWARD_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1,
		"forward"));

	FramebufferInitInfo fbInit("forward");
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_texture = m_rt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	fbInit.m_colorAttachments[0].m_clearValue.m_colorf = {{0.0, 0.0, 0.0, 1.0}};
	fbInit.m_depthStencilAttachment.m_texture = m_r->getDepthDownscale().m_hd.m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	ANKI_CHECK(initVol());
	ANKI_CHECK(initUpscale());

	return Error::NONE;
}

Error ForwardShading::initVol()
{
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_vol.m_noiseTex));

	ANKI_CHECK(getResourceManager().loadResource("programs/ForwardShadingVolumetricUpscale.ankiprog", m_vol.m_prog));

	ShaderProgramResourceConstantValueInitList<3> consts(m_vol.m_prog);
	consts.add("NOISE_TEX_SIZE", U32(m_vol.m_noiseTex->getWidth()))
		.add("SRC_SIZE", Vec2(m_r->getWidth() / VOLUMETRIC_FRACTION, m_r->getHeight() / VOLUMETRIC_FRACTION))
		.add("FB_SIZE", Vec2(m_width, m_height));

	const ShaderProgramResourceVariant* variant;
	m_vol.m_prog->getOrCreateVariant(consts.get(), variant);
	m_vol.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error ForwardShading::initUpscale()
{
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_upscale.m_noiseTex));

	// Shader
	ANKI_CHECK(getResourceManager().loadResource("programs/ForwardShadingUpscale.ankiprog", m_upscale.m_prog));

	ShaderProgramResourceConstantValueInitList<3> consts(m_upscale.m_prog);
	consts.add("NOISE_TEX_SIZE", U32(m_upscale.m_noiseTex->getWidth()))
		.add("SRC_SIZE", Vec2(m_r->getWidth() / FS_FRACTION, m_r->getHeight() / FS_FRACTION))
		.add("FB_SIZE", Vec2(m_r->getWidth(), m_r->getWidth()));

	const ShaderProgramResourceVariant* variant;
	m_upscale.m_prog->getOrCreateVariant(consts.get(), variant);
	m_upscale.m_grProg = variant->getProgram();

	return Error::NONE;
}

void ForwardShading::drawVolumetric(RenderingContext& ctx, CommandBufferPtr cmdb)
{
	cmdb->bindShaderProgram(m_vol.m_grProg);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->setDepthWrite(false);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);

	Vec4* unis = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	computeLinearizeDepthOptimal(ctx.m_renderQueue->m_cameraNear, ctx.m_renderQueue->m_cameraFar, unis->x(), unis->y());

	cmdb->informTextureSurfaceCurrentUsage(
		m_r->getVolumetric().m_main.getRt(), TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);

	cmdb->bindTextureAndSampler(0, 0, m_r->getDepthDownscale().m_hd.m_colorRt, m_r->getNearestSampler());
	cmdb->bindTextureAndSampler(0, 1, m_r->getDepthDownscale().m_qd.m_colorRt, m_r->getNearestSampler());
	cmdb->bindTexture(0, 2, m_r->getVolumetric().m_main.getRt());
	cmdb->bindTexture(0, 3, m_vol.m_noiseTex->getGrTexture());

	m_r->drawQuad(cmdb);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	cmdb->setDepthWrite(true);
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
}

void ForwardShading::drawUpscale(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	Vec4* linearDepth = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	computeLinearizeDepthOptimal(
		ctx.m_renderQueue->m_cameraNear, ctx.m_renderQueue->m_cameraFar, linearDepth->x(), linearDepth->y());

	cmdb->bindTexture(0, 0, m_r->getGBuffer().m_depthRt);
	cmdb->bindTextureAndSampler(0, 1, m_r->getDepthDownscale().m_hd.m_colorRt, m_r->getNearestSampler());
	cmdb->bindTexture(0, 2, m_rt);
	cmdb->bindTexture(0, 3, m_upscale.m_noiseTex->getGrTexture());

	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA);

	cmdb->bindShaderProgram(m_upscale.m_grProg);
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	m_r->drawQuad(cmdb);

	// Restore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

void ForwardShading::buildCommandBuffers(RenderingContext& ctx, U threadId, U threadCount) const
{
	const U problemSize = ctx.m_renderQueue->m_forwardShadingRenderables.getSize();
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		// Early exit
		return;
	}

	// Create the command buffer and set some state
	CommandBufferInitInfo cinf;
	cinf.m_flags = CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::GRAPHICS_WORK;
	if(end - start < COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS)
	{
		cinf.m_flags |= CommandBufferFlag::SMALL_BATCH;
	}
	cinf.m_framebuffer = m_fb;
	CommandBufferPtr cmdb = m_r->getGrManager().newInstance<CommandBuffer>(cinf);
	ctx.m_forwardShading.m_commandBuffers[threadId] = cmdb;

	cmdb->informTextureCurrentUsage(m_r->getDepthDownscale().m_qd.m_colorRt, TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureSurfaceCurrentUsage(
		m_r->getDepthDownscale().m_hd.m_colorRt, TextureSurfaceInfo(0, 0, 0, 0), TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->informTextureSurfaceCurrentUsage(m_r->getDepthDownscale().m_hd.m_depthRt,
		TextureSurfaceInfo(0, 0, 0, 0),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ);
	cmdb->informTextureCurrentUsage(m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE);
	cmdb->informTextureCurrentUsage(m_r->getShadowMapping().m_shadowAtlas, TextureUsageBit::SAMPLED_FRAGMENT);

	cmdb->bindTexture(0, 0, m_r->getDepthDownscale().m_qd.m_colorRt);
	cmdb->bindTexture(0, 1, m_r->getShadowMapping().m_shadowAtlas);
	bindUniforms(cmdb, 0, 0, ctx.m_lightShading.m_commonToken);
	bindUniforms(cmdb, 0, 1, ctx.m_lightShading.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, ctx.m_lightShading.m_spotLightsToken);
	bindStorage(cmdb, 0, 0, ctx.m_lightShading.m_clustersToken);
	bindStorage(cmdb, 0, 1, ctx.m_lightShading.m_lightIndicesToken);

	cmdb->setViewport(0, 0, m_width, m_height);
	cmdb->setBlendFactors(
		0, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendFactor::SRC_ALPHA, BlendFactor::DST_ALPHA, BlendFactor::ZERO);
	cmdb->setBlendOperation(0, BlendOperation::ADD);
	cmdb->setDepthWrite(false);

	// Start drawing
	m_r->getSceneDrawer().drawRange(Pass::GB_FS,
		ctx.m_renderQueue->m_viewMatrix,
		ctx.m_viewProjMatJitter,
		cmdb,
		ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + start,
		ctx.m_renderQueue->m_forwardShadingRenderables.getBegin() + end);
}

void ForwardShading::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void ForwardShading::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_rt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void ForwardShading::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);
	cmdb->setViewport(0, 0, m_width, m_height);

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_forwardShading.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_forwardShading.m_commandBuffers[i]);
		}
	}

	cmdb->endRenderPass();
}

} // end namespace anki
