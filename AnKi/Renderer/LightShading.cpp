// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/IndirectSpecular.h>
#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

LightShading::LightShading(Renderer* r)
	: RendererObject(r)
{
}

LightShading::~LightShading()
{
}

Error LightShading::init()
{
	ANKI_R_LOGV("Initializing light shading");

	Error err = initLightShading();

	if(!err)
	{
		err = initSkybox();
	}

	if(!err)
	{
		err = initApplyFog();
	}

	if(!err)
	{
		err = initApplyIndirect();
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to init light stage");
	}

	return err;
}

Error LightShading::initLightShading()
{
	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("Shaders/LightShading.ankiprog", m_lightShading.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_lightShading.m_prog);
	variantInitInfo.addConstant("TILE_COUNTS", m_r->getTileCounts());
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("TILE_SIZE", m_r->getTileSize());
	const ShaderProgramResourceVariant* variant;

	variantInitInfo.addMutation("USE_SHADOW_LAYERS", 0);
	m_lightShading.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_lightShading.m_grProg[0] = variant->getProgram();

	variantInitInfo.addMutation("USE_SHADOW_LAYERS", 1);
	m_lightShading.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_lightShading.m_grProg[1] = variant->getProgram();

	// Create RT descr
	m_lightShading.m_rtDescr = m_r->create2DRenderTargetDescription(
		m_r->getInternalResolution().x(), m_r->getInternalResolution().y(), m_r->getHdrFormat(), "Light Shading");
	m_lightShading.m_rtDescr.bake();

	// Create FB descr
	m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::DONT_CARE;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;

	if(getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs())
	{
		m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth = m_r->getVrsSriGeneration().getSriTexelDimension();
		m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelHeight = m_r->getVrsSriGeneration().getSriTexelDimension();
	}

	m_lightShading.m_fbDescr.bake();

	return Error::NONE;
}

Error LightShading::initSkybox()
{
	ANKI_CHECK(getResourceManager().loadResource("Shaders/LightShadingSkybox.ankiprog", m_skybox.m_prog));

	for(U32 method = 0; method < 2; ++method)
	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_skybox.m_prog);
		variantInitInfo.addMutation("METHOD", method);
		const ShaderProgramResourceVariant* variant;
		m_skybox.m_prog->getOrCreateVariant(variantInitInfo, variant);

		m_skybox.m_grProgs[method] = variant->getProgram();
	}

	return Error::NONE;
}

Error LightShading::initApplyFog()
{
	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("Shaders/LightShadingApplyFog.ankiprog", m_applyFog.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_applyFog.m_prog);
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("FINAL_Z_SPLIT", m_r->getVolumetricFog().getFinalClusterInZ());

	const ShaderProgramResourceVariant* variant;
	m_applyFog.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_applyFog.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error LightShading::initApplyIndirect()
{
	ANKI_CHECK(getResourceManager().loadResource("Shaders/LightShadingApplyIndirect.ankiprog", m_applyIndirect.m_prog));
	const ShaderProgramResourceVariant* variant;
	m_applyIndirect.m_prog->getOrCreateVariant(variant);
	m_applyIndirect.m_grProg = variant->getProgram();
	return Error::NONE;
}

void LightShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	if(enableVrs)
	{
		// Just set some low value, the attachment will take over
		cmdb->setVrsRate(VrsRate::_1x1);
	}

	// Do light shading first
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == 0)
	{
		cmdb->bindShaderProgram(m_lightShading.m_grProg[m_r->getRtShadowsEnabled()]);
		cmdb->setDepthWrite(false);

		// Bind all
		const ClusteredShadingContext& binning = ctx.m_clusteredShading;
		bindUniforms(cmdb, 0, 0, binning.m_clusteredShadingUniformsToken);

		bindUniforms(cmdb, 0, 1, binning.m_pointLightsToken);
		bindUniforms(cmdb, 0, 2, binning.m_spotLightsToken);
		rgraphCtx.bindColorTexture(0, 3, m_r->getShadowMapping().getShadowmapRt());

		bindStorage(cmdb, 0, 4, binning.m_clustersToken);

		cmdb->bindSampler(0, 5, m_r->getSamplers().m_nearestNearestClamp);
		cmdb->bindSampler(0, 6, m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindColorTexture(0, 7, m_r->getGBuffer().getColorRt(0));
		rgraphCtx.bindColorTexture(0, 8, m_r->getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 9, m_r->getGBuffer().getColorRt(2));
		rgraphCtx.bindTexture(0, 10, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

		if(m_r->getRtShadowsEnabled())
		{
			rgraphCtx.bindColorTexture(0, 11, m_r->getRtShadows().getRt());
		}
		else
		{
			rgraphCtx.bindColorTexture(0, 12, m_r->getShadowmapsResolve().getRt());
		}

		// Draw
		drawQuad(cmdb);
	}

	// Apply indirect
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == 0)
	{
		cmdb->setDepthWrite(false);
		cmdb->bindShaderProgram(m_applyIndirect.m_grProg);

		cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
		cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindColorTexture(0, 2, m_r->getIndirectDiffuse().getRt());
		rgraphCtx.bindColorTexture(0, 3, m_r->getIndirectSpecular().getRt());
		rgraphCtx.bindColorTexture(0, 4, m_r->getDepthDownscale().getHiZRt());
		rgraphCtx.bindTexture(0, 5, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		rgraphCtx.bindColorTexture(0, 6, m_r->getGBuffer().getColorRt(0));
		rgraphCtx.bindColorTexture(0, 7, m_r->getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 8, m_r->getGBuffer().getColorRt(2));
		cmdb->bindTexture(0, 9, m_r->getProbeReflections().getIntegrationLut());

		const ClusteredShadingContext& binning = ctx.m_clusteredShading;
		bindUniforms(cmdb, 0, 10, binning.m_clusteredShadingUniformsToken);

		const Vec4 pc(2.0f / Vec2(m_r->getInternalResolution()), 0.0f, 0.0f);
		cmdb->setPushConstants(&pc, sizeof(pc));

		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);

		drawQuad(cmdb);

		// Restore state
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	}

	// Skybox
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == 0)
	{
		cmdb->setDepthCompareOperation(CompareOperation::EQUAL);

		const Bool isSolidColor = ctx.m_renderQueue->m_skybox.m_skyboxTexture == nullptr;

		if(isSolidColor)
		{
			cmdb->bindShaderProgram(m_skybox.m_grProgs[0]);

			const Vec4 color(ctx.m_renderQueue->m_skybox.m_solidColor, 0.0);
			cmdb->setPushConstants(&color, sizeof(color));
		}
		else
		{
			cmdb->bindShaderProgram(m_skybox.m_grProgs[1]);

			class
			{
			public:
				Mat4 m_invertedViewProjectionJitter;
				Vec3 m_cameraPos;
				F32 m_padding = 0.0;
			} pc;

			pc.m_invertedViewProjectionJitter = ctx.m_matrices.m_invertedViewProjectionJitter;
			pc.m_cameraPos = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();

			cmdb->setPushConstants(&pc, sizeof(pc));

			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearRepeatAnisoResolutionScalingBias);
			cmdb->bindTexture(0, 1,
							  TextureViewPtr(const_cast<TextureView*>(ctx.m_renderQueue->m_skybox.m_skyboxTexture)));
		}

		drawQuad(cmdb);

		// Restore state
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}

	// Do the fog apply
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == rgraphCtx.m_secondLevelCommandBufferCount - 1u)
	{
		cmdb->bindShaderProgram(m_applyFog.m_grProg);

		// Bind all
		cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
		cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

		rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		rgraphCtx.bindColorTexture(0, 3, m_r->getVolumetricFog().getRt());

		class PushConsts
		{
		public:
			Vec2 m_padding;
			F32 m_near;
			F32 m_far;
		} regs;
		regs.m_near = ctx.m_renderQueue->m_cameraNear;
		regs.m_far = ctx.m_renderQueue->m_cameraFar;

		cmdb->setPushConstants(&regs, sizeof(regs));

		// finalPixelColor = pixelWithoutFog * transmitance + inScattering (see the shader)
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::SRC_ALPHA);

		drawQuad(cmdb);

		// Reset state
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
	}

	// Forward shading last
	if(enableVrs)
	{
		cmdb->setVrsRate(VrsRate::_2x2);
	}

	m_r->getForwardShading().run(ctx, rgraphCtx);

	if(enableVrs)
	{
		cmdb->setVrsRate(VrsRate::_1x1);
	}
}

void LightShading::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	const Bool fbDescrHasVrs = m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth > 0;

	if(enableVrs != fbDescrHasVrs)
	{
		// Re-bake the FB descriptor if the VRS state has changed

		if(enableVrs)
		{
			m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth =
				m_r->getVrsSriGeneration().getSriTexelDimension();
			m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelHeight =
				m_r->getVrsSriGeneration().getSriTexelDimension();
		}
		else
		{
			m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth = 0;
			m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelHeight = 0;
		}

		m_lightShading.m_fbDescr.bake();
	}

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

	RenderTargetHandle sriRt;
	if(enableVrs)
	{
		sriRt = m_r->getVrsSriGeneration().getSriRt();
	}

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Light&FW Shad");

	pass.setWork(computeNumberOfSecondLevelCommandBuffers(ctx.m_renderQueue->m_forwardShadingRenderables.getSize()),
				 [this, &ctx](RenderPassWorkContext& rgraphCtx) {
					 run(ctx, rgraphCtx);
				 });
	pass.setFramebufferInfo(m_lightShading.m_fbDescr, {m_runCtx.m_rt}, m_r->getGBuffer().getDepthRt(), sriRt);

	const TextureUsageBit readUsage = TextureUsageBit::SAMPLED_FRAGMENT;

	// All
	if(enableVrs)
	{
		pass.newDependency(RenderPassDependency(sriRt, TextureUsageBit::FRAMEBUFFER_SHADING_RATE));
	}

	// Light shading
	pass.newDependency(RenderPassDependency(m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));
	pass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(0), readUsage));
	pass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(1), readUsage));
	pass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), readUsage));
	pass.newDependency(
		RenderPassDependency(m_r->getGBuffer().getDepthRt(),
							 TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ,
							 TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)));
	pass.newDependency(RenderPassDependency(m_r->getShadowMapping().getShadowmapRt(), readUsage));
	if(m_r->getRtShadowsEnabled())
	{
		pass.newDependency(RenderPassDependency(m_r->getRtShadows().getRt(), readUsage));
	}
	else
	{
		pass.newDependency(RenderPassDependency(m_r->getShadowmapsResolve().getRt(), readUsage));
	}
	pass.newDependency(
		RenderPassDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::STORAGE_FRAGMENT_READ));

	// Apply indirect
	pass.newDependency(RenderPassDependency(m_r->getIndirectDiffuse().getRt(), readUsage));
	pass.newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), readUsage));
	pass.newDependency(RenderPassDependency(m_r->getIndirectSpecular().getRt(), readUsage));

	// Fog
	pass.newDependency(RenderPassDependency(m_r->getVolumetricFog().getRt(), readUsage));

	// For forward shading
	m_r->getForwardShading().setDependencies(ctx, pass);
}

} // end namespace anki
