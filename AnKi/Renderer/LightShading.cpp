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
	registerDebugRenderTarget("LightShading");
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
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/LightShading.ankiprogbin", m_lightShading.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_lightShading.m_prog);
	variantInitInfo.addConstant("kTileCount", m_r->getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kTileSize", m_r->getTileSize());
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
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kLoad;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::kDontCare;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;

	if(getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs())
	{
		m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth = m_r->getVrsSriGeneration().getSriTexelDimension();
		m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelHeight = m_r->getVrsSriGeneration().getSriTexelDimension();
	}

	m_lightShading.m_fbDescr.bake();

	// Debug visualization
	ANKI_CHECK(
		getResourceManager().loadResource("ShaderBinaries/VisualizeHdrRenderTarget.ankiprogbin", m_visualizeRtProg));
	m_visualizeRtProg->getOrCreateVariant(variant);
	m_visualizeRtGrProg = variant->getProgram();

	return Error::kNone;
}

Error LightShading::initSkybox()
{
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/LightShadingSkybox.ankiprogbin", m_skybox.m_prog));

	for(U32 method = 0; method < 2; ++method)
	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_skybox.m_prog);
		variantInitInfo.addMutation("METHOD", method);
		const ShaderProgramResourceVariant* variant;
		m_skybox.m_prog->getOrCreateVariant(variantInitInfo, variant);

		m_skybox.m_grProgs[method] = variant->getProgram();
	}

	return Error::kNone;
}

Error LightShading::initApplyFog()
{
	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/LightShadingApplyFog.ankiprogbin", m_applyFog.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_applyFog.m_prog);
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kFinalZSplit", m_r->getVolumetricFog().getFinalClusterInZ());

	const ShaderProgramResourceVariant* variant;
	m_applyFog.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_applyFog.m_grProg = variant->getProgram();

	return Error::kNone;
}

Error LightShading::initApplyIndirect()
{
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/LightShadingApplyIndirect.ankiprogbin",
												 m_applyIndirect.m_prog));
	const ShaderProgramResourceVariant* variant;
	m_applyIndirect.m_prog->getOrCreateVariant(variant);
	m_applyIndirect.m_grProg = variant->getProgram();
	return Error::kNone;
}

void LightShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs();
	if(enableVrs)
	{
		// Just set some low value, the attachment will take over
		cmdb->setVrsRate(VrsRate::k1x1);
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
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

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
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 6, m_r->getGBuffer().getColorRt(0));
		rgraphCtx.bindColorTexture(0, 7, m_r->getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 8, m_r->getGBuffer().getColorRt(2));
		cmdb->bindTexture(0, 9, m_r->getProbeReflections().getIntegrationLut());

		const ClusteredShadingContext& binning = ctx.m_clusteredShading;
		bindUniforms(cmdb, 0, 10, binning.m_clusteredShadingUniformsToken);

		const Vec4 pc(ctx.m_renderQueue->m_cameraNear, ctx.m_renderQueue->m_cameraFar, 0.0f, 0.0f);
		cmdb->setPushConstants(&pc, sizeof(pc));

		cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kOne);

		drawQuad(cmdb);

		// Restore state
		cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	// Skybox
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == 0)
	{
		cmdb->setDepthCompareOperation(CompareOperation::kEqual);

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
		cmdb->setDepthCompareOperation(CompareOperation::kLess);
	}

	// Do the fog apply
	if(rgraphCtx.m_currentSecondLevelCommandBufferIndex == rgraphCtx.m_secondLevelCommandBufferCount - 1u)
	{
		cmdb->bindShaderProgram(m_applyFog.m_grProg);

		// Bind all
		cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
		cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

		rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
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
		cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kSrcAlpha);

		drawQuad(cmdb);

		// Reset state
		cmdb->setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	// Forward shading last
	if(enableVrs)
	{
		cmdb->setVrsRate(VrsRate::k2x2);
	}

	m_r->getForwardShading().run(ctx, rgraphCtx);

	if(enableVrs)
	{
		// Restore
		cmdb->setVrsRate(VrsRate::k1x1);
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

	const TextureUsageBit readUsage = TextureUsageBit::kSampledFragment;

	// All
	if(enableVrs)
	{
		pass.newTextureDependency(sriRt, TextureUsageBit::kFramebufferShadingRate);
	}

	// Light shading
	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
	pass.newTextureDependency(m_r->getGBuffer().getColorRt(0), readUsage);
	pass.newTextureDependency(m_r->getGBuffer().getColorRt(1), readUsage);
	pass.newTextureDependency(m_r->getGBuffer().getColorRt(2), readUsage);
	pass.newTextureDependency(m_r->getGBuffer().getDepthRt(),
							  TextureUsageBit::kSampledFragment | TextureUsageBit::kFramebufferRead,
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	pass.newTextureDependency(m_r->getShadowMapping().getShadowmapRt(), readUsage);
	if(m_r->getRtShadowsEnabled())
	{
		pass.newTextureDependency(m_r->getRtShadows().getRt(), readUsage);
	}
	else
	{
		pass.newTextureDependency(m_r->getShadowmapsResolve().getRt(), readUsage);
	}
	pass.newBufferDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::kStorageFragmentRead);

	// Apply indirect
	pass.newTextureDependency(m_r->getIndirectDiffuse().getRt(), readUsage);
	pass.newTextureDependency(m_r->getDepthDownscale().getHiZRt(), readUsage);
	pass.newTextureDependency(m_r->getIndirectSpecular().getRt(), readUsage);

	// Fog
	pass.newTextureDependency(m_r->getVolumetricFog().getRt(), readUsage);

	// For forward shading
	m_r->getForwardShading().setDependencies(ctx, pass);
}

void LightShading::getDebugRenderTarget([[maybe_unused]] CString rtName,
										Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
										ShaderProgramPtr& optionalShaderProgram) const
{
	ANKI_ASSERT(rtName == "LightShading");
	handles[0] = m_runCtx.m_rt;
	optionalShaderProgram = m_visualizeRtGrProg;
}

} // end namespace anki
