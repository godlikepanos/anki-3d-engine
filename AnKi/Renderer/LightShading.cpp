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
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/VrsSriGeneration.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/Ssao.h>
#include <AnKi/Renderer/Reflections.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>

namespace anki {

Error LightShading::init()
{
	{
		// Load shaders and programs
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/LightShading.ankiprogbin",
									 {{"INDIRECT_DIFFUSE_TEX", getRenderer().isIndirectDiffuseClipmapsEnabled()}}, m_lightShading.m_prog,
									 m_lightShading.m_grProg));

		// Create RT descr
		const UVec2 internalResolution = getRenderer().getInternalResolution();
		m_lightShading.m_rtDescr =
			getRenderer().create2DRenderTargetDescription(internalResolution.x, internalResolution.y, getRenderer().getHdrFormat(), "Light Shading");
		m_lightShading.m_rtDescr.bake();
	}

	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/LightShadingSkybox.ankiprogbin", m_skybox.m_prog));

		for(MutatorValue method = 0; method < 3; ++method)
		{
			ANKI_CHECK(loadShaderProgram("ShaderBinaries/LightShadingSkybox.ankiprogbin", {{"METHOD", method}}, m_skybox.m_prog,
										 m_skybox.m_grProgs[method]));
		}
	}

	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/LightShadingApplyFog.ankiprogbin", m_applyFog.m_prog, m_applyFog.m_grProg));
	}

	return Error::kNone;
}

void LightShading::run(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(LightShading);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);

	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_cvarGrVrs;
	if(enableVrs)
	{
		// Just set some low value, the attachment will take over
		cmdb.setVrsRate(VrsRate::k1x1);
	}

	// Do light shading first
	{
		cmdb.pushDebugMarker("LightShading", Vec3(0.0f, 1.0f, 1.0f));

		cmdb.bindShaderProgram(m_lightShading.m_grProg.get());
		cmdb.setDepthWrite(false);

		// Bind all
		cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);
		cmdb.bindSrv(0, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		if(getRenderer().isIndirectDiffuseClipmapsEnabled())
		{
			rgraphCtx.bindSrv(1, 0, getIndirectDiffuseClipmaps().getRts().m_appliedIrradiance);
		}
		else
		{
			cmdb.bindSrv(1, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
		}
		cmdb.bindSrv(2, 0, getClusterBinning().getClustersBuffer());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindSrv(3, 0, getGBuffer().getColorRt(0));
		rgraphCtx.bindSrv(4, 0, getGBuffer().getColorRt(1));
		rgraphCtx.bindSrv(5, 0, getGBuffer().getColorRt(2));
		rgraphCtx.bindSrv(6, 0, getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(7, 0, getShadowmapsResolve().getRt());
		rgraphCtx.bindSrv(8, 0, getSsao().getRt());
		rgraphCtx.bindSrv(9, 0, getReflections().getRt());
		cmdb.bindSrv(10, 0, TextureView(&getRenderer().getProbeReflections().getIntegrationLut(), TextureSubresourceDesc::all()));

		// Draw
		drawQuad(cmdb);

		cmdb.popDebugMarker();
	}

	// Skybox
	{
		cmdb.pushDebugMarker("Skybox", Vec3(0.0f, 1.0f, 1.0f));

		cmdb.setDepthCompareOperation(CompareOperation::kEqual);

		const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
		if(getGeneratedSky().isEnabled())
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[2].get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, getRenderer().getGeneratedSky().getSkyLutRt());
			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);
		}
		else if(sky && sky->getSkyboxComponentType() == SkyboxComponentType::kImage2D)
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

			pc.m_invertedViewProjectionJitterMat = getRenderingContext().m_matrices.m_invertedViewProjectionJitter;
			pc.m_cameraPos = getRenderingContext().m_matrices.m_cameraTransform.getTranslationPart().xyz;
			pc.m_scale = sky->getSkyImageColorScale();
			pc.m_bias = sky->getSkyImageColorBias();

			cmdb.setFastConstants(&pc, sizeof(pc));

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get());
			cmdb.bindSrv(0, 0, TextureView(&sky->getSkyTexture(), TextureSubresourceDesc::all()));
		}
		else
		{
			cmdb.bindShaderProgram(m_skybox.m_grProgs[0].get());

			const Vec4 color((sky) ? sky->getSkySolidColor() : Vec3(0.0f), 0.0);
			cmdb.setFastConstants(&color, sizeof(color));
		}

		drawQuad(cmdb);

		// Restore state
		cmdb.setDepthCompareOperation(CompareOperation::kLess);

		cmdb.popDebugMarker();
	}

	// Apply the fog
	{
		cmdb.pushDebugMarker("LightApplyFog", Vec3(0.0f, 1.0f, 1.0f));

		cmdb.bindShaderProgram(m_applyFog.m_grProg.get());

		// Bind all
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(1, 0, getRenderer().getVolumetricFog().getRt());

		cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

		// finalPixelColor = pixelWithoutFog * transmitance + inScattering (see the shader)
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kSrcAlpha);

		drawQuad(cmdb);

		// Reset state
		cmdb.setBlendFactors(0, BlendFactor::kOne, BlendFactor::kZero);

		cmdb.popDebugMarker();
	}

	// Forward shading last
	{
		cmdb.pushDebugMarker("ForwardShading", Vec3(0.0f, 1.0f, 1.0f));

		if(enableVrs)
		{
			cmdb.setVrsRate(VrsRate::k2x2);
		}

		getRenderer().getForwardShading().run(rgraphCtx);

		if(enableVrs)
		{
			// Restore
			cmdb.setVrsRate(VrsRate::k1x1);
		}

		cmdb.popDebugMarker();
	}
}

void LightShading::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(LightShading);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_cvarGrVrs;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

	RenderTargetHandle sriRt;
	if(enableVrs)
	{
		sriRt = getRenderer().getVrsSriGeneration().getSriRt();
	}

	// Create pass
	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Light&FW Shad");

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		run(rgraphCtx);
	});

	GraphicsRenderPassTargetDesc colorRt(m_runCtx.m_rt);
	GraphicsRenderPassTargetDesc depthRt(getGBuffer().getDepthRt());
	depthRt.m_loadOperation = RenderTargetLoadOperation::kLoad;
	depthRt.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
	pass.setRenderpassInfo({colorRt}, &depthRt, (enableVrs) ? &sriRt : nullptr,
						   (enableVrs) ? getRenderer().getVrsSriGeneration().getSriTexelDimension() : 0,
						   (enableVrs) ? getRenderer().getVrsSriGeneration().getSriTexelDimension() : 0);

	const TextureUsageBit readUsage = TextureUsageBit::kSrvPixel;

	// All
	if(enableVrs)
	{
		pass.newTextureDependency(sriRt, TextureUsageBit::kShadingRate);
	}

	// Light shading
	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kRtvDsvWrite);
	pass.newTextureDependency(getGBuffer().getColorRt(0), readUsage);
	pass.newTextureDependency(getGBuffer().getColorRt(1), readUsage);
	pass.newTextureDependency(getGBuffer().getColorRt(2), readUsage);
	pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel | TextureUsageBit::kRtvDsvRead);
	pass.newTextureDependency(getShadowmapsResolve().getRt(), readUsage);
	pass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvPixel);
	pass.newTextureDependency(getSsao().getRt(), readUsage);
	pass.newTextureDependency(getReflections().getRt(), readUsage);

	if(getRenderer().isIndirectDiffuseClipmapsEnabled())
	{
		pass.newTextureDependency(getIndirectDiffuseClipmaps().getRts().m_appliedIrradiance, TextureUsageBit::kSrvPixel);
	}

	// Fog
	pass.newTextureDependency(getRenderer().getVolumetricFog().getRt(), readUsage);

	// Sky
	if(getRenderer().getGeneratedSky().isEnabled())
	{
		pass.newTextureDependency(getRenderer().getGeneratedSky().getSkyLutRt(), readUsage);
	}

	// For forward shading
	getRenderer().getForwardShading().setDependencies(pass);
}

} // end namespace anki
