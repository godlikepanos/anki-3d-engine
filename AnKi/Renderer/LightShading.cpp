// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ForwardShading.h>
#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/IndirectSpecular.h>
#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>

namespace anki {

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
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/LightShading.ankiprogbin", m_lightShading.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_lightShading.m_prog);
	variantInitInfo.addConstant("kTileCount", getRenderer().getTileCounts());
	variantInitInfo.addConstant("kZSplitCount", getRenderer().getZSplitCount());
	const ShaderProgramResourceVariant* variant;

	m_lightShading.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_lightShading.m_grProg.reset(&variant->getProgram());

	// Create RT descr
	const UVec2 internalResolution = getRenderer().getInternalResolution();
	m_lightShading.m_rtDescr =
		getRenderer().create2DRenderTargetDescription(internalResolution.x(), internalResolution.y(), getRenderer().getHdrFormat(), "Light Shading");
	m_lightShading.m_rtDescr.bake();

	// Create FB descr
	m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kLoad;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::kDontCare;
	m_lightShading.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;

	if(GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get())
	{
		m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth = getRenderer().getVrsSriGeneration().getSriTexelDimension();
		m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelHeight = getRenderer().getVrsSriGeneration().getSriTexelDimension();
	}

	m_lightShading.m_fbDescr.bake();

	// Debug visualization
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VisualizeHdrRenderTarget.ankiprogbin", m_visualizeRtProg, m_visualizeRtGrProg));

	return Error::kNone;
}

Error LightShading::initSkybox()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/LightShadingSkybox.ankiprogbin", m_skybox.m_prog));

	for(U32 method = 0; method < 2; ++method)
	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_skybox.m_prog);
		variantInitInfo.addMutation("METHOD", method);
		const ShaderProgramResourceVariant* variant;
		m_skybox.m_prog->getOrCreateVariant(variantInitInfo, variant);

		m_skybox.m_grProgs[method].reset(&variant->getProgram());
	}

	return Error::kNone;
}

Error LightShading::initApplyFog()
{
	// Load shaders and programs
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/LightShadingApplyFog.ankiprogbin", m_applyFog.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_applyFog.m_prog);
	variantInitInfo.addConstant("kZSplitCount", getRenderer().getZSplitCount());
	variantInitInfo.addConstant("kFinalZSplit", getRenderer().getVolumetricFog().getFinalClusterInZ());

	const ShaderProgramResourceVariant* variant;
	m_applyFog.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_applyFog.m_grProg.reset(&variant->getProgram());

	return Error::kNone;
}

Error LightShading::initApplyIndirect()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/LightShadingApplyIndirect.ankiprogbin", m_applyIndirect.m_prog, m_applyIndirect.m_grProg));
	return Error::kNone;
}

void LightShading::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(LightShading);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());

	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get();
	if(enableVrs)
	{
		// Just set some low value, the attachment will take over
		cmdb.setVrsRate(VrsRate::k1x1);
	}

	// Do light shading first
	{
		cmdb.bindShaderProgram(m_lightShading.m_grProg.get());
		cmdb.setDepthWrite(false);

		// Bind all
		cmdb.bindConstantBuffer(0, 0, getRenderer().getClusterBinning().getClusteredShadingConstants());
		cmdb.bindUavBuffer(0, 1, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		rgraphCtx.bindColorTexture(0, 2, getRenderer().getShadowMapping().getShadowmapRt());
		cmdb.bindUavBuffer(0, 3, getRenderer().getClusterBinning().getClustersBuffer());

		cmdb.bindSampler(0, 4, getRenderer().getSamplers().m_nearestNearestClamp.get());
		cmdb.bindSampler(0, 5, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindColorTexture(0, 6, getRenderer().getGBuffer().getColorRt(0));
		rgraphCtx.bindColorTexture(0, 7, getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 8, getRenderer().getGBuffer().getColorRt(2));
		rgraphCtx.bindTexture(0, 9, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 10, getRenderer().getShadowmapsResolve().getRt());

		// Draw
		drawQuad(cmdb);
	}

	// Apply indirect
	{
		cmdb.setDepthWrite(false);
		cmdb.bindShaderProgram(m_applyIndirect.m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindColorTexture(0, 2, getRenderer().getIndirectDiffuse().getRt());
		rgraphCtx.bindColorTexture(0, 3, getRenderer().getIndirectSpecular().getRt());
		rgraphCtx.bindColorTexture(0, 4, getRenderer().getDepthDownscale().getRt());
		rgraphCtx.bindTexture(0, 5, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 6, getRenderer().getGBuffer().getColorRt(0));
		rgraphCtx.bindColorTexture(0, 7, getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindColorTexture(0, 8, getRenderer().getGBuffer().getColorRt(2));
		cmdb.bindTexture(0, 9, &getRenderer().getProbeReflections().getIntegrationLut());

		cmdb.bindConstantBuffer(0, 10, getRenderer().getClusterBinning().getClusteredShadingConstants());

		const Vec4 pc(ctx.m_cameraNear, ctx.m_cameraFar, 0.0f, 0.0f);
		cmdb.setPushConstants(&pc, sizeof(pc));

		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kOne);

		drawQuad(cmdb);

		// Restore state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	// Skybox
	{
		cmdb.setDepthCompareOperation(CompareOperation::kEqual);

		const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();

		const Bool isSolidColor = (sky) ? sky->getSkyboxType() == SkyboxType::kSolidColor : true;

		if(isSolidColor)
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[0].get());

			const Vec4 color((sky) ? sky->getSolidColor() : Vec3(0.0f), 0.0);
			cmdb.setPushConstants(&color, sizeof(color));
		}
		else
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[1].get());

			class
			{
			public:
				Mat4 m_invertedViewProjectionJitter;
				Vec3 m_cameraPos;
				F32 m_padding = 0.0;
			} pc;

			pc.m_invertedViewProjectionJitter = ctx.m_matrices.m_invertedViewProjectionJitter;
			pc.m_cameraPos = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();

			cmdb.setPushConstants(&pc, sizeof(pc));

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get());
			cmdb.bindTexture(0, 1, &sky->getImageResource().getTextureView());
		}

		drawQuad(cmdb);

		// Restore state
		cmdb.setDepthCompareOperation(CompareOperation::kLess);
	}

	// Apply the fog
	{
		cmdb.bindShaderProgram(m_applyFog.m_grProg.get());

		// Bind all
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());

		rgraphCtx.bindTexture(0, 2, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 3, getRenderer().getVolumetricFog().getRt());

		class PushConsts
		{
		public:
			Vec2 m_padding;
			F32 m_near;
			F32 m_far;
		} regs;
		regs.m_near = ctx.m_cameraNear;
		regs.m_far = ctx.m_cameraFar;

		cmdb.setPushConstants(&regs, sizeof(regs));

		// finalPixelColor = pixelWithoutFog * transmitance + inScattering (see the shader)
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kSrcAlpha);

		drawQuad(cmdb);

		// Reset state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);
	}

	// Forward shading last
	{
		if(enableVrs)
		{
			cmdb.setVrsRate(VrsRate::k2x2);
		}

		getRenderer().getForwardShading().run(ctx, rgraphCtx);

		if(enableVrs)
		{
			// Restore
			cmdb.setVrsRate(VrsRate::k1x1);
		}
	}
}

void LightShading::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(LightShading);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get();
	const Bool fbDescrHasVrs = m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth > 0;

	if(enableVrs != fbDescrHasVrs)
	{
		// Re-bake the FB descriptor if the VRS state has changed

		if(enableVrs)
		{
			m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelWidth = getRenderer().getVrsSriGeneration().getSriTexelDimension();
			m_lightShading.m_fbDescr.m_shadingRateAttachmentTexelHeight = getRenderer().getVrsSriGeneration().getSriTexelDimension();
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
		sriRt = getRenderer().getVrsSriGeneration().getSriRt();
	}

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Light&FW Shad");

	pass.setWork(1, [this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});
	pass.setFramebufferInfo(m_lightShading.m_fbDescr, {m_runCtx.m_rt}, getRenderer().getGBuffer().getDepthRt(), sriRt);

	const TextureUsageBit readUsage = TextureUsageBit::kSampledFragment;

	// All
	if(enableVrs)
	{
		pass.newTextureDependency(sriRt, TextureUsageBit::kFramebufferShadingRate);
	}

	// Light shading
	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
	pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(0), readUsage);
	pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), readUsage);
	pass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
	pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment | TextureUsageBit::kFramebufferRead,
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), readUsage);
	pass.newTextureDependency(getRenderer().getShadowmapsResolve().getRt(), readUsage);
	pass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kUavFragmentRead);
	pass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
							 BufferUsageBit::kUavFragmentRead);

	// Apply indirect
	pass.newTextureDependency(getRenderer().getIndirectDiffuse().getRt(), readUsage);
	pass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage);
	pass.newTextureDependency(getRenderer().getIndirectSpecular().getRt(), readUsage);

	// Fog
	pass.newTextureDependency(getRenderer().getVolumetricFog().getRt(), readUsage);

	// For forward shading
	getRenderer().getForwardShading().setDependencies(pass);
}

void LightShading::getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
										ShaderProgramPtr& optionalShaderProgram) const
{
	ANKI_ASSERT(rtName == "LightShading");
	handles[0] = m_runCtx.m_rt;
	optionalShaderProgram = m_visualizeRtGrProg;
}

} // end namespace anki
