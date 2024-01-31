// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Resource/AsyncLoader.h>

namespace anki {

static NumericCVar<U32> g_indirectDiffuseProbeTileResolutionCVar(CVarSubsystem::kRenderer, "IndirectDiffuseProbeTileResolution",
																 (ANKI_PLATFORM_MOBILE) ? 16 : 32, 8, 32, "GI tile resolution");
static NumericCVar<U32> g_indirectDiffuseProbeShadowMapResolutionCVar(CVarSubsystem::kRenderer, "IndirectDiffuseProbeShadowMapResolution", 128, 4,
																	  2048, "GI shadowmap resolution");

static StatCounter g_giProbeRenderCountStatVar(StatCategory::kRenderer, "GI probes rendered", StatFlag::kMainThreadUpdates);
static StatCounter g_giProbeCellsRenderCountStatVar(StatCategory::kRenderer, "GI probes cells rendered", StatFlag::kMainThreadUpdates);

static Vec3 computeCellCenter(U32 cellIdx, const GlobalIlluminationProbeComponent& probe)
{
	const Vec3 halfAabbSize = probe.getBoxVolumeSize() / 2.0f;
	const Vec3 aabbMin = -halfAabbSize + probe.getWorldPosition();
	U32 x, y, z;
	unflatten3dArrayIndex(probe.getCellCountsPerDimension().x(), probe.getCellCountsPerDimension().y(), probe.getCellCountsPerDimension().z(),
						  cellIdx, x, y, z);
	const Vec3 cellSize = probe.getBoxVolumeSize() / Vec3(probe.getCellCountsPerDimension());
	const Vec3 halfCellSize = cellSize / 2.0f;
	const Vec3 cellCenter = aabbMin + halfCellSize + cellSize * Vec3(UVec3(x, y, z));

	return cellCenter;
}

Error IndirectDiffuseProbes::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize global illumination");
	}

	return err;
}

Error IndirectDiffuseProbes::initInternal()
{
	m_tileSize = g_indirectDiffuseProbeTileResolutionCVar.get();

	ANKI_CHECK(initGBuffer());
	ANKI_CHECK(initLightShading());
	ANKI_CHECK(initShadowMapping());
	ANKI_CHECK(initIrradiance());

	return Error::kNone;
}

Error IndirectDiffuseProbes::initGBuffer()
{
	// Create RT descriptions
	{
		RenderTargetDescription texinit =
			getRenderer().create2DRenderTargetDescription(m_tileSize, m_tileSize, kGBufferColorRenderTargetFormats[0], "GI GBuffer");
		texinit.m_type = TextureType::kCube;

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(RendererString().sprintf("GI GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_type = TextureType::k2D;
		texinit.m_format = getRenderer().getDepthNoStencilFormat();
		texinit.setName("GI GBuff Depth");
		m_gbuffer.m_depthRtDescr = texinit;
		m_gbuffer.m_depthRtDescr.bake();
	}

	return Error::kNone;
}

Error IndirectDiffuseProbes::initShadowMapping()
{
	const U32 resolution = g_indirectDiffuseProbeShadowMapResolutionCVar.get();
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		getRenderer().create2DRenderTargetDescription(resolution, resolution, getRenderer().getDepthNoStencilFormat(), "GI SM");
	m_shadowMapping.m_rtDescr.bake();

	// Create the FB descr
	m_shadowMapping.m_fbDescr.m_colorAttachmentCount = 0;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
	m_shadowMapping.m_fbDescr.bake();

	return Error::kNone;
}

Error IndirectDiffuseProbes::initLightShading()
{
	// Init RT descr
	{
		m_lightShading.m_rtDescr = getRenderer().create2DRenderTargetDescription(m_tileSize, m_tileSize, getRenderer().getHdrFormat(), "GI LS");
		m_lightShading.m_rtDescr.m_type = TextureType::kCube;
		m_lightShading.m_rtDescr.bake();
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

Error IndirectDiffuseProbes::initIrradiance()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IrradianceDice.ankiprogbin",
								 {{"THREDGROUP_SIZE_SQRT", MutatorValue(m_tileSize)}, {"STORE_LOCATION", 0}, {"SECOND_BOUNCE", 1}},
								 m_irradiance.m_prog, m_irradiance.m_grProg));

	return Error::kNone;
}

void IndirectDiffuseProbes::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	// Iterate the visible probes to find a candidate for update
	WeakArray<GlobalIlluminationProbeComponent*> visibleProbes =
		getRenderer().getPrimaryNonRenderableVisibility().getInterestingVisibleComponents().m_globalIlluminationProbes;
	GlobalIlluminationProbeComponent* bestCandidateProbe = nullptr;
	GlobalIlluminationProbeComponent* secondBestCandidateProbe = nullptr;
	for(GlobalIlluminationProbeComponent* probe : visibleProbes)
	{
		if(probe->getCellsNeedsRefresh())
		{
			if(probe->getNextCellForRefresh() != 0)
			{
				bestCandidateProbe = probe;
				break;
			}
			else
			{
				secondBestCandidateProbe = probe;
			}
		}
	}

	GlobalIlluminationProbeComponent* probeToRefresh = (bestCandidateProbe) ? bestCandidateProbe : secondBestCandidateProbe;
	if(probeToRefresh == nullptr || ResourceManager::getSingleton().getAsyncLoader().getTasksInFlightCount() != 0) [[likely]]
	{
		// Nothing to update or can't update right now, early exit
		m_runCtx = {};
		return;
	}

	const Bool probeTouchedFirstTime = probeToRefresh->getNextCellForRefresh() == 0;
	if(probeTouchedFirstTime)
	{
		g_giProbeRenderCountStatVar.increment(1);
	}

	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	// Create some common resources to save on memory
	Array<RenderTargetHandle, kMaxColorRenderTargets> gbufferColorRts;
	for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
	{
		gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
	}
	const RenderTargetHandle gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

	const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
	const Bool doShadows = dirLightc && dirLightc->getShadowEnabled();
	const RenderTargetHandle shadowsRt = (doShadows) ? rgraph.newRenderTarget(m_shadowMapping.m_rtDescr) : RenderTargetHandle();
	const RenderTargetHandle lightShadingRt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);
	const RenderTargetHandle irradianceVolume = rgraph.importRenderTarget(&probeToRefresh->getVolumeTexture(), TextureUsageBit::kNone);

	m_runCtx.m_probeVolumeHandle = irradianceVolume;

	const U32 beginCellIdx = probeToRefresh->getNextCellForRefresh();
	for(U32 cellIdx = beginCellIdx; cellIdx < min(beginCellIdx + kProbeCellRefreshesPerFrame, probeToRefresh->getCellCount()); ++cellIdx)
	{
		const Vec3 cellCenter = computeCellCenter(cellIdx, *probeToRefresh);

		// For each face do everything up to light shading
		for(U8 f = 0; f < 6; ++f)
		{
			// GBuffer visibility
			GpuVisibilityOutput visOut;
			GpuMeshletVisibilityOutput meshletVisOut;
			Frustum frustum;
			{
				frustum.setPerspective(kClusterObjectFrustumNearPlane, probeToRefresh->getRenderRadius(), kPi / 2.0f, kPi / 2.0f);
				frustum.setWorldTransform(Transform(cellCenter.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[f], 1.0f));
				frustum.update();

				Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

				FrustumGpuVisibilityInput visIn;
				visIn.m_passesName = computeTempPassName("GI: GBuffer", cellIdx, "face", f);
				visIn.m_technique = RenderingTechnique::kGBuffer;
				visIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
				visIn.m_lodReferencePoint = cellCenter;
				visIn.m_lodDistances = lodDistances;
				visIn.m_rgraph = &rgraph;
				visIn.m_viewportSize = UVec2(m_tileSize);

				getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);

				if(getRenderer().runSoftwareMeshletRendering())
				{
					GpuMeshletVisibilityInput meshIn;
					meshIn.m_passesName = visIn.m_passesName;
					meshIn.m_technique = RenderingTechnique::kGBuffer;
					meshIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
					meshIn.m_cameraTransform = frustum.getViewMatrix().getInverseTransformation();
					meshIn.m_viewportSize = UVec2(m_tileSize);
					meshIn.m_rgraph = &rgraph;
					meshIn.fillBuffers(visOut);

					getRenderer().getGpuVisibility().populateRenderGraph(meshIn, meshletVisOut);
				}
			}

			// GBuffer
			{
				// Create the FB descriptor
				FramebufferDescription fbDescr;
				fbDescr.m_colorAttachmentCount = kGBufferColorRenderTargetCount;
				for(U j = 0; j < kGBufferColorRenderTargetCount; ++j)
				{
					fbDescr.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::kClear;
					fbDescr.m_colorAttachments[j].m_surface.m_face = f;
				}
				fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
				fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
				fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
				fbDescr.bake();

				// Create the pass
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(computeTempPassName("GI: GBuffer", cellIdx, "face", f));
				pass.setFramebufferInfo(fbDescr, gbufferColorRts, gbufferDepthRt);

				for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
				{
					pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kFramebufferWrite, TextureSurfaceInfo(0, 0, f, 0));
				}
				pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kAllFramebuffer, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

				pass.newBufferDependency((meshletVisOut.isFilled()) ? meshletVisOut.m_dependency : visOut.m_dependency,
										 BufferUsageBit::kIndirectDraw);

				pass.setWork(1, [this, visOut, meshletVisOut, viewProjMat = frustum.getViewProjectionMatrix(),
								 viewMat = frustum.getViewMatrix()](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);
					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					cmdb.setViewport(0, 0, m_tileSize, m_tileSize);

					RenderableDrawerArguments args;
					args.m_viewMatrix = viewMat;
					args.m_cameraTransform = args.m_viewMatrix.getInverseTransformation();
					args.m_viewProjectionMatrix = viewProjMat;
					args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
					args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
					args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
					args.m_viewport = UVec4(0, 0, m_tileSize, m_tileSize);
					args.fill(visOut);

					if(meshletVisOut.isFilled())
					{
						args.fill(meshletVisOut);
					}

					getRenderer().getSceneDrawer().drawMdi(args, cmdb);

					// It's secondary, no need to restore any state
				});
			}

			// Shadow visibility. Optional
			GpuVisibilityOutput shadowVisOut;
			GpuMeshletVisibilityOutput shadowMeshletVisOut;
			Mat4 cascadeProjMat;
			Mat3x4 cascadeViewMat;
			Mat4 cascadeViewProjMat;
			if(doShadows)
			{
				constexpr U32 kCascadeCount = 1;
				dirLightc->computeCascadeFrustums(frustum, Array<F32, kCascadeCount>{probeToRefresh->getShadowsRenderRadius()},
												  WeakArray<Mat4>(&cascadeProjMat, kCascadeCount), WeakArray<Mat3x4>(&cascadeViewMat, kCascadeCount));

				cascadeViewProjMat = cascadeProjMat * Mat4(cascadeViewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));

				Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar.get(), g_lod1MaxDistanceCVar.get()};

				FrustumGpuVisibilityInput visIn;
				visIn.m_passesName = computeTempPassName("GI: Shadows", cellIdx, "face", f);
				visIn.m_technique = RenderingTechnique::kDepth;
				visIn.m_viewProjectionMatrix = cascadeViewProjMat;
				visIn.m_lodReferencePoint = cellCenter;
				visIn.m_lodDistances = lodDistances;
				visIn.m_rgraph = &rgraph;
				visIn.m_viewportSize = UVec2(m_shadowMapping.m_rtDescr.m_height);

				getRenderer().getGpuVisibility().populateRenderGraph(visIn, shadowVisOut);

				if(getRenderer().runSoftwareMeshletRendering())
				{
					GpuMeshletVisibilityInput meshIn;
					meshIn.m_passesName = visIn.m_passesName;
					meshIn.m_technique = RenderingTechnique::kDepth;
					meshIn.m_viewProjectionMatrix = cascadeViewProjMat;
					meshIn.m_cameraTransform = cascadeViewMat.getInverseTransformation();
					meshIn.m_viewportSize = visIn.m_viewportSize;
					meshIn.m_rgraph = &rgraph;
					meshIn.fillBuffers(shadowVisOut);

					getRenderer().getGpuVisibility().populateRenderGraph(meshIn, shadowMeshletVisOut);
				}
			}

			// Shadow pass. Optional
			if(doShadows)
			{
				// Create the pass
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(computeTempPassName("GI: Shadows", cellIdx, "face", f));
				pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, shadowsRt);

				pass.newTextureDependency(shadowsRt, TextureUsageBit::kAllFramebuffer, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
				pass.newBufferDependency((shadowMeshletVisOut.isFilled()) ? shadowMeshletVisOut.m_dependency : shadowVisOut.m_dependency,
										 BufferUsageBit::kIndirectDraw);

				pass.setWork(1, [this, shadowVisOut, shadowMeshletVisOut, cascadeViewProjMat, cascadeViewMat](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);
					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

					const U32 rez = m_shadowMapping.m_rtDescr.m_width;
					cmdb.setViewport(0, 0, rez, rez);

					RenderableDrawerArguments args;
					args.m_viewMatrix = cascadeViewMat;
					args.m_cameraTransform = cascadeViewMat.getInverseTransformation();
					args.m_viewProjectionMatrix = cascadeViewProjMat;
					args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
					args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
					args.m_renderingTechinuqe = RenderingTechnique::kDepth;
					args.m_viewport = UVec4(0, 0, rez, rez);
					args.fill(shadowVisOut);

					if(shadowMeshletVisOut.isFilled())
					{
						args.fill(shadowMeshletVisOut);
					}

					getRenderer().getSceneDrawer().drawMdi(args, cmdb);

					// It's secondary, no need to restore the state
				});
			}

			// Light visibility
			GpuVisibilityNonRenderablesOutput lightVis;
			{
				GpuVisibilityNonRenderablesInput in;
				in.m_passesName = computeTempPassName("GI: Light visibility", cellIdx, "face", f);
				in.m_objectType = GpuSceneNonRenderableObjectType::kLight;
				in.m_viewProjectionMat = frustum.getViewProjectionMatrix();
				in.m_rgraph = &rgraph;
				getRenderer().getGpuVisibilityNonRenderables().populateRenderGraph(in, lightVis);
			}

			// Light shading pass
			{
				// Create FB descr
				FramebufferDescription fbDescr;
				fbDescr.m_colorAttachmentCount = 1;
				fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
				fbDescr.m_colorAttachments[0].m_surface.m_face = f;
				fbDescr.bake();

				// Create the pass
				GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(computeTempPassName("GI: Light shading", cellIdx, "face", f));
				pass.setFramebufferInfo(fbDescr, {lightShadingRt});

				pass.newBufferDependency(lightVis.m_visiblesBufferHandle, BufferUsageBit::kUavFragmentRead);

				pass.newTextureDependency(lightShadingRt, TextureUsageBit::kFramebufferWrite, TextureSurfaceInfo(0, 0, f, 0));

				for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
				{
					pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSampledFragment, TextureSurfaceInfo(0, 0, f, 0));
				}
				pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kSampledFragment, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

				if(shadowsRt.isValid())
				{
					pass.newTextureDependency(shadowsRt, TextureUsageBit::kSampledFragment);
				}

				pass.setWork(1, [this, visibleLightsBuffer = lightVis.m_visiblesBuffer, viewProjMat = frustum.getViewProjectionMatrix(), cellCenter,
								 gbufferColorRts, gbufferDepthRt, probeToRefresh, cascadeViewProjMat, shadowsRt,
								 faceIdx = f](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

					const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
					const Bool doShadows = dirLightc && dirLightc->getShadowEnabled();

					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					const U32 rez = m_tileSize;
					cmdb.setViewport(0, 0, rez, rez);

					// Draw light shading
					TraditionalDeferredLightShadingDrawInfo dsInfo;
					dsInfo.m_viewProjectionMatrix = viewProjMat;
					dsInfo.m_invViewProjectionMatrix = viewProjMat.getInverse();
					dsInfo.m_cameraPosWSpace = cellCenter.xyz1();
					dsInfo.m_viewport = UVec4(0, 0, m_tileSize, m_tileSize);
					dsInfo.m_effectiveShadowDistance = (doShadows) ? probeToRefresh->getShadowsRenderRadius() : -1.0f;

					if(doShadows)
					{
						const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
						dsInfo.m_dirLightMatrix = biasMat4 * cascadeViewProjMat;
					}
					else
					{
						dsInfo.m_dirLightMatrix = Mat4::getIdentity();
					}

					dsInfo.m_visibleLightsBuffer = visibleLightsBuffer;

					dsInfo.m_gbufferRenderTargets[0] = gbufferColorRts[0];
					dsInfo.m_gbufferRenderTargetSubresourceInfos[0].m_firstFace = faceIdx;
					dsInfo.m_gbufferRenderTargets[1] = gbufferColorRts[1];
					dsInfo.m_gbufferRenderTargetSubresourceInfos[1].m_firstFace = faceIdx;
					dsInfo.m_gbufferRenderTargets[2] = gbufferColorRts[2];
					dsInfo.m_gbufferRenderTargetSubresourceInfos[2].m_firstFace = faceIdx;
					dsInfo.m_gbufferDepthRenderTarget = gbufferDepthRt;
					dsInfo.m_directionalLightShadowmapRenderTarget = shadowsRt;
					dsInfo.m_renderpassContext = &rgraphCtx;

					m_lightShading.m_deferred.drawLights(dsInfo);
				});
			}
		} // For all faces

		// Irradiance pass. First & 2nd bounce
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(computeTempPassName("GI: Irradiance", cellIdx));

			pass.newTextureDependency(lightShadingRt, TextureUsageBit::kSampledCompute);
			pass.newTextureDependency(irradianceVolume, TextureUsageBit::kUavComputeWrite);
			for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSampledCompute);
			}

			pass.setWork([this, lightShadingRt, gbufferColorRts, irradianceVolume, cellIdx, probeToRefresh](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_irradiance.m_grProg.get());

				// Bind resources
				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
				rgraphCtx.bindColorTexture(0, 1, lightShadingRt);

				for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
				{
					rgraphCtx.bindColorTexture(0, 2, gbufferColorRts[i], i);
				}

				rgraphCtx.bindUavTexture(0, 3, irradianceVolume, TextureSubresourceInfo());

				class
				{
				public:
					IVec3 m_volumeTexel;
					I32 m_nextTexelOffsetInU;
				} unis;

				U32 x, y, z;
				unflatten3dArrayIndex(probeToRefresh->getCellCountsPerDimension().x(), probeToRefresh->getCellCountsPerDimension().y(),
									  probeToRefresh->getCellCountsPerDimension().z(), cellIdx, x, y, z);
				unis.m_volumeTexel = IVec3(x, y, z);

				unis.m_nextTexelOffsetInU = probeToRefresh->getCellCountsPerDimension().x();
				cmdb.setPushConstants(&unis, sizeof(unis));

				// Dispatch
				cmdb.dispatchCompute(1, 1, 1);
			});
		}

		probeToRefresh->incrementRefreshedCells(1);
		g_giProbeCellsRenderCountStatVar.increment(1);
	}
}

} // end namespace anki
