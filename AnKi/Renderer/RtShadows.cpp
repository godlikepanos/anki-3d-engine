// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki
{

RtShadows::~RtShadows()
{
}

Error RtShadows::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize ray traced shadows");
	}

	return Error::NONE;
}

Error RtShadows::initInternal(const ConfigSet& cfg)
{
	m_useSvgf = cfg.getNumberU8("r_rtShadowsSvgf") != 0;
	m_atrousPassCount = cfg.getNumberU8("r_rtShadowsSvgfAtrousPassCount");

	// Ray gen program
	ANKI_CHECK(getResourceManager().loadResource("Shaders/RtShadowsRayGen.ankiprog", m_rayGenProg));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_rayGenProg);
	variantInitInfo.addMutation("SVGF", m_useSvgf);
	const ShaderProgramResourceVariant* variant;
	m_rayGenProg->getOrCreateVariant(variantInitInfo, variant);
	m_rtLibraryGrProg = variant->getProgram();
	m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

	// Miss prog
	ANKI_CHECK(getResourceManager().loadResource("Shaders/RtShadowsMiss.ankiprog", m_missProg));
	m_missProg->getOrCreateVariant(variant);
	m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();

	// Denoise program
	if(!m_useSvgf)
	{
		ANKI_CHECK(getResourceManager().loadResource("Shaders/RtShadowsDenoise.ankiprog", m_denoiseProg));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_denoiseProg);
		variantInitInfo.addConstant("OUT_IMAGE_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));
		variantInitInfo.addConstant("SAMPLE_COUNT", 8u);
		variantInitInfo.addConstant("SPIRAL_TURN_COUNT", 27u);
		variantInitInfo.addConstant("PIXEL_RADIUS", 12u);

		m_denoiseProg->getOrCreateVariant(variantInitInfo, variant);
		m_grDenoiseProg = variant->getProgram();
	}

	// SVGF variance program
	if(m_useSvgf)
	{
		ANKI_CHECK(getResourceManager().loadResource("Shaders/RtShadowsSvgfVariance.ankiprog", m_svgfVarianceProg));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_svgfVarianceProg);
		variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getWidth() / 2, m_r->getHeight() / 2));

		m_svgfVarianceProg->getOrCreateVariant(variantInitInfo, variant);
		m_svgfVarianceGrProg = variant->getProgram();
	}

	// SVGF atrous program
	if(m_useSvgf)
	{
		ANKI_CHECK(getResourceManager().loadResource("Shaders/RtShadowsSvgfAtrous.ankiprog", m_svgfAtrousProg));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_svgfAtrousProg);
		variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getWidth() / 2, m_r->getHeight() / 2));
		variantInitInfo.addMutation("LAST_PASS", 0);

		m_svgfAtrousProg->getOrCreateVariant(variantInitInfo, variant);
		m_svgfAtrousGrProg = variant->getProgram();

		variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));
		variantInitInfo.addMutation("LAST_PASS", 1);
		m_svgfAtrousProg->getOrCreateVariant(variantInitInfo, variant);
		m_svgfAtrousLastPassGrProg = variant->getProgram();
	}

	// Debug program
	ANKI_CHECK(getResourceManager().loadResource("Shaders/RtShadowsVisualizeRenderTarget.ankiprog",
												 m_visualizeRenderTargetsProg));

	// Shadow RT
	{
		TextureInitInfo texinit =
			m_r->create2DRenderTargetInitInfo(m_r->getWidth(), m_r->getHeight(), Format::R32G32_UINT,
											  TextureUsageBit::ALL_SAMPLED | TextureUsageBit::IMAGE_TRACE_RAYS_WRITE
												  | TextureUsageBit::IMAGE_COMPUTE_WRITE,
											  "RtShadows");
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_historyAndFinalRt = m_r->createAndClearRenderTarget(texinit);
	}

	// Render RT
	{
		m_intermediateShadowsRtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth() / 2, m_r->getHeight() / 2,
																			Format::R32G32_UINT, "RtShadows Tmp");
		m_intermediateShadowsRtDescr.bake();
	}

	// Moments RT
	if(m_useSvgf)
	{
		TextureInitInfo texinit =
			m_r->create2DRenderTargetInitInfo(m_r->getWidth() / 2, m_r->getHeight() / 2, Format::R32G32_SFLOAT,
											  TextureUsageBit::ALL_SAMPLED | TextureUsageBit::IMAGE_TRACE_RAYS_WRITE
												  | TextureUsageBit::IMAGE_COMPUTE_WRITE,
											  "RtShadows Moments #1");
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_momentsRts[0] = m_r->createAndClearRenderTarget(texinit);

		texinit.setName("RtShadows Moments #2");
		m_momentsRts[1] = m_r->createAndClearRenderTarget(texinit);
	}

	// History len RT
	if(m_useSvgf)
	{
		TextureInitInfo texinit =
			m_r->create2DRenderTargetInitInfo(m_r->getWidth() / 2, m_r->getHeight() / 2, Format::R8_UNORM,
											  TextureUsageBit::ALL_SAMPLED | TextureUsageBit::IMAGE_TRACE_RAYS_WRITE
												  | TextureUsageBit::IMAGE_COMPUTE_WRITE,
											  "RtShadows History Length #1");
		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_historyLengthRts[0] = m_r->createAndClearRenderTarget(texinit);

		texinit.setName("RtShadows History Length #2");
		m_historyLengthRts[1] = m_r->createAndClearRenderTarget(texinit);
	}

	// Variance RT
	if(m_useSvgf)
	{
		m_varianceRtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth() / 2, m_r->getHeight() / 2,
																 Format::R32_SFLOAT, "RtShadows Variance");
		m_varianceRtDescr.bake();
	}

	// Misc
	m_sbtRecordSize = getAlignedRoundUp(getGrManager().getDeviceCapabilities().m_sbtRecordAlignment, m_sbtRecordSize);

	return Error::NONE;
}

void RtShadows::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(R_RT_SHADOWS);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	buildSbt();
	const U32 prevRtIdx = m_r->getFrameCount() & 1;

	// Import RTs
	{
		if(ANKI_UNLIKELY(!m_rtsImportedOnce))
		{
			m_runCtx.m_historyAndFinalRt =
				rgraph.importRenderTarget(m_historyAndFinalRt, TextureUsageBit::SAMPLED_FRAGMENT);

			if(m_useSvgf)
			{
				m_runCtx.m_prevMomentsRt =
					rgraph.importRenderTarget(m_momentsRts[prevRtIdx], TextureUsageBit::SAMPLED_FRAGMENT);

				m_runCtx.m_prevHistoryLengthRt =
					rgraph.importRenderTarget(m_historyLengthRts[prevRtIdx], TextureUsageBit::SAMPLED_FRAGMENT);
			}

			m_rtsImportedOnce = true;
		}
		else
		{
			m_runCtx.m_historyAndFinalRt = rgraph.importRenderTarget(m_historyAndFinalRt);

			if(m_useSvgf)
			{
				m_runCtx.m_prevMomentsRt = rgraph.importRenderTarget(m_momentsRts[prevRtIdx]);
				m_runCtx.m_prevHistoryLengthRt = rgraph.importRenderTarget(m_historyLengthRts[prevRtIdx]);
			}
		}

		m_runCtx.m_intermediateShadowsRts[0] = rgraph.newRenderTarget(m_intermediateShadowsRtDescr);

		if(m_useSvgf)
		{
			m_runCtx.m_intermediateShadowsRts[1] = rgraph.newRenderTarget(m_intermediateShadowsRtDescr);

			m_runCtx.m_currentMomentsRt = rgraph.importRenderTarget(m_momentsRts[!prevRtIdx], TextureUsageBit::NONE);
			m_runCtx.m_currentHistoryLengthRt =
				rgraph.importRenderTarget(m_historyLengthRts[!prevRtIdx], TextureUsageBit::NONE);

			if(m_atrousPassCount > 1)
			{
				m_runCtx.m_varianceRts[0] = rgraph.newRenderTarget(m_varianceRtDescr);
			}
			m_runCtx.m_varianceRts[1] = rgraph.newRenderTarget(m_varianceRtDescr);
		}
	}

	// RT shadows pass
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<RtShadows*>(rgraphCtx.m_userData)->run(rgraphCtx); },
			this, 0);

		rpass.newDependency(RenderPassDependency(m_runCtx.m_historyAndFinalRt, TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::IMAGE_TRACE_RAYS_WRITE));
		rpass.newDependency(
			RenderPassDependency(m_r->getAccelerationStructureBuilder().getAccelerationStructureHandle(),
								 AccelerationStructureUsageBit::TRACE_RAYS_READ));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(
			RenderPassDependency(m_r->getMotionVectors().getMotionVectorsRt(), TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(
			RenderPassDependency(m_r->getMotionVectors().getRejectionFactorRt(), TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_TRACE_RAYS));

		if(m_useSvgf)
		{
			rpass.newDependency(RenderPassDependency(m_runCtx.m_prevMomentsRt, TextureUsageBit::SAMPLED_TRACE_RAYS));
			rpass.newDependency(
				RenderPassDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::IMAGE_TRACE_RAYS_WRITE));

			rpass.newDependency(
				RenderPassDependency(m_runCtx.m_prevHistoryLengthRt, TextureUsageBit::SAMPLED_TRACE_RAYS));
			rpass.newDependency(
				RenderPassDependency(m_runCtx.m_currentHistoryLengthRt, TextureUsageBit::IMAGE_TRACE_RAYS_WRITE));
		}
	}

	// Denoise pass
	if(!m_useSvgf)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows Denoise");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<RtShadows*>(rgraphCtx.m_userData)->runDenoise(rgraphCtx);
			},
			this, 0);

		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE));

		rpass.newDependency(RenderPassDependency(m_runCtx.m_historyAndFinalRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));
	}

	// Variance calculation pass
	if(m_useSvgf)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows SVGF Variance");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<RtShadows*>(rgraphCtx.m_userData)->runSvgfVariance(rgraphCtx);
			},
			this, 0);

		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_currentHistoryLengthRt, TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE));

		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::IMAGE_COMPUTE_WRITE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_varianceRts[1], TextureUsageBit::IMAGE_COMPUTE_WRITE));
	}

	// SVGF Atrous
	if(m_useSvgf)
	{
		m_runCtx.m_atrousPassIdx = 0;

		for(U32 i = 0; i < m_atrousPassCount; ++i)
		{
			const Bool lastPass = i == U32(m_atrousPassCount - 1);
			const U32 readRtIdx = (i + 1) & 1;

			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows SVGF Atrous");
			rpass.setWork(
				[](RenderPassWorkContext& rgraphCtx) {
					static_cast<RtShadows*>(rgraphCtx.m_userData)->runSvgfAtrous(rgraphCtx);
				},
				this, 0);

			rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE));
			rpass.newDependency(
				RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE));
			rpass.newDependency(
				RenderPassDependency(m_runCtx.m_intermediateShadowsRts[readRtIdx], TextureUsageBit::SAMPLED_COMPUTE));
			rpass.newDependency(
				RenderPassDependency(m_runCtx.m_varianceRts[readRtIdx], TextureUsageBit::SAMPLED_COMPUTE));

			if(lastPass)
			{
				rpass.newDependency(
					RenderPassDependency(m_runCtx.m_historyAndFinalRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));
			}
			else
			{
				rpass.newDependency(RenderPassDependency(m_runCtx.m_intermediateShadowsRts[!readRtIdx],
														 TextureUsageBit::IMAGE_COMPUTE_WRITE));
				rpass.newDependency(
					RenderPassDependency(m_runCtx.m_varianceRts[!readRtIdx], TextureUsageBit::IMAGE_COMPUTE_WRITE));
			}
		}
	}

	// Find out the lights that will take part in RT pass
	{
		RenderQueue& rqueue = *m_runCtx.m_ctx->m_renderQueue;
		m_runCtx.m_layersWithRejectedHistory.unsetAll();

		if(rqueue.m_directionalLight.hasShadow())
		{
			U32 layerIdx;
			Bool rejectHistory;
			const Bool layerFound = findShadowLayer(0, layerIdx, rejectHistory);
			(void)layerFound;
			ANKI_ASSERT(layerFound && "Directional can't fail");

			rqueue.m_directionalLight.m_shadowLayer = U8(layerIdx);
			ANKI_ASSERT(rqueue.m_directionalLight.m_shadowLayer < MAX_RT_SHADOW_LAYERS);
			m_runCtx.m_layersWithRejectedHistory.set(layerIdx, rejectHistory);
		}

		for(PointLightQueueElement& light : rqueue.m_pointLights)
		{
			if(!light.hasShadow())
			{
				continue;
			}

			U32 layerIdx;
			Bool rejectHistory;
			const Bool layerFound = findShadowLayer(light.m_uuid, layerIdx, rejectHistory);

			if(layerFound)
			{
				light.m_shadowLayer = U8(layerIdx);
				ANKI_ASSERT(light.m_shadowLayer < MAX_RT_SHADOW_LAYERS);
				m_runCtx.m_layersWithRejectedHistory.set(layerIdx, rejectHistory);
			}
			else
			{
				// Disable shadows
				light.m_shadowRenderQueues = {};
			}
		}

		for(SpotLightQueueElement& light : rqueue.m_spotLights)
		{
			if(!light.hasShadow())
			{
				continue;
			}

			U32 layerIdx;
			Bool rejectHistory;
			const Bool layerFound = findShadowLayer(light.m_uuid, layerIdx, rejectHistory);

			if(layerFound)
			{
				light.m_shadowLayer = U8(layerIdx);
				ANKI_ASSERT(light.m_shadowLayer < MAX_RT_SHADOW_LAYERS);
				m_runCtx.m_layersWithRejectedHistory.set(layerIdx, rejectHistory);
			}
			else
			{
				// Disable shadows
				light.m_shadowRenderQueue = nullptr;
			}
		}
	}
}

void RtShadows::run(RenderPassWorkContext& rgraphCtx)
{
	const RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;

	cmdb->bindShaderProgram(m_rtLibraryGrProg);

	bindUniforms(cmdb, 0, 0, ctx.m_lightShadingUniformsToken);

	bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 3, m_r->getShadowMapping().getShadowmapRt());

	bindStorage(cmdb, 0, 4, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 5, rsrc.m_indicesToken);

	cmdb->bindSampler(0, 6, m_r->getSamplers().m_trilinearRepeat);

	rgraphCtx.bindImage(0, 7, m_runCtx.m_intermediateShadowsRts[0]);

	rgraphCtx.bindColorTexture(0, 8, m_runCtx.m_historyAndFinalRt);
	cmdb->bindSampler(0, 9, m_r->getSamplers().m_trilinearClamp);
	cmdb->bindSampler(0, 10, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindTexture(0, 11, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 12, m_r->getMotionVectors().getMotionVectorsRt());
	rgraphCtx.bindColorTexture(0, 13, m_r->getMotionVectors().getRejectionFactorRt());
	rgraphCtx.bindColorTexture(0, 14, m_r->getGBuffer().getColorRt(2));
	rgraphCtx.bindAccelerationStructure(0, 15, m_r->getAccelerationStructureBuilder().getAccelerationStructureHandle());

	if(!m_useSvgf)
	{
		// Bind something random
		rgraphCtx.bindColorTexture(0, 16, m_r->getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindImage(0, 17, m_runCtx.m_intermediateShadowsRts[0]);
		rgraphCtx.bindColorTexture(0, 18, m_r->getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindImage(0, 19, m_runCtx.m_intermediateShadowsRts[0]);
	}
	else
	{
		rgraphCtx.bindColorTexture(0, 16, m_runCtx.m_prevHistoryLengthRt);
		rgraphCtx.bindImage(0, 17, m_runCtx.m_currentHistoryLengthRt);
		rgraphCtx.bindColorTexture(0, 18, m_runCtx.m_prevMomentsRt);
		rgraphCtx.bindImage(0, 19, m_runCtx.m_currentMomentsRt);
	}

	cmdb->bindAllBindless(1);

	RtShadowsUniforms unis;
	for(U32 i = 0; i < MAX_RT_SHADOW_LAYERS; ++i)
	{
		unis.historyRejectFactor[i] = F32(m_runCtx.m_layersWithRejectedHistory.get(i));
	}
	cmdb->setPushConstants(&unis, sizeof(unis));

	cmdb->traceRays(m_runCtx.m_sbtBuffer, m_runCtx.m_sbtOffset, m_sbtRecordSize, m_runCtx.m_hitGroupCount, 1,
					m_r->getWidth() / 2, m_r->getHeight() / 2, 1);
}

void RtShadows::runDenoise(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grDenoiseProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_intermediateShadowsRts[0]);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	rgraphCtx.bindImage(0, 4, m_runCtx.m_historyAndFinalRt);

	RtShadowsDenoiseUniforms unis;
	unis.invViewProjMat = m_runCtx.m_ctx->m_matrices.m_invertedViewProjectionJitter;
	unis.time = F32(m_r->getGlobalTimestamp());
	cmdb->setPushConstants(&unis, sizeof(unis));

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth(), m_r->getHeight());
}

void RtShadows::runSvgfVariance(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_svgfVarianceGrProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindColorTexture(0, 2, m_runCtx.m_intermediateShadowsRts[0]);
	rgraphCtx.bindColorTexture(0, 3, m_runCtx.m_currentMomentsRt);
	rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_currentHistoryLengthRt);
	rgraphCtx.bindTexture(0, 5, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 6, m_r->getGBuffer().getColorRt(2));

	rgraphCtx.bindImage(0, 7, m_runCtx.m_intermediateShadowsRts[1]);
	rgraphCtx.bindImage(0, 8, m_runCtx.m_varianceRts[1]);

	const Mat4& invViewProjMat = m_runCtx.m_ctx->m_matrices.m_invertedViewProjectionJitter;
	cmdb->setPushConstants(&invViewProjMat, sizeof(invViewProjMat));

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth() / 2, m_r->getHeight() / 2);
}

void RtShadows::runSvgfAtrous(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const Bool lastPass = m_runCtx.m_atrousPassIdx == m_atrousPassCount - 1;
	const U32 readRtIdx = (m_runCtx.m_atrousPassIdx + 1) & 1;

	if(lastPass)
	{
		cmdb->bindShaderProgram(m_svgfAtrousLastPassGrProg);
	}
	else
	{
		cmdb->bindShaderProgram(m_svgfAtrousGrProg);
	}

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));
	rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_intermediateShadowsRts[readRtIdx]);
	rgraphCtx.bindColorTexture(0, 5, m_runCtx.m_varianceRts[readRtIdx]);

	if(lastPass)
	{
		rgraphCtx.bindImage(0, 6, m_runCtx.m_historyAndFinalRt);
	}
	else
	{
		rgraphCtx.bindImage(0, 6, m_runCtx.m_intermediateShadowsRts[!readRtIdx]);
		rgraphCtx.bindImage(0, 7, m_runCtx.m_varianceRts[!readRtIdx]);
	}

	const U32 width = (lastPass) ? m_r->getWidth() : m_r->getWidth() / 2;
	const U32 height = (lastPass) ? m_r->getHeight() : m_r->getHeight() / 2;

	const Mat4& invViewProjMat = m_runCtx.m_ctx->m_matrices.m_invertedViewProjectionJitter;
	cmdb->setPushConstants(&invViewProjMat, sizeof(invViewProjMat));

	dispatchPPCompute(cmdb, 8, 8, width, height);

	++m_runCtx.m_atrousPassIdx;
}

void RtShadows::buildSbt()
{
	// Get some things
	RenderingContext& ctx = *m_runCtx.m_ctx;

	ANKI_ASSERT(ctx.m_renderQueue->m_rayTracingQueue);
	ConstWeakArray<RayTracingInstanceQueueElement> instanceElements =
		ctx.m_renderQueue->m_rayTracingQueue->m_rayTracingInstances;
	const U32 instanceCount = instanceElements.getSize();
	ANKI_ASSERT(instanceCount > 0);

	const U32 shaderHandleSize = getGrManager().getDeviceCapabilities().m_shaderGroupHandleSize;

	const U32 extraSbtRecords = 1 + 1; // Raygen + miss

	m_runCtx.m_hitGroupCount = instanceCount;

	// Allocate SBT
	StagingGpuMemoryToken token;
	U8* sbt = allocateStorage<U8*>(PtrSize(m_sbtRecordSize) * (instanceCount + extraSbtRecords), token);
	const U8* sbtStart = sbt;
	(void)sbtStart;
	m_runCtx.m_sbtBuffer = token.m_buffer;
	m_runCtx.m_sbtOffset = token.m_offset;

	// Set the miss and ray gen handles
	ConstWeakArray<U8> shaderGroupHandles = m_rtLibraryGrProg->getShaderGroupHandles();
	memcpy(sbt, &shaderGroupHandles[m_rayGenShaderGroupIdx * shaderHandleSize], shaderHandleSize);
	sbt += m_sbtRecordSize;
	memcpy(sbt, &shaderGroupHandles[m_missShaderGroupIdx * shaderHandleSize], shaderHandleSize);
	sbt += m_sbtRecordSize;

	// Init SBT and instances
	ANKI_ASSERT(m_sbtRecordSize >= shaderHandleSize + sizeof(ModelGpuDescriptor));
	for(U32 instanceIdx = 0; instanceIdx < instanceCount; ++instanceIdx)
	{
		const RayTracingInstanceQueueElement& element = instanceElements[instanceIdx];

		// Init SBT record
		memcpy(sbt, &shaderGroupHandles[element.m_shaderGroupHandleIndices[RayType::SHADOWS] * shaderHandleSize],
			   shaderHandleSize);
		memcpy(sbt + shaderHandleSize, &element.m_modelDescriptor, sizeof(element.m_modelDescriptor));
		sbt += m_sbtRecordSize;
	}

	ANKI_ASSERT(sbtStart + m_sbtRecordSize * (instanceCount + extraSbtRecords) == sbt);
}

Bool RtShadows::findShadowLayer(U64 lightUuid, U32& layerIdx, Bool& rejectHistoryBuffer)
{
	const U64 crntFrame = m_r->getFrameCount();
	layerIdx = MAX_U32;
	U32 nextBestLayerIdx = MAX_U32;
	U64 nextBestLayerFame = crntFrame;
	rejectHistoryBuffer = false;

	for(U32 i = 0; i < m_shadowLayers.getSize(); ++i)
	{
		ShadowLayer& layer = m_shadowLayers[i];
		if(layer.m_lightUuid == lightUuid && layer.m_frameLastUsed == crntFrame - 1)
		{
			// Found it being used last frame
			layerIdx = i;
			layer.m_frameLastUsed = crntFrame;
			layer.m_lightUuid = lightUuid;
			break;
		}
		else if(layer.m_lightUuid == lightUuid || layer.m_frameLastUsed == MAX_U64)
		{
			// Found an empty slot or slot used by the same light
			layerIdx = i;
			layer.m_frameLastUsed = crntFrame;
			layer.m_lightUuid = lightUuid;
			rejectHistoryBuffer = true;
			break;
		}
		else if(layer.m_frameLastUsed < nextBestLayerFame)
		{
			nextBestLayerIdx = i;
			nextBestLayerFame = crntFrame;
		}
	}

	// Not found but there is a good candidate. Use that
	if(layerIdx == MAX_U32 && nextBestLayerIdx != MAX_U32)
	{
		layerIdx = nextBestLayerIdx;
		m_shadowLayers[nextBestLayerIdx].m_frameLastUsed = crntFrame;
		m_shadowLayers[nextBestLayerIdx].m_lightUuid = lightUuid;
		rejectHistoryBuffer = true;
	}

	return layerIdx != MAX_U32;
}

void RtShadows::getDebugRenderTarget(CString rtName, RenderTargetHandle& handle,
									 ShaderProgramPtr& optionalShaderProgram) const
{
	U32 layerGroup = 0;
	if(rtName == "RtShadows")
	{
		layerGroup = 0;
	}
	else if(rtName == "RtShadows1")
	{
		layerGroup = 1;
	}
	else
	{
		ANKI_ASSERT(rtName == "RtShadows2");
		layerGroup = 2;
	}

	handle = m_runCtx.m_historyAndFinalRt;

	ShaderProgramResourceVariantInitInfo variantInit(m_visualizeRenderTargetsProg);
	variantInit.addMutation("LAYER_GROUP", layerGroup);

	const ShaderProgramResourceVariant* variant;
	m_visualizeRenderTargetsProg->getOrCreateVariant(variantInit, variant);
	optionalShaderProgram = variant->getProgram();
}

} // end namespace anki
