// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/PrimaryNonRenderableVisibility.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Resource/AsyncLoader.h>

namespace anki {

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
	m_tileSize = g_indirectDiffuseProbeTileResolutionCVar;

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
		RenderTargetDesc texinit =
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
	const U32 resolution = g_indirectDiffuseProbeShadowMapResolutionCVar;
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		getRenderer().create2DRenderTargetDescription(resolution, resolution, getRenderer().getDepthNoStencilFormat(), "GI SM");
	m_shadowMapping.m_rtDescr.bake();

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
	if(probeToRefresh == nullptr || AsyncLoader::getSingleton().getTasksInFlightCount() != 0) [[likely]]
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

	RenderGraphBuilder& rgraph = rctx.m_renderGraphDescr;

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
			Frustum frustum;
			{
				frustum.setPerspective(kClusterObjectFrustumNearPlane, probeToRefresh->getRenderRadius(), kPi / 2.0f, kPi / 2.0f);
				frustum.setWorldTransform(
					Transform(cellCenter.xyz0(), Frustum::getOmnidirectionalFrustumRotations()[f], Vec4(1.0f, 1.0f, 1.0f, 0.0f)));
				frustum.update();

				Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar, g_lod1MaxDistanceCVar};

				FrustumGpuVisibilityInput visIn;
				visIn.m_passesName = generateTempPassName("GI: GBuffer cell:%u face:%u", cellIdx, f);
				visIn.m_technique = RenderingTechnique::kGBuffer;
				visIn.m_viewProjectionMatrix = frustum.getViewProjectionMatrix();
				visIn.m_lodReferencePoint = cellCenter;
				visIn.m_lodDistances = lodDistances;
				visIn.m_rgraph = &rgraph;
				visIn.m_viewportSize = UVec2(m_tileSize);
				visIn.m_limitMemory = true;

				getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOut);
			}

			// GBuffer
			{
				GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("GI: GBuffer cell:%u face:%u", cellIdx, f));

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
				pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kAllRtvDsv,
										  TextureSubresourceDesc::firstSurface(DepthStencilAspectBit::kDepth));

				pass.newBufferDependency(visOut.m_dependency, BufferUsageBit::kIndirectDraw);

				pass.setWork([this, visOut, viewProjMat = frustum.getViewProjectionMatrix(),
							  viewMat = frustum.getViewMatrix()](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseGBuffer);
					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					cmdb.setViewport(0, 0, m_tileSize, m_tileSize);

					RenderableDrawerArguments args;
					args.m_viewMatrix = viewMat;
					args.m_cameraTransform = args.m_viewMatrix.invertTransformation();
					args.m_viewProjectionMatrix = viewProjMat;
					args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
					args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
					args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
					args.m_viewport = UVec4(0, 0, m_tileSize, m_tileSize);
					args.fill(visOut);

					getRenderer().getRenderableDrawer().drawMdi(args, cmdb);

					// It's secondary, no need to restore any state
				});
			}

			// Shadow visibility. Optional
			GpuVisibilityOutput shadowVisOut;
			Mat4 cascadeProjMat;
			Mat3x4 cascadeViewMat;
			Mat4 cascadeViewProjMat;
			if(doShadows)
			{
				constexpr U32 kCascadeCount = 1;
				dirLightc->computeCascadeFrustums(frustum, Array<F32, kCascadeCount>{probeToRefresh->getShadowsRenderRadius()},
												  WeakArray<Mat4>(&cascadeProjMat, kCascadeCount), WeakArray<Mat3x4>(&cascadeViewMat, kCascadeCount));

				cascadeViewProjMat = cascadeProjMat * Mat4(cascadeViewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));

				Array<F32, kMaxLodCount - 1> lodDistances = {g_lod0MaxDistanceCVar, g_lod1MaxDistanceCVar};

				FrustumGpuVisibilityInput visIn;
				visIn.m_passesName = generateTempPassName("GI: Shadows cell:%u face:%u", cellIdx, f);
				visIn.m_technique = RenderingTechnique::kDepth;
				visIn.m_viewProjectionMatrix = cascadeViewProjMat;
				visIn.m_lodReferencePoint = cellCenter;
				visIn.m_lodDistances = lodDistances;
				visIn.m_rgraph = &rgraph;
				visIn.m_viewportSize = UVec2(m_shadowMapping.m_rtDescr.m_height);
				visIn.m_limitMemory = true;

				getRenderer().getGpuVisibility().populateRenderGraph(visIn, shadowVisOut);
			}

			// Shadow pass. Optional
			if(doShadows)
			{
				// Create the pass
				GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("GI: Shadows cell:%u face:%u", cellIdx, f));

				GraphicsRenderPassTargetDesc depthRti(shadowsRt);
				depthRti.m_loadOperation = RenderTargetLoadOperation::kClear;
				depthRti.m_subresource.m_depthStencilAspect = DepthStencilAspectBit::kDepth;
				depthRti.m_clearValue.m_depthStencil.m_depth = 1.0f;
				pass.setRenderpassInfo({}, &depthRti);

				pass.newTextureDependency(shadowsRt, TextureUsageBit::kAllRtvDsv,
										  TextureSubresourceDesc::firstSurface(DepthStencilAspectBit::kDepth));
				pass.newBufferDependency(shadowVisOut.m_dependency, BufferUsageBit::kIndirectDraw);

				pass.setWork([this, shadowVisOut, cascadeViewProjMat, cascadeViewMat](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseShadows);
					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

					const U32 rez = m_shadowMapping.m_rtDescr.m_width;
					cmdb.setViewport(0, 0, rez, rez);

					RenderableDrawerArguments args;
					args.m_viewMatrix = cascadeViewMat;
					args.m_cameraTransform = cascadeViewMat.invertTransformation();
					args.m_viewProjectionMatrix = cascadeViewProjMat;
					args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
					args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
					args.m_renderingTechinuqe = RenderingTechnique::kDepth;
					args.m_viewport = UVec4(0, 0, rez, rez);
					args.fill(shadowVisOut);

					getRenderer().getRenderableDrawer().drawMdi(args, cmdb);

					cmdb.setPolygonOffset(0.0, 0.0);

					// It's secondary, no need to restore the state
				});
			}

			// Light visibility
			GpuVisibilityNonRenderablesOutput lightVis;
			{
				GpuVisibilityNonRenderablesInput in;
				in.m_passesName = generateTempPassName("GI: Light visibility cell:%u face:%u", cellIdx, f);
				in.m_objectType = GpuSceneNonRenderableObjectType::kLight;
				in.m_viewProjectionMat = frustum.getViewProjectionMatrix();
				in.m_rgraph = &rgraph;
				getRenderer().getGpuVisibilityNonRenderables().populateRenderGraph(in, lightVis);
			}

			// Light shading pass
			{
				GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("GI: Light shading cell:%u face:%u", cellIdx, f));

				GraphicsRenderPassTargetDesc colorRti(lightShadingRt);
				colorRti.m_loadOperation = RenderTargetLoadOperation::kClear;
				colorRti.m_subresource.m_face = f;
				pass.setRenderpassInfo({colorRti});

				pass.newBufferDependency(lightVis.m_visiblesBufferHandle, BufferUsageBit::kSrvPixel);

				pass.newTextureDependency(lightShadingRt, TextureUsageBit::kRtvDsvWrite, TextureSubresourceDesc::surface(0, f, 0));

				for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
				{
					pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSrvPixel, TextureSubresourceDesc::surface(0, f, 0));
				}
				pass.newTextureDependency(gbufferDepthRt, TextureUsageBit::kSrvPixel,
										  TextureSubresourceDesc::firstSurface(DepthStencilAspectBit::kDepth));

				if(shadowsRt.isValid())
				{
					pass.newTextureDependency(shadowsRt, TextureUsageBit::kSrvPixel);
				}

				if(getRenderer().getGeneratedSky().isEnabled())
				{
					pass.newTextureDependency(getRenderer().getGeneratedSky().getSkyLutRt(), TextureUsageBit::kSrvPixel);
				}

				pass.setWork([this, visibleLightsBuffer = lightVis.m_visiblesBuffer, viewProjMat = frustum.getViewProjectionMatrix(), cellCenter,
							  gbufferColorRts, gbufferDepthRt, probeToRefresh, cascadeViewProjMat, shadowsRt, faceIdx = f,
							  &rctx](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseLightShading);

					const LightComponent* dirLightc = SceneGraph::getSingleton().getDirectionalLight();
					const Bool doShadows = dirLightc && dirLightc->getShadowEnabled();

					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					const U32 rez = m_tileSize;
					cmdb.setViewport(0, 0, rez, rez);

					// Draw light shading
					TraditionalDeferredLightShadingDrawInfo dsInfo;
					dsInfo.m_viewProjectionMatrix = viewProjMat;
					dsInfo.m_invViewProjectionMatrix = viewProjMat.invert();
					dsInfo.m_cameraPosWSpace = cellCenter.xyz1();
					dsInfo.m_viewport = UVec4(0, 0, m_tileSize, m_tileSize);
					dsInfo.m_effectiveShadowDistance = (doShadows) ? probeToRefresh->getShadowsRenderRadius() : -1.0f;

					if(doShadows)
					{
						const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
						dsInfo.m_dirLightMatrix = biasMat4 * cascadeViewProjMat;
					}
					else
					{
						dsInfo.m_dirLightMatrix = Mat4::getIdentity();
					}

					dsInfo.m_visibleLightsBuffer = visibleLightsBuffer;

					dsInfo.m_gbufferRenderTargets[0] = gbufferColorRts[0];
					dsInfo.m_gbufferRenderTargetSubresource[0].m_face = faceIdx;
					dsInfo.m_gbufferRenderTargets[1] = gbufferColorRts[1];
					dsInfo.m_gbufferRenderTargetSubresource[1].m_face = faceIdx;
					dsInfo.m_gbufferRenderTargets[2] = gbufferColorRts[2];
					dsInfo.m_gbufferRenderTargetSubresource[2].m_face = faceIdx;
					dsInfo.m_gbufferDepthRenderTarget = gbufferDepthRt;
					dsInfo.m_directionalLightShadowmapRenderTarget = shadowsRt;
					dsInfo.m_skyLutRenderTarget =
						(getRenderer().getGeneratedSky().isEnabled()) ? getRenderer().getGeneratedSky().getSkyLutRt() : RenderTargetHandle();
					dsInfo.m_globalRendererConsts = rctx.m_globalRenderingConstantsBuffer;
					dsInfo.m_renderpassContext = &rgraphCtx;

					m_lightShading.m_deferred.drawLights(dsInfo);
				});
			}
		} // For all faces

		// Irradiance pass. First & 2nd bounce
		{
			NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass(generateTempPassName("GI: Irradiance cell:%u", cellIdx));

			pass.newTextureDependency(lightShadingRt, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(irradianceVolume, TextureUsageBit::kUavCompute);
			for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
			{
				pass.newTextureDependency(gbufferColorRts[i], TextureUsageBit::kSrvCompute);
			}

			pass.setWork([this, lightShadingRt, gbufferColorRts, irradianceVolume, cellIdx, probeToRefresh](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(IndirectDiffuseIrradiance);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_irradiance.m_grProg.get());

				// Bind resources
				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
				rgraphCtx.bindSrv(0, 0, lightShadingRt);

				for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
				{
					rgraphCtx.bindSrv(i + 1, 0, gbufferColorRts[i]);
				}

				rgraphCtx.bindUav(0, 0, irradianceVolume);

				class
				{
				public:
					IVec3 m_volumeTexel;
					I32 m_nextTexelOffsetInU;
				} consts;

				U32 x, y, z;
				unflatten3dArrayIndex(probeToRefresh->getCellCountsPerDimension().x(), probeToRefresh->getCellCountsPerDimension().y(),
									  probeToRefresh->getCellCountsPerDimension().z(), cellIdx, x, y, z);
				consts.m_volumeTexel = IVec3(x, probeToRefresh->getCellCountsPerDimension().y() - y - 1, z);

				consts.m_nextTexelOffsetInU = probeToRefresh->getCellCountsPerDimension().x();
				cmdb.setFastConstants(&consts, sizeof(consts));

				// Dispatch
				cmdb.dispatchCompute(1, 1, 1);
			});
		}

		probeToRefresh->incrementRefreshedCells(1);
		g_giProbeCellsRenderCountStatVar.increment(1);
	}
}

} // end namespace anki
