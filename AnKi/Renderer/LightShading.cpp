// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/Ssr.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>

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

	if(err)
	{
		ANKI_R_LOGE("Failed to init light stage");
	}

	return err;
}

Error LightShading::initLightShading()
{
	// Load shaders and programs
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/LightShading.ankiprogbin", m_lightShading.m_prog, m_lightShading.m_grProg));

	// Create RT descr
	const UVec2 internalResolution = getRenderer().getInternalResolution();
	m_lightShading.m_rtDescr =
		getRenderer().create2DRenderTargetDescription(internalResolution.x(), internalResolution.y(), getRenderer().getHdrFormat(), "Light Shading");
	m_lightShading.m_rtDescr.bake();

	// Debug visualization
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VisualizeHdrRenderTarget.ankiprogbin", m_visualizeRtProg, m_visualizeRtGrProg));

	return Error::kNone;
}

Error LightShading::initSkybox()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/LightShadingSkybox.ankiprogbin", m_skybox.m_prog));

	for(MutatorValue method = 0; method < 3; ++method)
	{
		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/LightShadingSkybox.ankiprogbin", {{"METHOD", method}}, m_skybox.m_prog, m_skybox.m_grProgs[method]));
	}

	return Error::kNone;
}

Error LightShading::initApplyFog()
{
	// Load shaders and programs
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/LightShadingApplyFog.ankiprogbin", m_applyFog.m_prog, m_applyFog.m_grProg));

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
		cmdb.bindUniformBuffer(ANKI_REG(b0), ctx.m_globalRenderingUniformsBuffer);
		cmdb.bindStorageBuffer(ANKI_REG(t0), getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		cmdb.bindStorageBuffer(ANKI_REG(t1), getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		cmdb.bindStorageBuffer(ANKI_REG(t2),
							   getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
		cmdb.bindStorageBuffer(ANKI_REG(t3),
							   getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kReflectionProbe));
		cmdb.bindStorageBuffer(ANKI_REG(t4), getRenderer().getClusterBinning().getClustersBuffer());

		cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_nearestNearestClamp.get());
		cmdb.bindSampler(ANKI_REG(s1), getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindTexture(ANKI_REG(t5), getRenderer().getGBuffer().getColorRt(0));
		rgraphCtx.bindTexture(ANKI_REG(t6), getRenderer().getGBuffer().getColorRt(1));
		rgraphCtx.bindTexture(ANKI_REG(t7), getRenderer().getGBuffer().getColorRt(2));
		rgraphCtx.bindTexture(ANKI_REG(t8), getRenderer().getGBuffer().getDepthRt());
		rgraphCtx.bindTexture(ANKI_REG(t9), getRenderer().getShadowmapsResolve().getRt());
		rgraphCtx.bindTexture(ANKI_REG(t10), getRenderer().getSsao().getRt());
		rgraphCtx.bindTexture(ANKI_REG(t11), getRenderer().getSsr().getRt());
		cmdb.bindTexture(ANKI_REG(t12), TextureView(&getRenderer().getProbeReflections().getIntegrationLut(), TextureSubresourceDescriptor::all()));

		// Draw
		drawQuad(cmdb);
	}

	// Skybox
	{
		cmdb.setDepthCompareOperation(CompareOperation::kEqual);

		const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
		const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();

		const Bool isSolidColor =
			(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));

		if(isSolidColor)
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[0].get());

			const Vec4 color((sky) ? sky->getSolidColor() : Vec3(0.0f), 0.0);
			cmdb.setPushConstants(&color, sizeof(color));
		}
		else if(sky->getSkyboxType() == SkyboxType::kImage2D)
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[1].get());

			class
			{
			public:
				Mat4 m_invertedViewProjectionJitterMat;

				Vec3 m_cameraPos;
				F32 m_padding;

				Vec3 m_scale;
				F32 m_padding1;

				Vec3 m_bias;
				F32 m_padding2;
			} pc;

			pc.m_invertedViewProjectionJitterMat = ctx.m_matrices.m_invertedViewProjectionJitter;
			pc.m_cameraPos = ctx.m_matrices.m_cameraTransform.getTranslationPart().xyz();
			pc.m_scale = sky->getImageScale();
			pc.m_bias = sky->getImageBias();

			cmdb.setPushConstants(&pc, sizeof(pc));

			cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get());
			cmdb.bindTexture(ANKI_REG(t0), TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDescriptor::all()));
		}
		else
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[2].get());

			cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindTexture(ANKI_REG(t0), getRenderer().getSky().getSkyLutRt());
			cmdb.bindUniformBuffer(ANKI_REG(b0), ctx.m_globalRenderingUniformsBuffer);
		}

		drawQuad(cmdb);

		// Restore state
		cmdb.setDepthCompareOperation(CompareOperation::kLess);
	}

	// Apply the fog
	{
		cmdb.bindShaderProgram(m_applyFog.m_grProg.get());

		// Bind all
		cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_nearestNearestClamp.get());
		cmdb.bindSampler(ANKI_REG(s1), getRenderer().getSamplers().m_trilinearClamp.get());

		rgraphCtx.bindTexture(ANKI_REG(t0), getRenderer().getGBuffer().getDepthRt());
		rgraphCtx.bindTexture(ANKI_REG(t1), getRenderer().getVolumetricFog().getRt());

		class PushConsts
		{
		public:
			F32 m_zSplitCount;
			F32 m_finalZSplit;
			F32 m_near;
			F32 m_far;
		} regs;
		regs.m_zSplitCount = F32(getRenderer().getZSplitCount());
		regs.m_finalZSplit = F32(getRenderer().getVolumetricFog().getFinalClusterInZ());
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

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

	RenderTargetHandle sriRt;
	if(enableVrs)
	{
		sriRt = getRenderer().getVrsSriGeneration().getSriRt();
	}

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Light&FW Shad");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});

	RenderTargetInfo colorRt(m_runCtx.m_rt);
	RenderTargetInfo depthRt(getRenderer().getGBuffer().getDepthRt());
	depthRt.m_loadOperation = RenderTargetLoadOperation::kLoad;
	depthRt.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	pass.setRenderpassInfo({colorRt}, &depthRt, 0, 0, kMaxU32, kMaxU32, (enableVrs) ? &sriRt : nullptr,
						   (enableVrs) ? getRenderer().getVrsSriGeneration().getSriTexelDimension() : 0,
						   (enableVrs) ? getRenderer().getVrsSriGeneration().getSriTexelDimension() : 0);

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
	pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment | TextureUsageBit::kFramebufferRead);
	pass.newTextureDependency(getRenderer().getShadowmapsResolve().getRt(), readUsage);
	pass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kStorageFragmentRead);
	pass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
							 BufferUsageBit::kStorageFragmentRead);
	pass.newTextureDependency(getRenderer().getSsao().getRt(), readUsage);
	pass.newTextureDependency(getRenderer().getSsr().getRt(), readUsage);

	// Fog
	pass.newTextureDependency(getRenderer().getVolumetricFog().getRt(), readUsage);

	// Sky
	if(getRenderer().getSky().isEnabled())
	{
		pass.newTextureDependency(getRenderer().getSky().getSkyLutRt(), readUsage);
	}

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
