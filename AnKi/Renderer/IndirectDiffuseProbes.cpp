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
			getRenderer().create2DRenderTargetDescription(m_tileSize * 6, m_tileSize, kGBufferColorRenderTargetFormats[0], "GI GBuffer");

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(RendererString().sprintf("GI GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = getRenderer().getDepthNoStencilFormat();
		texinit.setName("GI GBuff Depth");
		m_gbuffer.m_depthRtDescr = texinit;
		m_gbuffer.m_depthRtDescr.bake();
	}

	// Create FB descr
	{
		m_gbuffer.m_fbDescr.m_colorAttachmentCount = kGBufferColorRenderTargetCount;

		for(U j = 0; j < kGBufferColorRenderTargetCount; ++j)
		{
			m_gbuffer.m_fbDescr.m_colorAttachments[j].m_loadOperation = AttachmentLoadOperation::kClear;
		}

		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
		m_gbuffer.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;

		m_gbuffer.m_fbDescr.bake();
	}

	return Error::kNone;
}

Error IndirectDiffuseProbes::initShadowMapping()
{
	const U32 resolution = g_indirectDiffuseProbeShadowMapResolutionCVar.get();
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		getRenderer().create2DRenderTargetDescription(resolution * 6, resolution, getRenderer().getDepthNoStencilFormat(), "GI SM");
	m_shadowMapping.m_rtDescr.bake();

	// FB descr
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
		m_lightShading.m_rtDescr = getRenderer().create2DRenderTargetDescription(m_tileSize * 6, m_tileSize, getRenderer().getHdrFormat(), "GI LS");
		m_lightShading.m_rtDescr.bake();
	}

	// Create FB descr
	{
		m_lightShading.m_fbDescr.m_colorAttachmentCount = 1;
		m_lightShading.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
		m_lightShading.m_fbDescr.bake();
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

Error IndirectDiffuseProbes::initIrradiance()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IrradianceDice.ankiprogbin", m_irradiance.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_irradiance.m_prog);
	variantInitInfo.addMutation("WORKGROUP_SIZE_XY", m_tileSize);
	variantInitInfo.addMutation("LIGHT_SHADING_TEX", 0);
	variantInitInfo.addMutation("STORE_LOCATION", 0);
	variantInitInfo.addMutation("SECOND_BOUNCE", 1);

	const ShaderProgramResourceVariant* variant;
	m_irradiance.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_irradiance.m_grProg.reset(&variant->getProgram());

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

		// GBuffer visibility
		Array<GpuVisibilityOutput, 6> visOuts;
		Array<Frustum, 6> frustums;
		for(U32 i = 0; i < 6; ++i)
		{
			Frustum& frustum = frustums[i];
			frustum.setPerspective(kClusterObjectFrustumNearPlane, probeToRefresh->getRenderRadius(), kPi / 2.0f, kPi / 2.0f);
			frustum.setWorldTransform(Transform(cellCenter.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[i], 1.0f));
			frustum.update();

			Array<F32, kMaxLodCount - 1> lodDistances = {1000.0f, 1001.0f}; // Something far to force detailed LODs

			FrustumGpuVisibilityInput visIn;
			visIn.m_passesName = "GI GBuffer visibility";
			visIn.m_technique = RenderingTechnique::kGBuffer;
			visIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
			visIn.m_lodReferencePoint = cellCenter;
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOuts[i]);
		}

		// GBuffer
		Array<Mat4, 6> viewProjMats;
		Array<Mat3x4, 6> viewMats;
		{
			// Prepare the matrices
			for(U32 f = 0; f < 6; ++f)
			{
				viewProjMats[f] = frustums[f].getViewProjectionMatrix();
				viewMats[f] = frustums[f].getViewMatrix();
			}

			// Create the pass
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI GBuffer");
			pass.setFramebufferInfo(m_gbuffer.m_fbDescr, gbufferColorRts, gbufferDepthRt);

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kFramebufferWrite);
			}
			pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kAllFramebuffer, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			for(U32 i = 0; i < 6; ++i)
			{
				pass.newBufferDependency(visOuts[i].m_someBufferHandle, BufferUsageBit::kIndirectDraw);
			}

			pass.setWork(6, [this, visOuts, viewProjMats, viewMats](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
				const U32 faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

				const U32 viewportX = faceIdx * m_tileSize;
				cmdb.setViewport(viewportX, 0, m_tileSize, m_tileSize);
				cmdb.setScissor(viewportX, 0, m_tileSize, m_tileSize);

				RenderableDrawerArguments args;
				args.m_viewMatrix = viewMats[faceIdx];
				args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
				args.m_viewProjectionMatrix = viewProjMats[faceIdx];
				args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
				args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
				args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
				args.m_viewport = UVec4(viewportX, 0, m_tileSize, m_tileSize);
				args.fillMdi(visOuts[faceIdx]);

				getRenderer().getSceneDrawer().drawMdi(args, cmdb);

				// It's secondary, no need to restore any state
			});
		}

		// Shadow visibility. Optional
		Array<GpuVisibilityOutput, 6> shadowVisOuts;
		Array<Mat4, 6> cascadeProjMats;
		Array<Mat3x4, 6> cascadeViewMats;
		Array<Mat4, 6> cascadeViewProjMats;
		if(doShadows)
		{
			for(U32 i = 0; i < 6; ++i)
			{
				constexpr U32 kCascadeCount = 1;
				dirLightc->computeCascadeFrustums(frustums[i], Array<F32, kCascadeCount>{probeToRefresh->getShadowsRenderRadius()},
												  WeakArray<Mat4>(&cascadeProjMats[i], kCascadeCount),
												  WeakArray<Mat3x4>(&cascadeViewMats[i], kCascadeCount));

				cascadeViewProjMats[i] = cascadeProjMats[i] * Mat4(cascadeViewMats[i], Vec4(0.0f, 0.0f, 0.0f, 1.0f));

				Array<F32, kMaxLodCount - 1> lodDistances = {1000.0f, 1001.0f}; // Something far to force detailed LODs

				FrustumGpuVisibilityInput visIn;
				visIn.m_passesName = "GI shadows visibility";
				visIn.m_technique = RenderingTechnique::kDepth;
				visIn.m_viewProjectionMatrix = cascadeViewProjMats[i];
				visIn.m_lodReferencePoint = cellCenter;
				visIn.m_lodDistances = lodDistances;
				visIn.m_rgraph = &rgraph;

				getRenderer().getGpuVisibility().populateRenderGraph(visIn, shadowVisOuts[i]);
			}
		}

		// Shadow pass. Optional
		if(doShadows)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI shadows");
			pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, shadowsRt);

			pass.newTextureDependency(shadowsRt, TextureUsageBit::kAllFramebuffer, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			for(U32 i = 0; i < 6; ++i)
			{
				pass.newBufferDependency(shadowVisOuts[i].m_someBufferHandle, BufferUsageBit::kIndirectDraw);
			}

			pass.setWork(6, [this, shadowVisOuts, cascadeViewProjMats, cascadeViewMats](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

				const U32 faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

				const U32 rez = m_shadowMapping.m_rtDescr.m_height;
				cmdb.setViewport(rez * faceIdx, 0, rez, rez);
				cmdb.setScissor(rez * faceIdx, 0, rez, rez);

				RenderableDrawerArguments args;
				args.m_viewMatrix = cascadeViewMats[faceIdx];
				args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
				args.m_viewProjectionMatrix = cascadeViewProjMats[faceIdx];
				args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
				args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
				args.m_renderingTechinuqe = RenderingTechnique::kDepth;
				args.m_viewport = UVec4(rez * faceIdx, 0, rez, rez);
				args.fillMdi(shadowVisOuts[faceIdx]);

				getRenderer().getSceneDrawer().drawMdi(args, cmdb);

				// It's secondary, no need to restore the state
			});
		}

		// Light visibility
		Array<GpuVisibilityNonRenderablesOutput, 6> lightVis;
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GpuVisibilityNonRenderablesInput in;
			in.m_passesName = "GI light visibility";
			in.m_objectType = GpuSceneNonRenderableObjectType::kLight;
			in.m_viewProjectionMat = frustums[faceIdx].getViewProjectionMatrix();
			in.m_rgraph = &rgraph;
			getRenderer().getGpuVisibilityNonRenderables().populateRenderGraph(in, lightVis[faceIdx]);
		}

		// Light shading pass
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI light shading");
			pass.setFramebufferInfo(m_lightShading.m_fbDescr, {lightShadingRt});

			Array<BufferOffsetRange, 6> visibleLightsBuffers;
			for(U32 f = 0; f < 6; ++f)
			{
				visibleLightsBuffers[f] = lightVis[f].m_visiblesBuffer;
				pass.newBufferDependency(lightVis[f].m_visiblesBufferHandle, BufferUsageBit::kUavFragmentRead);
			}

			pass.newTextureDependency(lightShadingRt, TextureUsageBit::kFramebufferWrite);

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSampledFragment);
			}
			pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kSampledFragment, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			if(shadowsRt.isValid())
			{
				pass.newTextureDependency(shadowsRt, TextureUsageBit::kSampledFragment);
			}

			pass.setWork(1, [this, visibleLightsBuffers, viewProjMats, cellCenter, gbufferColorRts, gbufferDepthRt, probeToRefresh,
							 cascadeViewProjMats, shadowsRt](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

				const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
				const Bool doShadows = dirLightc && dirLightc->getShadowEnabled();

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
				{
					const U32 rez = m_tileSize;
					cmdb.setScissor(rez * faceIdx, 0, rez, rez);
					cmdb.setViewport(rez * faceIdx, 0, rez, rez);

					// Draw light shading
					TraditionalDeferredLightShadingDrawInfo dsInfo;
					dsInfo.m_viewProjectionMatrix = viewProjMats[faceIdx];
					dsInfo.m_invViewProjectionMatrix = viewProjMats[faceIdx].getInverse();
					dsInfo.m_cameraPosWSpace = cellCenter.xyz1();
					dsInfo.m_viewport = UVec4(faceIdx * m_tileSize, 0, m_tileSize, m_tileSize);
					dsInfo.m_gbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize * 6), 1.0f / F32(m_tileSize));
					dsInfo.m_gbufferTexCoordsBias = Vec2(0.0f, 0.0f);
					dsInfo.m_lightbufferTexCoordsBias = Vec2(-F32(faceIdx), 0.0f);
					dsInfo.m_lightbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize), 1.0f / F32(m_tileSize));

					dsInfo.m_effectiveShadowDistance = (doShadows) ? probeToRefresh->getShadowsRenderRadius() : -1.0f;

					if(doShadows)
					{
						const F32 xScale = 1.0f / 6.0f;
						const F32 yScale = 1.0f;
						const F32 xOffset = F32(faceIdx) * (1.0f / 6.0f);
						const F32 yOffset = 0.0f;
						const Mat4 atlasMtx(xScale, 0.0f, 0.0f, xOffset, 0.0f, yScale, 0.0f, yOffset, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
						const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
						dsInfo.m_dirLightMatrix = atlasMtx * biasMat4 * cascadeViewProjMats[faceIdx];
					}
					else
					{
						dsInfo.m_dirLightMatrix = Mat4::getIdentity();
					}

					dsInfo.m_visibleLightsBuffer = visibleLightsBuffers[faceIdx];

					dsInfo.m_gbufferRenderTargets[0] = gbufferColorRts[0];
					dsInfo.m_gbufferRenderTargets[1] = gbufferColorRts[1];
					dsInfo.m_gbufferRenderTargets[2] = gbufferColorRts[2];
					dsInfo.m_gbufferDepthRenderTarget = gbufferDepthRt;
					dsInfo.m_directionalLightShadowmapRenderTarget = shadowsRt;
					dsInfo.m_renderpassContext = &rgraphCtx;

					m_lightShading.m_deferred.drawLights(dsInfo);
				}
			});
		}

		// Irradiance pass. First & 2nd bounce
		{
			ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GI irradiance");

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
