// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Volumetric.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/LightBin.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

Error Volumetric::initMain(const ConfigSet& config)
{
	// Misc
	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_main.m_noiseTex));

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("programs/VolumetricFog.ankiprog", m_main.m_prog));

	ShaderProgramResourceMutationInitList<1> mutators(m_main.m_prog);
	mutators.add("ENABLE_SHADOWS", 1);

	ShaderProgramResourceConstantValueInitList<3> consts(m_main.m_prog);
	consts.add("FB_SIZE", UVec2(m_width, m_height))
		.add("CLUSTER_COUNT",
			UVec3(m_r->getLightShading().getLightBin().getClusterer().getClusterCountX(),
				m_r->getLightShading().getLightBin().getClusterer().getClusterCountY(),
				m_r->getLightShading().getLightBin().getClusterer().getClusterCountZ()))
		.add("NOISE_MAP_SIZE", U32(m_main.m_noiseTex->getWidth()));

	const ShaderProgramResourceVariant* variant;
	m_main.m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_main.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error Volumetric::initHBlur(const ConfigSet& config)
{
	// Progs
	ANKI_CHECK(m_r->getResourceManager().loadResource("programs/LumaAwareBlur.ankiprog", m_hblur.m_prog));

	ShaderProgramResourceMutationInitList<3> mutators(m_hblur.m_prog);
	mutators.add("HORIZONTAL", 1).add("KERNEL_SIZE", 11).add("COLOR_COMPONENTS", 3);
	ShaderProgramResourceConstantValueInitList<1> consts(m_hblur.m_prog);
	consts.add("TEXTURE_SIZE", UVec2(m_width, m_height));

	const ShaderProgramResourceVariant* variant;
	m_hblur.m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_hblur.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error Volumetric::initVBlur(const ConfigSet& config)
{
	// Progs
	ANKI_CHECK(m_r->getResourceManager().loadResource("programs/LumaAwareBlur.ankiprog", m_vblur.m_prog));

	ShaderProgramResourceMutationInitList<3> mutators(m_vblur.m_prog);
	mutators.add("HORIZONTAL", 0).add("KERNEL_SIZE", 11).add("COLOR_COMPONENTS", 3);
	ShaderProgramResourceConstantValueInitList<1> consts(m_vblur.m_prog);
	consts.add("TEXTURE_SIZE", UVec2(m_width, m_height));

	const ShaderProgramResourceVariant* variant;
	m_vblur.m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_vblur.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error Volumetric::init(const ConfigSet& config)
{
	m_width = m_r->getWidth() / VOLUMETRIC_FRACTION;
	m_height = m_r->getHeight() / VOLUMETRIC_FRACTION;

	ANKI_R_LOGI("Initializing volumetric pass. Size %ux%u", m_width, m_height);

	for(U i = 0; i < 2; ++i)
	{
		// RT
		TextureInitInfo rtInit = m_r->create2DRenderTargetInitInfo(m_width,
			m_height,
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
			SamplingFilter::LINEAR,
			1,
			"volmain");
		rtInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_rtTextures[i] = m_r->createAndClearRenderTarget(rtInit);
	}

	// FB
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	Error err = initMain(config);

	if(!err)
	{
		err = initHBlur(config);
	}

	if(!err)
	{
		err = initVBlur(config);
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to initialize volumetric pass");
	}

	return err;
}

void Volumetric::runMain(CommandBufferPtr& cmdb, const RenderingContext& ctx, const RenderGraph& rgraph)
{
	cmdb->setViewport(0, 0, m_width, m_height);

	cmdb->bindTexture(0, 0, rgraph.getTexture(m_r->getDepthDownscale().getQuarterColorRt()));
	cmdb->bindTexture(0, 1, m_main.m_noiseTex->getGrTexture());
	cmdb->bindTexture(0, 2, rgraph.getTexture(m_runCtx.m_rts[(m_r->getFrameCount() + 1) & 1]));
	cmdb->bindTexture(0, 3, rgraph.getTexture(m_r->getShadowMapping().getShadowmapRt()));

	const LightShadingResources& rsrc = m_r->getLightShading().getResources();
	bindUniforms(cmdb, 0, 0, rsrc.m_commonToken);
	bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);

	struct Unis
	{
		Vec4 m_linearizeNoiseTexOffsetLayer;
		Vec4 m_fogParticleColorPad1;
		Mat4 m_prevViewProjMatMulInvViewProjMat;
	};

	Unis* uniforms = allocateAndBindUniforms<Unis*>(sizeof(Unis), cmdb, 0, 3);
	computeLinearizeDepthOptimal(ctx.m_renderQueue->m_cameraNear,
		ctx.m_renderQueue->m_cameraFar,
		uniforms->m_linearizeNoiseTexOffsetLayer.x(),
		uniforms->m_linearizeNoiseTexOffsetLayer.y());
	F32 texelOffset = 1.0 / m_main.m_noiseTex->getWidth();
	uniforms->m_linearizeNoiseTexOffsetLayer.z() = m_r->getFrameCount() * texelOffset;
	uniforms->m_linearizeNoiseTexOffsetLayer.w() = m_r->getFrameCount() & (m_main.m_noiseTex->getLayerCount() - 1);
	uniforms->m_fogParticleColorPad1 = Vec4(m_main.m_fogParticleColor, 0.0);
	uniforms->m_prevViewProjMatMulInvViewProjMat =
		ctx.m_prevViewProjMat * ctx.m_renderQueue->m_viewProjectionMatrix.getInverse();

	bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 1, rsrc.m_lightIndicesToken);

	cmdb->bindShaderProgram(m_main.m_grProg);

	m_r->drawQuad(cmdb);
}

void Volumetric::runHBlur(CommandBufferPtr& cmdb, const RenderGraph& rgraph)
{
	cmdb->bindTexture(0, 0, rgraph.getTexture(m_runCtx.m_rts[m_r->getFrameCount() & 1]));
	cmdb->bindShaderProgram(m_hblur.m_grProg);
	cmdb->setViewport(0, 0, m_width, m_height);

	m_r->drawQuad(cmdb);
}

void Volumetric::runVBlur(CommandBufferPtr& cmdb, const RenderGraph& rgraph)
{
	cmdb->bindTexture(0, 0, rgraph.getTexture(m_runCtx.m_rts[(m_r->getFrameCount() + 1) & 1]));
	cmdb->bindShaderProgram(m_vblur.m_grProg);
	cmdb->setViewport(0, 0, m_width, m_height);

	m_r->drawQuad(cmdb);
}

void Volumetric::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RTs
	const U rtToRenderIdx = m_r->getFrameCount() & 1;
	m_runCtx.m_rts[rtToRenderIdx] =
		rgraph.importRenderTarget("VOL #1", m_rtTextures[rtToRenderIdx], TextureUsageBit::NONE);
	const U rtToReadIdx = !rtToRenderIdx;
	m_runCtx.m_rts[rtToReadIdx] =
		rgraph.importRenderTarget("VOL #2", m_rtTextures[rtToReadIdx], TextureUsageBit::SAMPLED_FRAGMENT);

	// Create main render pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("VOL main");

		pass.setWork(runMainCallback, this, 0);
		pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rts[rtToRenderIdx]}}, {});

		pass.newConsumer({m_r->getDepthDownscale().getQuarterColorRt(), TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({m_runCtx.m_rts[rtToRenderIdx], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({m_runCtx.m_rts[rtToReadIdx], TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newProducer({m_runCtx.m_rts[rtToRenderIdx], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Create HBlur pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("VOL hblur");

		pass.setWork(runHBlurCallback, this, 0);
		pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rts[rtToReadIdx]}}, {});

		pass.newConsumer({m_runCtx.m_rts[rtToReadIdx], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({m_runCtx.m_rts[rtToRenderIdx], TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newProducer({m_runCtx.m_rts[rtToReadIdx], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Create VBlur pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("VOL vblur");

		pass.setWork(runVBlurCallback, this, 0);
		pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rts[rtToRenderIdx]}}, {});

		pass.newConsumer({m_runCtx.m_rts[rtToRenderIdx], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({m_runCtx.m_rts[rtToReadIdx], TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newProducer({m_runCtx.m_rts[rtToRenderIdx], TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}
}

RenderTargetHandle Volumetric::getRt() const
{
	return m_runCtx.m_rts[m_r->getFrameCount() & 1];
}

} // end namespace anki
