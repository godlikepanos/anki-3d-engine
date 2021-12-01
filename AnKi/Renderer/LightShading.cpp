// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Renderer/Ssr.h>
#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

LightShading::LightShading(Renderer* r)
	: RendererObject(r)
{
}

LightShading::~LightShading()
{
}

Error LightShading::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing light stage");
	Error err = initLightShading(config);

	if(!err)
	{
		err = initApplyFog(config);
	}

	if(!err)
	{
		err = initApplyIndirect(config);
	}

	if(err)
	{
		ANKI_R_LOGE("Failed to init light stage");
	}

	return err;
}

Error LightShading::initLightShading(const ConfigSet& config)
{
	// Load shaders and programs
	ANKI_CHECK(getResourceManager().loadResource("Shaders/LightShading.ankiprog", m_lightShading.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_lightShading.m_prog);
	variantInitInfo.addConstant("TILE_COUNTS", m_r->getTileCounts());
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("TILE_SIZE", m_r->getTileSize());
	variantInitInfo.addConstant("IR_MIPMAP_COUNT", U32(m_r->getProbeReflections().getReflectionTextureMipmapCount()));
	const ShaderProgramResourceVariant* variant;

	variantInitInfo.addMutation("USE_SHADOW_LAYERS", 0);
	m_lightShading.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_lightShading.m_grProg[0] = variant->getProgram();

	variantInitInfo.addMutation("USE_SHADOW_LAYERS", 1);
	m_lightShading.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_lightShading.m_grProg[1] = variant->getProgram();

	// Create RT descr
	m_lightShading.m_rtDescr =
		m_r->create2DRenderTargetDescription(m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
											 LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "Light Shading");
	m_lightShading.m_rtDescr.bake();

	// Create FB descr
	m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
	m_lightShading.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::DONT_CARE;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_lightShading.m_fbDescr.bake();

	return Error::NONE;
}

Error LightShading::initApplyFog(const ConfigSet& config)
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

Error LightShading::initApplyIndirect(const ConfigSet& config)
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

		bindUniforms(cmdb, 0, 4, binning.m_reflectionProbesToken);
		rgraphCtx.bindColorTexture(0, 5, m_r->getProbeReflections().getReflectionRt());
		cmdb->bindTexture(0, 6, m_r->getProbeReflections().getIntegrationLut());

		bindStorage(cmdb, 0, 7, binning.m_clustersToken);

		cmdb->bindSampler(0, 8, m_r->getSamplers().m_nearestNearestClamp);
		cmdb->bindSampler(0, 9, m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindColorTexture(0, 10, m_r->getGBuffer().getColorRt(0));
		rgraphCtx.bindColorTexture(0, 11, m_r->getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 12, m_r->getGBuffer().getColorRt(2));
		rgraphCtx.bindTexture(0, 13, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		rgraphCtx.bindColorTexture(0, 14, m_r->getSsr().getRt());

		if(m_r->getRtShadowsEnabled())
		{
			rgraphCtx.bindColorTexture(0, 15, m_r->getRtShadows().getRt());
		}
		else
		{
			rgraphCtx.bindColorTexture(0, 16, m_r->getShadowmapsResolve().getRt());
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
		rgraphCtx.bindColorTexture(0, 3, m_r->getDepthDownscale().getHiZRt());
		rgraphCtx.bindTexture(0, 4, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		rgraphCtx.bindColorTexture(0, 5, m_r->getGBuffer().getColorRt(0));

		const Vec4 pc(2.0f / Vec2(m_r->getInternalResolution()), 0.0f, 0.0f);
		cmdb->setPushConstants(&pc, sizeof(pc));

		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);

		drawQuad(cmdb);

		// Restore state
		cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
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
	m_r->getForwardShading().run(ctx, rgraphCtx);
}

void LightShading::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Light&FW Shad.");

	pass.setWork(computeNumberOfSecondLevelCommandBuffers(ctx.m_renderQueue->m_forwardShadingRenderables.getSize()),
				 [this, &ctx](RenderPassWorkContext& rgraphCtx) {
					 run(ctx, rgraphCtx);
				 });
	pass.setFramebufferInfo(m_lightShading.m_fbDescr, {{m_runCtx.m_rt}}, {m_r->getGBuffer().getDepthRt()});

	const TextureUsageBit readUsage = TextureUsageBit::SAMPLED_FRAGMENT;

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

	// Refl & indirect
	pass.newDependency(RenderPassDependency(m_r->getSsr().getRt(), readUsage));
	pass.newDependency(RenderPassDependency(m_r->getProbeReflections().getReflectionRt(), readUsage));

	// Apply indirect
	pass.newDependency(RenderPassDependency(m_r->getIndirectDiffuse().getRt(), readUsage));
	pass.newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), readUsage));

	// Fog
	pass.newDependency(RenderPassDependency(m_r->getVolumetricFog().getRt(), readUsage));

	// For forward shading
	m_r->getForwardShading().setDependencies(ctx, pass);
}

} // end namespace anki
