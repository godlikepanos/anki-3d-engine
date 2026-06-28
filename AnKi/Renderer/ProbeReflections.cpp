// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Utils/MipmapGenerator.h>
#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Renderer/Reflections.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/GpuMemory/CopyEngine.h>
#include <AnKi/Shaders/Include/TraditionalDeferredShadingTypes.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

ANKI_SVAR(ProbeReflectionCount, StatCategory::kRenderer, "Reflection probes rendered", StatFlag::kMainThreadUpdates)

// 32-bit radical inverse (Van der Corput), matching Unreal's ReverseBits.
static U32 reverseBits32(U32 b)
{
	b = (b << 16u) | (b >> 16u);
	b = ((b & 0x00ff00ffu) << 8u) | ((b & 0xff00ff00u) >> 8u);
	b = ((b & 0x0f0f0f0fu) << 4u) | ((b & 0xf0f0f0f0u) >> 4u);
	b = ((b & 0x33333333u) << 2u) | ((b & 0xccccccccu) >> 2u);
	b = ((b & 0x55555555u) << 1u) | ((b & 0xaaaaaaaau) >> 1u);
	return b;
}

Error ProbeReflections::initIntegrationLut()
{
	// CPU pre-integration of the specular environment BRDF (split-sum DFG), mirroring Unreal's PreintegratedGF generation (FSystemTextures,
	// SystemTextures.cpp). Replaces EngineAssets/IblDfg.png. R16G16 unorm, indexed by (u = NoV, v = roughness): .r = scale A, .g = bias B, so
	// single-scattering indirect specular = F0 * A + B. The Turquin multiple-scattering compensation is applied analytically at runtime in
	// specularDFG() (LightFunctions.hlsl).
	constexpr U32 kWidth = 128; // NoV
	constexpr U32 kHeight = 32; // roughness
	constexpr U32 kSampleCount = 128;

	// Create the texture
	TextureInitInfo texInit("IntegrationDfgLut");
	texInit.m_width = kWidth;
	texInit.m_height = kHeight;
	texInit.m_format = Format::kR16G16_Unorm;
	texInit.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kCopyDestination;
	const PtrSize memReq = GrManager::getSingleton().getTextureMemoryRequirement(texInit);
	m_integrationLut.m_allocation = getRenderer().getRendedererGpuMemoryPool().allocate(memReq, 1);
	texInit.m_memoryBuffer = m_integrationLut.m_allocation;
	m_integrationLut.m_texture = GrManager::getSingleton().newTexture(texInit);

	// Integrate on the CPU
	RendererDynamicArray<U16> data;
	data.resize(kWidth * kHeight * 2); // 2 channels

	for(U32 y = 0; y < kHeight; ++y)
	{
		const F32 roughness = (F32(y) + 0.5f) / F32(kHeight);
		const F32 m = roughness * roughness;
		const F32 m2 = m * m;

		for(U32 x = 0; x < kWidth; ++x)
		{
			const F32 NoV = (F32(x) + 0.5f) / F32(kWidth);
			const Vec3 V(sqrt(1.0f - NoV * NoV), 0.0f, NoV);

			F32 A = 0.0f;
			F32 B = 0.0f;
			for(U32 i = 0; i < kSampleCount; ++i)
			{
				const F32 e1 = F32(i) / F32(kSampleCount);
				const F32 e2 = F32(F64(reverseBits32(i)) / F64(0x100000000ll));

				const F32 phi = 2.0f * kPi * e1;
				const F32 cosTheta = sqrt((1.0f - e2) / (1.0f + (m2 - 1.0f) * e2));
				const F32 sinTheta = sqrt(1.0f - cosTheta * cosTheta);
				const Vec3 H(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
				const Vec3 L = 2.0f * V.dot(H) * H - V;

				const F32 NoL = max(L.z, 0.0f);
				const F32 NoH = max(H.z, 0.0f);
				const F32 VoH = max(V.dot(H), 0.0f);

				if(NoL > 0.0f)
				{
					// Schlick-linear Smith visibility, matching Unreal's generator (NOT exact Smith).
					const F32 visV = NoL * (NoV * (1.0f - m) + m);
					const F32 visL = NoV * (NoL * (1.0f - m) + m);
					const F32 vis = 0.5f / (visV + visL);

					const F32 noLVisPdf = NoL * vis * (4.0f * VoH / NoH);
					F32 fc = 1.0f - VoH;
					fc *= fc * fc * fc * fc; // (1 - VoH)^5
					A += noLVisPdf * (1.0f - fc);
					B += noLVisPdf * fc;
				}
			}

			A /= F32(kSampleCount);
			B /= F32(kSampleCount);

			const U32 idx = (y * kWidth + x) * 2;
			data[idx + 0] = U16(clamp(A, 0.0f, 1.0f) * 65535.0f + 0.5f);
			data[idx + 1] = U16(clamp(B, 0.0f, 1.0f) * 65535.0f + 0.5f);
		}
	}

	// Upload to the GPU
	const TextureView view(m_integrationLut.get(), TextureSubresourceDesc::surface(0, 0, 0));

	const TextureBarrierInfo toCopy = {view, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination};
	CopyEngine::getSingleton().setPipelineBarrier({&toCopy, 1}, {}, {});

	WeakArray<U8> mappedMem;
	{
		const CopyEngineLockGuard lock = CopyEngine::getSingleton().copyBufferToTexture(U32(data.getSizeInBytes()), mappedMem, view);
		memcpy(mappedMem.getBegin(), data.getBegin(), data.getSizeInBytes());
	}

	const TextureBarrierInfo toSrv = {view, TextureUsageBit::kCopyDestination, TextureUsageBit::kSrvPixel};
	CopyEngine::getSingleton().setPipelineBarrier({&toSrv, 1}, {}, {});

	FencePtr fence;
	CopyEngine::getSingleton().flush(fence);
	const Bool signaled = fence->clientWait(kMaxSecond);
	if(!signaled)
	{
		ANKI_R_LOGF("GPU timeout detected");
	}

	return Error::kNone;
}

Error ProbeReflections::init()
{
	ANKI_CHECK(initIntegrationLut());

	m_gbuffer.m_tileSize = g_cvarRenderProbeReflectionsResolution;

	{
		RenderTargetDesc texinit = getRenderer().create2DRenderTargetDescription(m_gbuffer.m_tileSize, m_gbuffer.m_tileSize,
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

	m_lightShading.m_tileSize = g_cvarRenderProbeReflectionsResolution;
	m_lightShading.m_mipCount = computeMaxMipmapCount2d(m_lightShading.m_tileSize, m_lightShading.m_tileSize, 8);

	{
		const U32 resolution = g_cvarRenderProbeReflectionsShadowMapResolution;
		ANKI_ASSERT(resolution > 8);

		// RT descr
		m_shadowMapping.m_rtDescr =
			getRenderer().create2DRenderTargetDescription(resolution, resolution, getRenderer().getDepthNoStencilFormat(), "CubeRefl SM");
		m_shadowMapping.m_rtDescr.bake();
	}

	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

void ProbeReflections::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

	const Bool bRtReflections = GrManager::getSingleton().getDeviceCapabilities().m_rayTracing && g_cvarRenderReflectionsRt;
	if(bRtReflections)
	{
		return;
	}

	// Iterate the visible probes to find a candidate for update
	WeakArray<ReflectionProbeComponent*> visibleProbes = getPrimaryNonRenderableVisibility().getInterestingVisibleComponents().m_reflectionProbes;
	ReflectionProbeComponent* probeToRefresh = nullptr;
	for(ReflectionProbeComponent* probe : visibleProbes)
	{
		if(probe->getEnvironmentTextureNeedsRefresh())
		{
			probeToRefresh = probe;
			break;
		}
	}

	if(probeToRefresh == nullptr || AsyncLoader::getSingleton().getTasksInFlightCount() != 0
	   || (isIndirectDiffuseProbesEnabled() && getIndirectDiffuseProbes().hasCurrentlyRefreshedVolumeRt())) [[likely]]
	{
		// Nothing to update or can't update right now, early exit
		m_runCtx = {};
		return;
	}

	g_svarProbeReflectionCount.increment(1);
	probeToRefresh->setEnvironmentTextureAsRefreshed();

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
	const Bool doShadows = dirLightc && dirLightc->getShadowEnabled();

	// Create render targets now to save memory
	const RenderTargetHandle probeTexture = rgraph.importRenderTarget(&probeToRefresh->getReflectionTexture(), true, TextureUsageBit::kNone);
	m_runCtx.m_probeTex = probeTexture;
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
		Frustum frustum;
		{
			frustum.setPerspective(kClusterObjectFrustumNearPlane, probeToRefresh->getRenderRadius(), kPi / 2.0f, kPi / 2.0f);
			frustum.setWorldTransform(
				Transform(probeToRefresh->getWorldPosition().xyz0, Frustum::getOmnidirectionalFrustumRotations()[f], Vec4(1.0f, 1.0f, 1.0f, 0.0f)));
			frustum.update();

			Array<F32, kMaxLodCount - 1> lodDistances = {g_cvarRenderLod0MaxDistance, g_cvarRenderLod1MaxDistance};

			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = generateTempPassName("Cube refl: GBuffer face:%u", f);
			visIn.m_technique = RenderingTechnique::kGBuffer;
			visIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
			visIn.m_lodReferencePoint = probeToRefresh->getWorldPosition();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewportSize = UVec2(m_gbuffer.m_tileSize);
			visIn.m_limitMemory = true;

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);
		}

		// GBuffer pass
		{
			// Create pass
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: GBuffer face:%u", f));

			Array<GraphicsRenderPassTargetDesc, kGBufferColorRenderTargetCount> colorRtis;
			for(U j = 0; j < kGBufferColorRenderTargetCount; ++j)
			{
				colorRtis[j].m_loadOperation = RenderTargetLoadOperation::kClear;
				colorRtis[j].m_subresource.m_face = f;
				colorRtis[j].m_handle = gbufferColorRts[j];
			}
			GraphicsRenderPassTargetDesc depthRti(gbufferDepthRt);
			depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
			depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
			depthRti.m_clearValue.m_depthStencil.m_depth = 1.0f;

			pass.setRenderpassInfo(colorRtis, &depthRti);

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kRtvDsvWrite, TextureSubresourceDesc::surface(0, f, 0));
			}

			pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kAllRtvDsv, DepthStencilAspectBit::kDepth);
			pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kIndirectDraw);

			pass.setWork(
				[this, visOut, viewProjMat = frustum.getViewProjectionMatrix(), viewMat = frustum.getViewMatrix()](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(ProbeReflections);
					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					cmdb.setViewport(0, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

					RenderableDrawerArguments args;
					args.m_viewMatrix = viewMat;
					args.m_cameraTransform = viewMat.invertTransformation();
					args.m_viewProjectionMatrix = viewProjMat;
					args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care about prev mats
					args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
					args.m_viewport = UVec4(0, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
					args.fill(visOut);

					getRenderer().getRenderableDrawer().drawMdi(args, rgraphCtx);
				});
		}

		// Shadow visibility. Optional
		GpuVisibilityOutput shadowVisOut;
		Mat4 cascadeViewProjMat;
		Mat3x4 cascadeViewMat;
		Mat4 cascadeProjMat;
		if(doShadows)
		{
			constexpr U32 kCascadeCount = 1;
			dirLightc->computeCascadeFrustums(frustum, Array<F32, kCascadeCount>{probeToRefresh->getShadowsRenderRadius()},
											  WeakArray<Mat4>(&cascadeProjMat, kCascadeCount), WeakArray<Mat3x4>(&cascadeViewMat, kCascadeCount));

			cascadeViewProjMat = cascadeProjMat * Mat4(cascadeViewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));

			Array<F32, kMaxLodCount - 1> lodDistances = {g_cvarRenderLod0MaxDistance, g_cvarRenderLod1MaxDistance};

			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = generateTempPassName("Cube refl: Shadows face:%u", f);
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_viewProjectionMatrix = cascadeViewProjMat;
			visIn.m_lodReferencePoint = probeToRefresh->getWorldPosition();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;
			visIn.m_viewportSize = UVec2(m_shadowMapping.m_rtDescr.m_height);
			visIn.m_limitMemory = true;

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, shadowVisOut);
		}

		// Shadows. Optional
		if(doShadows)
		{
			// Pass
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: Shadows face:%u", f));

			GraphicsRenderPassTargetDesc depthRti(shadowMapRt);
			depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
			depthRti.m_clearValue.m_depthStencil.m_depth = 1.0f;
			depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;

			pass.setRenderpassInfo({}, &depthRti);

			pass.newTextureDependency(shadowMapRt, TextureUsageBit::kAllRtvDsv, DepthStencilAspectBit::kDepth);
			pass.newBufferDependency(shadowVisOut.m_dependency, BufferUsageBit::kIndirectDraw);

			pass.setWork([this, shadowVisOut, cascadeViewProjMat, cascadeViewMat](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
				cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

				const U32 rez = m_shadowMapping.m_rtDescr.m_height;
				cmdb.setViewport(0, 0, rez, rez);

				RenderableDrawerArguments args;
				args.m_viewMatrix = cascadeViewMat;
				args.m_cameraTransform = cascadeViewMat.invertTransformation();
				args.m_viewProjectionMatrix = cascadeViewProjMat;
				args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
				args.m_renderingTechinuqe = RenderingTechnique::kDepth;
				args.m_viewport = UVec4(0, 0, rez, rez);
				args.fill(shadowVisOut);

				getRenderer().getRenderableDrawer().drawMdi(args, rgraphCtx);

				cmdb.setPolygonOffset(0.0, 0.0);
			});
		}

		// Light visibility
		GpuVisibilityNonRenderablesOutput lightVis;
		{
			GpuVisibilityNonRenderablesInput in;
			in.m_passesName = generateTempPassName("Cube refl: Light visibility face:%u", f);
			in.m_objectType = GpuSceneNonRenderableObjectType::kLight;
			in.m_viewProjectionMat = frustum.getViewProjectionMatrix();
			in.m_rgraph = &rgraph;
			getRenderer().getGpuVisibilityNonRenderables().populateRenderGraph(in, lightVis);
		}

		// Light shading pass
		{
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Cube refl: light shading face:%u", f));

			GraphicsRenderPassTargetDesc colorRti(probeTexture);
			colorRti.m_subresource.m_face = f;
			colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
			pass.setRenderpassInfo({colorRti});

			pass.newBufferDependency(lightVis.m_visiblesBufferHandle, BufferUsageBit::kSrvPixel);
			pass.newTextureDependency(probeTexture, TextureUsageBit::kRtvDsvWrite, TextureSubresourceDesc::surface(0, f, 0));

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSrvPixel, TextureSubresourceDesc::surface(0, f, 0));
			}
			pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kSrvPixel, DepthStencilAspectBit::kDepth);

			if(shadowMapRt.isValid())
			{
				pass.newTextureDependency(shadowMapRt, TextureUsageBit::kSrvPixel);
			}

			if(getRenderer().getGeneratedSky().isEnabled())
			{
				pass.newTextureDependency(getRenderer().getGeneratedSky().getSkyLutRt(), TextureUsageBit::kSrvPixel);
			}

			if(isIndirectDiffuseProbesEnabled() && getIndirectDiffuseProbes().hasCurrentlyRefreshedVolumeRt())
			{
				pass.newTextureDependency(getRenderer().getIndirectDiffuseProbes().getCurrentlyRefreshedVolumeRt(), TextureUsageBit::kSrvPixel);
			}
			else if(!isIndirectDiffuseProbesEnabled())
			{
				getIndirectDiffuseClipmaps().setDependencies(pass, TextureUsageBit::kSrvPixel);
			}

			pass.setWork([this, visResult = lightVis.m_visiblesBuffer, viewProjMat = frustum.getViewProjectionMatrix(),
						  cascadeViewProjMat = cascadeViewProjMat, probeToRefresh, gbufferColorRts, gbufferDepthRt, shadowMapRt,
						  faceIdx = f](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ProbeReflections);

				TraditionalDeferredLightShadingDrawInfo dsInfo;
				dsInfo.m_viewProjectionMatrix = viewProjMat;
				dsInfo.m_invViewProjectionMatrix = viewProjMat.invert();
				dsInfo.m_cameraPosWSpace = probeToRefresh->getWorldPosition().xyz1;
				dsInfo.m_viewport = UVec4(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
				dsInfo.m_effectiveShadowDistance = probeToRefresh->getShadowsRenderRadius();
				dsInfo.m_applyIndirectDiffuse = true;

				const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
				dsInfo.m_dirLightMatrix = biasMat4 * cascadeViewProjMat;

				dsInfo.m_visibleLightsBuffer = visResult;
				dsInfo.m_gbufferRenderTargets[0] = gbufferColorRts[0];
				dsInfo.m_gbufferRenderTargetSubresource[0].m_face = faceIdx;
				dsInfo.m_gbufferRenderTargets[1] = gbufferColorRts[1];
				dsInfo.m_gbufferRenderTargetSubresource[1].m_face = faceIdx;
				dsInfo.m_gbufferRenderTargets[2] = gbufferColorRts[2];
				dsInfo.m_gbufferRenderTargetSubresource[2].m_face = faceIdx;
				dsInfo.m_gbufferDepthRenderTarget = gbufferDepthRt;
				dsInfo.m_useIndirectDiffuseClipmaps = isIndirectDiffuseClipmapsEnabled();
				if(shadowMapRt.isValid())
				{
					dsInfo.m_directionalLightShadowmapRenderTarget = shadowMapRt;
				}
				dsInfo.m_skyLutRenderTarget =
					(getRenderer().getGeneratedSky().isEnabled()) ? getRenderer().getGeneratedSky().getSkyLutRt() : RenderTargetHandle();
				dsInfo.m_globalRendererConsts = getRenderingContext().m_globalRenderingConstantsBuffer;
				dsInfo.m_renderpassContext = &rgraphCtx;

				m_lightShading.m_deferred.drawLights(dsInfo);
			});
		}
	} // For 6 faces

	// Mipmapping "passes"
	{
		const MipmapGeneratorTargetArguments desc = {
			.m_handle = probeTexture,
			.m_targetSize = UVec2(probeToRefresh->getReflectionTexture().getWidth(), probeToRefresh->getReflectionTexture().getHeight()),
			.m_layerCount = 1,
			.m_mipmapCount = m_lightShading.m_mipCount,
			.m_isCubeTexture = true};

		getRenderer().getMipmapGenerator().populateRenderGraph(desc, rgraph, "Cube refl: Gen mips");
	}
}

} // end namespace anki
