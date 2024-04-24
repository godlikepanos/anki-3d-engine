// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Shaders/Include/TraditionalDeferredShadingTypes.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

static NumericCVar<U32> g_probeReflectionIrradianceResolutionCVar(CVarSubsystem::kRenderer, "ProbeReflectionIrradianceResolution", 16, 4, 2048,
																  "Reflection probe irradiance resolution");
static NumericCVar<U32> g_probeReflectionShadowMapResolutionCVar(CVarSubsystem::kRenderer, "ProbeReflectionShadowMapResolution", 64, 4, 2048,
																 "Reflection probe shadow resolution");
static StatCounter g_probeReflectionCountStatVar(StatCategory::kRenderer, "Reflection probes rendered", StatFlag::kMainThreadUpdates);

Error ProbeReflections::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize image reflections");
	}

	return err;
}

Error ProbeReflections::initInternal()
{
	// Init cache entries
	ANKI_CHECK(initGBuffer());
	ANKI_CHECK(initLightShading());
	ANKI_CHECK(initIrradiance());
	ANKI_CHECK(initIrradianceToRefl());
	ANKI_CHECK(initShadowMapping());

	// Load split sum integration LUT
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/IblDfg.png", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::kLinear;
	sinit.m_mipmapFilter = SamplingFilter::kBase;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_addressing = SamplingAddressing::kClamp;
	m_integrationLutSampler = GrManager::getSingleton().newSampler(sinit);

	return Error::kNone;
}

Error ProbeReflections::initGBuffer()
{
	m_gbuffer.m_tileSize = g_reflectionProbeResolutionCVar.get();

	// Create RT descriptions
	{
		RenderTargetDescription texinit = getRenderer().create2DRenderTargetDescription(m_gbuffer.m_tileSize, m_gbuffer.m_tileSize,
																						kGBufferColorRenderTargetFormats[0], "CubeRefl GBuffer");

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			texinit.m_type = TextureType::kCube;
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(RendererString().sprintf("CubeRefl GBuff Col #%u", i));
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = getRenderer().getDepthNoStencilFormat();
		texinit.m_type = TextureType::k2D;
		texinit.setName("CubeRefl GBuff Depth");
		m_gbuffer.m_depthRtDescr = texinit;
		m_gbuffer.m_depthRtDescr.bake();
	}

	return Error::kNone;
}

Error ProbeReflections::initLightShading()
{
	m_lightShading.m_tileSize = g_reflectionProbeResolutionCVar.get();
	m_lightShading.m_mipCount = computeMaxMipmapCount2d(m_lightShading.m_tileSize, m_lightShading.m_tileSize, 8);

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

Error ProbeReflections::initIrradiance()
{
	m_irradiance.m_workgroupSize = g_probeReflectionIrradianceResolutionCVar.get();

	// Create prog
	{
		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/IrradianceDice.ankiprogbin",
							  {{"THREDGROUP_SIZE_SQRT", MutatorValue(m_irradiance.m_workgroupSize)}, {"STORE_LOCATION", 1}, {"SECOND_BOUNCE", 0}},
							  m_irradiance.m_prog, m_irradiance.m_grProg));
	}

	// Create buff
	{
		BufferInitInfo init;
		init.m_usage = BufferUsageBit::kAllStorage;
		init.m_size = 6 * sizeof(Vec4);
		m_irradiance.m_diceValuesBuff = GrManager::getSingleton().newBuffer(init);
	}

	return Error::kNone;
}

Error ProbeReflections::initIrradianceToRefl()
{
	// Create program
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/ApplyIrradianceToReflection.ankiprogbin", m_irradianceToRefl.m_prog, m_irradianceToRefl.m_grProg));
	return Error::kNone;
}

Error ProbeReflections::initShadowMapping()
{
	const U32 resolution = g_probeReflectionShadowMapResolutionCVar.get();
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		getRenderer().create2DRenderTargetDescription(resolution, resolution, getRenderer().getDepthNoStencilFormat(), "CubeRefl SM");
	m_shadowMapping.m_rtDescr.bake();

	return Error::kNone;
}

void ProbeReflections::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

	// Iterate the visible probes to find a candidate for update
	WeakArray<ReflectionProbeComponent*> visibleProbes =
		getRenderer().getPrimaryNonRenderableVisibility().getInterestingVisibleComponents().m_reflectionProbes;
	ReflectionProbeComponent* probeToRefresh = nullptr;
	for(ReflectionProbeComponent* probe : visibleProbes)
	{
		if(probe->getEnvironmentTextureNeedsRefresh())
		{
			probeToRefresh = probe;
			break;
		}
	}

	if(probeToRefresh == nullptr || ResourceManager::getSingleton().getAsyncLoader().getTasksInFlightCount() != 0) [[likely]]
	{
		// Nothing to update or can't update right now, early exit
		m_runCtx = {};
		return;
	}

	g_probeReflectionCountStatVar.increment(1);
	probeToRefresh->setEnvironmentTextureAsRefreshed();

	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
	const Bool doShadows = dirLightc && dirLightc->getShadowEnabled();

	// Create render targets now to save memory
	const RenderTargetHandle probeTexture = rgraph.importRenderTarget(&probeToRefresh->getReflectionTexture(), TextureUsageBit::kNone);
	m_runCtx.m_probeTex = probeTexture;
	const BufferHandle irradianceDiceValuesBuffHandle = rgraph.importBuffer(BufferView(m_irradiance.m_diceValuesBuff.get()), BufferUsageBit::kNone);
	const RenderTargetHandle gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);
	const RenderTargetHandle shadowMapRt = (doShadows) ? rgraph.newRenderTarget(m_shadowMapping.m_rtDescr) : RenderTargetHandle();

	Array<RenderTargetHandle, kGBufferColorRenderTargetCount> gbufferColorRts;
	for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
	{
		gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
	}

	for(U8 f = 0; f < 6; ++f)
	{
		// GBuffer visibility
		GpuVisibilityOutput visOut;
		GpuMeshletVisibilityOutput meshletVisOut;
		Frustum frustum;
		{
			frustum.setPerspective(kClusterObjectFrustumNearPlane, probeToRefresh->getRenderRadius(), kPi / 2.0f, kPi / 2.0f);
			frustum.setWorldTransform(
				Transform(probeToRefresh->getWorldPosition().xyz0(), Frustum::getOmnidirectionalFrustumRotations()[f], Vec4(1.0f, 1.0f, 1.0f, 0.0f)));
			frustum.update();

			Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = generateTempPassName("Cube refl: GBuffer", f);
			visIn.m_technique = RenderingTechnique::kGBuffer;
			visIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
			visIn.m_lodReferencePoint = probeToRefresh->getWorldPosition();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewportSize = UVec2(m_gbuffer.m_tileSize);

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

			if(getRenderer().runSoftwareMeshletRendering())
			{
				GpuMeshletVisibilityInput meshIn;
				meshIn.m_passesName = "Cube refl: GBuffer";
				meshIn.m_technique = RenderingTechnique::kGBuffer;
				meshIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
				meshIn.m_cameraTransform = frustum.getViewMatrix().getInverseTransformation();
				meshIn.m_viewportSize = UVec2(m_gbuffer.m_tileSize);
				meshIn.m_rgraph = &rgraph;
				meshIn.fillBuffers(visOut);

				getRenderer().getGpuVisibility().populateRenderGraph(meshIn, meshletVisOut);
			}
		}

		// GBuffer pass
		{
			// Create pass
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: GBuffer", f));

			Array<RenderTargetInfo, kGBufferColorRenderTargetCount> colorRtis;
			for(U j = 0; j < kGBufferColorRenderTargetCount; ++j)
			{
				colorRtis[j].m_loadOperation = RenderTargetLoadOperation::kClear;
				colorRtis[j].m_surface.m_face = f;
				colorRtis[j].m_handle = gbufferColorRts[j];
			}
			RenderTargetInfo depthRti(gbufferDepthRt);
			depthRti.m_aspect = DepthStencilAspectBit::kDepth;
			depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
			depthRti.m_clearValue.m_depthStencil.m_depth = 1.0f;

			pass.setRenderpassInfo(colorRtis, &depthRti);

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kFramebufferWrite, TextureSurfaceInfo(0, f, 0));
			}

			pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kAllFramebuffer, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			pass.newBufferDependency((meshletVisOut.isFilled()) ? meshletVisOut.m_dependency : visOut.m_dependency, BufferUsageBit::kIndirectDraw);

			pass.setWork([this, visOut, meshletVisOut, viewProjMat = frustum.getViewProjectionMatrix(),
						  viewMat = frustum.getViewMatrix()](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ProbeReflections);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.setViewport(0, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

				RenderableDrawerArguments args;
				args.m_viewMatrix = viewMat;
				args.m_cameraTransform = viewMat.getInverseTransformation();
				args.m_viewProjectionMatrix = viewProjMat;
				args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care about prev mats
				args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
				args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
				args.m_viewport = UVec4(0, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
				args.fill(visOut);

				if(meshletVisOut.isFilled())
				{
					args.fill(meshletVisOut);
				}

				getRenderer().getSceneDrawer().drawMdi(args, cmdb);
			});
		}

		// Shadow visibility. Optional
		GpuVisibilityOutput shadowVisOut;
		GpuMeshletVisibilityOutput shadowMeshletVisOut;
		Mat4 cascadeViewProjMat;
		Mat3x4 cascadeViewMat;
		Mat4 cascadeProjMat;
		if(doShadows)
		{
			constexpr U32 kCascadeCount = 1;
			dirLightc->computeCascadeFrustums(frustum, Array<F32, kCascadeCount>{probeToRefresh->getShadowsRenderRadius()},
											  WeakArray<Mat4>(&cascadeProjMat, kCascadeCount), WeakArray<Mat3x4>(&cascadeViewMat, kCascadeCount));

			cascadeViewProjMat = cascadeProjMat * Mat4(cascadeViewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));

			Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = generateTempPassName("Cube refl: Shadows", f);
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_viewProjectionMatrix = cascadeViewProjMat;
			visIn.m_lodReferencePoint = probeToRefresh->getWorldPosition();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewportSize = UVec2(m_shadowMapping.m_rtDescr.m_height);

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, shadowVisOut);

			if(getRenderer().runSoftwareMeshletRendering())
			{
				GpuMeshletVisibilityInput meshIn;
				meshIn.m_passesName = "Cube refl: Shadows";
				meshIn.m_technique = RenderingTechnique::kDepth;
				meshIn.m_viewProjectionMatrix = cascadeViewProjMat;
				meshIn.m_cameraTransform = cascadeViewMat.getInverseTransformation();
				meshIn.m_viewportSize = UVec2(m_shadowMapping.m_rtDescr.m_height);
				meshIn.m_rgraph = &rgraph;
				meshIn.fillBuffers(shadowVisOut);

				getRenderer().getGpuVisibility().populateRenderGraph(meshIn, shadowMeshletVisOut);
			}
		}

		// Shadows. Optional
		if(doShadows)
		{
			// Pass
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: Shadows", f));

			RenderTargetInfo depthRti(shadowMapRt);
			depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
			depthRti.m_clearValue.m_depthStencil.m_depth = 1.0f;
			depthRti.m_aspect = DepthStencilAspectBit::kDepth;

			pass.setRenderpassInfo({}, &depthRti);

			pass.newTextureDependency(shadowMapRt, TextureUsageBit::kAllFramebuffer, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			pass.newBufferDependency((shadowMeshletVisOut.isFilled()) ? shadowMeshletVisOut.m_dependency : shadowVisOut.m_dependency,
									 BufferUsageBit::kIndirectDraw);

			pass.setWork([this, shadowVisOut, shadowMeshletVisOut, cascadeViewProjMat, cascadeViewMat](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
				cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

				const U32 rez = m_shadowMapping.m_rtDescr.m_height;
				cmdb.setViewport(0, 0, rez, rez);

				RenderableDrawerArguments args;
				args.m_viewMatrix = cascadeViewMat;
				args.m_cameraTransform = cascadeViewMat.getInverseTransformation();
				args.m_viewProjectionMatrix = cascadeViewProjMat;
				args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
				args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAniso.get();
				args.m_renderingTechinuqe = RenderingTechnique::kDepth;
				args.m_viewport = UVec4(0, 0, rez, rez);
				args.fill(shadowVisOut);

				if(shadowMeshletVisOut.isFilled())
				{
					args.fill(shadowMeshletVisOut);
				}

				getRenderer().getSceneDrawer().drawMdi(args, cmdb);
			});
		}

		// Light visibility
		GpuVisibilityNonRenderablesOutput lightVis;
		{
			GpuVisibilityNonRenderablesInput in;
			in.m_passesName = generateTempPassName("Cube refl: Light visibility", f);
			in.m_objectType = GpuSceneNonRenderableObjectType::kLight;
			in.m_viewProjectionMat = frustum.getViewProjectionMatrix();
			in.m_rgraph = &rgraph;
			getRenderer().getGpuVisibilityNonRenderables().populateRenderGraph(in, lightVis);
		}

		// Light shading pass
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: light shading", f));

			RenderTargetInfo colorRti(probeTexture);
			colorRti.m_surface.m_face = f;
			colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
			pass.setRenderpassInfo({colorRti});

			pass.newBufferDependency(lightVis.m_visiblesBufferHandle, BufferUsageBit::kStorageFragmentRead);
			pass.newTextureDependency(probeTexture, TextureUsageBit::kFramebufferWrite, TextureSubresourceInfo(TextureSurfaceInfo(0, f, 0)));

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSampledFragment, TextureSurfaceInfo(0, f, 0));
			}
			pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kSampledFragment, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			if(shadowMapRt.isValid())
			{
				pass.newTextureDependency(shadowMapRt, TextureUsageBit::kSampledFragment);
			}

			if(getRenderer().getSky().isEnabled())
			{
				pass.newTextureDependency(getRenderer().getSky().getSkyLutRt(), TextureUsageBit::kSampledFragment);
			}

			pass.setWork([this, visResult = lightVis.m_visiblesBuffer, viewProjMat = frustum.getViewProjectionMatrix(),
						  cascadeViewProjMat = cascadeViewProjMat, probeToRefresh, gbufferColorRts, gbufferDepthRt, shadowMapRt, faceIdx = f,
						  &rctx](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

				TraditionalDeferredLightShadingDrawInfo dsInfo;
				dsInfo.m_viewProjectionMatrix = viewProjMat;
				dsInfo.m_invViewProjectionMatrix = viewProjMat.getInverse();
				dsInfo.m_cameraPosWSpace = probeToRefresh->getWorldPosition().xyz1();
				dsInfo.m_viewport = UVec4(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
				dsInfo.m_effectiveShadowDistance = probeToRefresh->getShadowsRenderRadius();

				const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
				dsInfo.m_dirLightMatrix = biasMat4 * cascadeViewProjMat;

				dsInfo.m_visibleLightsBuffer = visResult;
				dsInfo.m_gbufferRenderTargets[0] = gbufferColorRts[0];
				dsInfo.m_gbufferRenderTargetSubresourceInfos[0].m_firstFace = faceIdx;
				dsInfo.m_gbufferRenderTargets[1] = gbufferColorRts[1];
				dsInfo.m_gbufferRenderTargetSubresourceInfos[1].m_firstFace = faceIdx;
				dsInfo.m_gbufferRenderTargets[2] = gbufferColorRts[2];
				dsInfo.m_gbufferRenderTargetSubresourceInfos[2].m_firstFace = faceIdx;
				dsInfo.m_gbufferDepthRenderTarget = gbufferDepthRt;
				if(shadowMapRt.isValid())
				{
					dsInfo.m_directionalLightShadowmapRenderTarget = shadowMapRt;
				}
				dsInfo.m_skyLutRenderTarget = (getRenderer().getSky().isEnabled()) ? getRenderer().getSky().getSkyLutRt() : RenderTargetHandle();
				dsInfo.m_globalRendererConsts = rctx.m_globalRenderingUniformsBuffer;
				dsInfo.m_renderpassContext = &rgraphCtx;

				m_lightShading.m_deferred.drawLights(dsInfo);
			});
		}
	} // For 6 faces

	// Compute Irradiance
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cube refl: Irradiance");

		pass.newTextureDependency(probeTexture, TextureUsageBit::kSampledCompute);

		pass.newBufferDependency(irradianceDiceValuesBuffHandle, BufferUsageBit::kStorageComputeWrite);

		pass.setWork([this, probeTexture](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_irradiance.m_grProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

			rgraphCtx.bindColorTexture(0, 1, probeTexture);

			cmdb.bindStorageBuffer(0, 3, BufferView(m_irradiance.m_diceValuesBuff.get()));

			cmdb.dispatchCompute(1, 1, 1);
		});
	}

	// Append irradiance back to refl cubemap
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cube refl: Apply indirect");

		for(U i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
		{
			pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSampledCompute);
		}

		pass.newTextureDependency(probeTexture, TextureUsageBit::kStorageComputeRead | TextureUsageBit::kStorageComputeWrite);

		pass.newBufferDependency(irradianceDiceValuesBuffHandle, BufferUsageBit::kStorageComputeRead);

		pass.setWork([this, gbufferColorRts, probeTexture](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_irradianceToRefl.m_grProg.get());

			// Bind resources
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

			for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
			{
				rgraphCtx.bindColorTexture(0, 1, gbufferColorRts[i], i);
			}

			cmdb.bindStorageBuffer(0, 2, BufferView(m_irradiance.m_diceValuesBuff.get()));

			for(U8 f = 0; f < 6; ++f)
			{
				TextureSubresourceInfo subresource;
				subresource.m_faceCount = 1;
				subresource.m_firstFace = f;
				rgraphCtx.bindStorageTexture(0, 3, probeTexture, subresource, f);
			}

			dispatchPPCompute(cmdb, 8, 8, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
		});
	}

	// Mipmapping "passes"
	{
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: Gen mips", faceIdx));

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, faceIdx, 0));
			subresource.m_mipmapCount = m_lightShading.m_mipCount;
			pass.newTextureDependency(probeTexture, TextureUsageBit::kGenerateMipmaps, subresource);

			pass.setWork([this, faceIdx, probeTexture](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

				TextureSubresourceInfo subresource(TextureSurfaceInfo(0, faceIdx, 0));
				subresource.m_mipmapCount = m_lightShading.m_mipCount;

				Texture* texToBind;
				rgraphCtx.getRenderTargetState(probeTexture, subresource, texToBind);

				TextureViewInitInfo viewInit(texToBind, subresource);
				TextureViewPtr view = GrManager::getSingleton().newTextureView(viewInit);
				rgraphCtx.m_commandBuffer->generateMipmaps2d(view.get());
			});
		}
	}
}

} // end namespace anki
