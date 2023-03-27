// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

class IndirectDiffuseProbes::InternalContext
{
public:
	IndirectDiffuseProbes* m_gi = nullptr;
	RenderingContext* m_ctx = nullptr;

	GlobalIlluminationProbeQueueElementForRefresh* m_probeToUpdateThisFrame = nullptr;

	Array<RenderTargetHandle, kGBufferColorRenderTargetCount> m_gbufferColorRts;
	RenderTargetHandle m_gbufferDepthRt;
	RenderTargetHandle m_shadowsRt;
	RenderTargetHandle m_lightShadingRt;
	RenderTargetHandle m_irradianceVolume;

	U32 m_gbufferDrawcallCount = kMaxU32;
	U32 m_smDrawcallCount = kMaxU32;

	static void foo()
	{
		static_assert(std::is_trivially_destructible<InternalContext>::value, "See file");
	}
};

IndirectDiffuseProbes::~IndirectDiffuseProbes()
{
}

RenderTargetHandle IndirectDiffuseProbes::getCurrentlyRefreshedVolumeRt() const
{
	ANKI_ASSERT(m_giCtx && m_giCtx->m_irradianceVolume.isValid());
	return m_giCtx->m_irradianceVolume;
}

Bool IndirectDiffuseProbes::hasCurrentlyRefreshedVolumeRt() const
{
	return m_giCtx != nullptr;
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
	m_tileSize = ConfigSet::getSingleton().getRIndirectDiffuseProbeTileResolution();

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
		RenderTargetDescription texinit = m_r->create2DRenderTargetDescription(
			m_tileSize * 6, m_tileSize, kGBufferColorRenderTargetFormats[0], "GI GBuffer");

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(
				StringRaii(&getMemoryPool()).sprintf("GI GBuff Col #%u", i).toCString());
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = m_r->getDepthNoStencilFormat();
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
	const U32 resolution = ConfigSet::getSingleton().getRIndirectDiffuseProbeShadowMapResolution();
	ANKI_ASSERT(resolution > 8);

	// RT descr
	m_shadowMapping.m_rtDescr =
		m_r->create2DRenderTargetDescription(resolution * 6, resolution, m_r->getDepthNoStencilFormat(), "GI SM");
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
		m_lightShading.m_rtDescr =
			m_r->create2DRenderTargetDescription(m_tileSize * 6, m_tileSize, m_r->getHdrFormat(), "GI LS");
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
	ANKI_CHECK(
		ResourceManager::getSingleton().loadResource("ShaderBinaries/IrradianceDice.ankiprogbin", m_irradiance.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_irradiance.m_prog);
	variantInitInfo.addMutation("WORKGROUP_SIZE_XY", m_tileSize);
	variantInitInfo.addMutation("LIGHT_SHADING_TEX", 0);
	variantInitInfo.addMutation("STORE_LOCATION", 0);
	variantInitInfo.addMutation("SECOND_BOUNCE", 1);

	const ShaderProgramResourceVariant* variant;
	m_irradiance.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_irradiance.m_grProg = variant->getProgram();

	return Error::kNone;
}

void IndirectDiffuseProbes::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

	if(rctx.m_renderQueue->m_giProbeForRefresh == nullptr) [[likely]]
	{
		m_giCtx = nullptr;
		return;
	}

	InternalContext* giCtx = newInstance<InternalContext>(*rctx.m_tempPool);
	m_giCtx = giCtx;
	giCtx->m_gi = this;
	giCtx->m_ctx = &rctx;
	giCtx->m_probeToUpdateThisFrame = rctx.m_renderQueue->m_giProbeForRefresh;
	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	// Compute task counts for some of the passes
	U32 gbufferTaskCount, smTaskCount;
	{
		giCtx->m_gbufferDrawcallCount = 0;
		giCtx->m_smDrawcallCount = 0;
		for(const RenderQueue* rq : giCtx->m_probeToUpdateThisFrame->m_renderQueues)
		{
			ANKI_ASSERT(rq);
			giCtx->m_gbufferDrawcallCount += rq->m_renderables.getSize();

			if(rq->m_directionalLight.hasShadow())
			{
				giCtx->m_smDrawcallCount += rq->m_directionalLight.m_shadowRenderQueues[0]->m_renderables.getSize();
			}
		}

		gbufferTaskCount = computeNumberOfSecondLevelCommandBuffers(giCtx->m_gbufferDrawcallCount);
		smTaskCount = computeNumberOfSecondLevelCommandBuffers(giCtx->m_smDrawcallCount);
	}

	// GBuffer
	{
		// RTs
		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			giCtx->m_gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
		}
		giCtx->m_gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI gbuff");
		pass.setFramebufferInfo(m_gbuffer.m_fbDescr, giCtx->m_gbufferColorRts, giCtx->m_gbufferDepthRt);
		pass.setWork(gbufferTaskCount, [this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runGBufferInThread(rgraphCtx, *giCtx);
		});

		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			pass.newTextureDependency(giCtx->m_gbufferColorRts[i], TextureUsageBit::kFramebufferWrite);
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(giCtx->m_gbufferDepthRt, TextureUsageBit::kAllFramebuffer, subresource);

		pass.newBufferDependency(m_r->getGpuSceneBufferHandle(),
								 BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead);
	}

	// Shadow pass. Optional
	if(giCtx->m_probeToUpdateThisFrame->m_renderQueues[0]->m_directionalLight.m_uuid
	   && giCtx->m_probeToUpdateThisFrame->m_renderQueues[0]->m_directionalLight.m_shadowCascadeCount > 0)
	{
		// Update light matrices
		for(U i = 0; i < 6; ++i)
		{
			ANKI_ASSERT(giCtx->m_probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_uuid
						&& giCtx->m_probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_shadowCascadeCount
							   == 1);

			const F32 xScale = 1.0f / 6.0f;
			const F32 yScale = 1.0f;
			const F32 xOffset = F32(i) * (1.0f / 6.0f);
			const F32 yOffset = 0.0f;
			const Mat4 atlasMtx(xScale, 0.0f, 0.0f, xOffset, 0.0f, yScale, 0.0f, yOffset, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f);

			Mat4& lightMat =
				giCtx->m_probeToUpdateThisFrame->m_renderQueues[i]->m_directionalLight.m_textureMatrices[0];
			lightMat = atlasMtx * lightMat;
		}

		// RT
		giCtx->m_shadowsRt = rgraph.newRenderTarget(m_shadowMapping.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI SM");
		pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, giCtx->m_shadowsRt);
		pass.setWork(smTaskCount, [this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runShadowmappingInThread(rgraphCtx, *giCtx);
		});

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(giCtx->m_shadowsRt, TextureUsageBit::kAllFramebuffer, subresource);

		pass.newBufferDependency(m_r->getGpuSceneBufferHandle(),
								 BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead);
	}
	else
	{
		giCtx->m_shadowsRt = {};
	}

	// Light shading pass
	{
		// RT
		giCtx->m_lightShadingRt = rgraph.newRenderTarget(m_lightShading.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("GI LS");
		pass.setFramebufferInfo(m_lightShading.m_fbDescr, {giCtx->m_lightShadingRt});
		pass.setWork(1, [this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runLightShading(rgraphCtx, *giCtx);
		});

		pass.newTextureDependency(giCtx->m_lightShadingRt, TextureUsageBit::kFramebufferWrite);

		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			pass.newTextureDependency(giCtx->m_gbufferColorRts[i], TextureUsageBit::kSampledFragment);
		}
		pass.newTextureDependency(giCtx->m_gbufferDepthRt, TextureUsageBit::kSampledFragment,
								  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		if(giCtx->m_shadowsRt.isValid())
		{
			pass.newTextureDependency(giCtx->m_shadowsRt, TextureUsageBit::kSampledFragment);
		}
	}

	// Irradiance pass. First & 2nd bounce
	{
		m_giCtx->m_irradianceVolume = rgraph.importRenderTarget(
			TexturePtr(m_giCtx->m_probeToUpdateThisFrame->m_volumeTexture), TextureUsageBit::kNone);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("GI IR");

		pass.setWork([this, giCtx](RenderPassWorkContext& rgraphCtx) {
			runIrradiance(rgraphCtx, *giCtx);
		});

		pass.newTextureDependency(giCtx->m_lightShadingRt, TextureUsageBit::kSampledCompute);

		for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
		{
			pass.newTextureDependency(giCtx->m_gbufferColorRts[i], TextureUsageBit::kSampledCompute);
		}

		pass.newTextureDependency(m_giCtx->m_irradianceVolume, TextureUsageBit::kImageComputeWrite);
	}
}

void IndirectDiffuseProbes::runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const GlobalIlluminationProbeQueueElementForRefresh& probe = *giCtx.m_probeToUpdateThisFrame;

	I32 start, end;
	U32 startu, endu;
	splitThreadedProblem(rgraphCtx.m_currentSecondLevelCommandBufferIndex, rgraphCtx.m_secondLevelCommandBufferCount,
						 giCtx.m_gbufferDrawcallCount, startu, endu);
	start = I32(startu);
	end = I32(endu);

	I32 drawcallCount = 0;
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		const I32 faceDrawcallCount = I32(probe.m_renderQueues[faceIdx]->m_renderables.getSize());
		const I32 localStart = max(I32(0), start - drawcallCount);
		const I32 localEnd = min(faceDrawcallCount, end - drawcallCount);

		if(localStart < localEnd)
		{
			const U32 viewportX = faceIdx * m_tileSize;
			cmdb->setViewport(viewportX, 0, m_tileSize, m_tileSize);
			cmdb->setScissor(viewportX, 0, m_tileSize, m_tileSize);

			const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

			ANKI_ASSERT(localStart >= 0 && localEnd <= faceDrawcallCount);

			RenderableDrawerArguments args;
			args.m_viewMatrix = rqueue.m_viewMatrix;
			args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
			args.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
			args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
			args.m_sampler = m_r->getSamplers().m_trilinearRepeat;

			m_r->getSceneDrawer().drawRange(args, rqueue.m_renderables.getBegin() + localStart,
											rqueue.m_renderables.getBegin() + localEnd, cmdb);
		}

		drawcallCount += faceDrawcallCount;
	}
	ANKI_ASSERT(giCtx.m_gbufferDrawcallCount == U32(drawcallCount));

	// It's secondary, no need to restore the state
}

void IndirectDiffuseProbes::runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const
{
	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

	const GlobalIlluminationProbeQueueElementForRefresh& probe = *giCtx.m_probeToUpdateThisFrame;

	I32 start, end;
	U32 startu, endu;
	splitThreadedProblem(rgraphCtx.m_currentSecondLevelCommandBufferIndex, rgraphCtx.m_secondLevelCommandBufferCount,
						 giCtx.m_smDrawcallCount, startu, endu);
	start = I32(startu);
	end = I32(endu);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

	I32 drawcallCount = 0;
	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& faceRenderQueue = *probe.m_renderQueues[faceIdx];
		ANKI_ASSERT(faceRenderQueue.m_directionalLight.hasShadow());
		ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0]);
		const RenderQueue& cascadeRenderQueue = *faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0];

		const I32 faceDrawcallCount = I32(cascadeRenderQueue.m_renderables.getSize());
		const I32 localStart = max(I32(0), start - drawcallCount);
		const I32 localEnd = min(faceDrawcallCount, end - drawcallCount);

		if(localStart < localEnd)
		{
			const U32 rez = m_shadowMapping.m_rtDescr.m_height;
			cmdb->setViewport(rez * faceIdx, 0, rez, rez);
			cmdb->setScissor(rez * faceIdx, 0, rez, rez);

			ANKI_ASSERT(localStart >= 0 && localEnd <= faceDrawcallCount);

			RenderableDrawerArguments args;
			args.m_viewMatrix = cascadeRenderQueue.m_viewMatrix;
			args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
			args.m_viewProjectionMatrix = cascadeRenderQueue.m_viewProjectionMatrix;
			args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
			args.m_sampler = m_r->getSamplers().m_trilinearRepeatAniso;

			m_r->getSceneDrawer().drawRange(args, cascadeRenderQueue.m_renderables.getBegin() + localStart,
											cascadeRenderQueue.m_renderables.getBegin() + localEnd, cmdb);
		}
	}

	// It's secondary, no need to restore the state
}

void IndirectDiffuseProbes::runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
{
	ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	const GlobalIlluminationProbeQueueElementForRefresh& probe = *giCtx.m_probeToUpdateThisFrame;

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		ANKI_ASSERT(probe.m_renderQueues[faceIdx]);
		const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

		const U32 rez = m_tileSize;
		cmdb->setScissor(rez * faceIdx, 0, rez, rez);
		cmdb->setViewport(rez * faceIdx, 0, rez, rez);

		// Draw light shading
		TraditionalDeferredLightShadingDrawInfo dsInfo;
		dsInfo.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
		dsInfo.m_invViewProjectionMatrix = rqueue.m_viewProjectionMatrix.getInverse();
		dsInfo.m_cameraPosWSpace = rqueue.m_cameraTransform.getTranslationPart().xyz1();
		dsInfo.m_viewport = UVec4(faceIdx * m_tileSize, 0, m_tileSize, m_tileSize);
		dsInfo.m_gbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize * 6), 1.0f / F32(m_tileSize));
		dsInfo.m_gbufferTexCoordsBias = Vec2(0.0f, 0.0f);
		dsInfo.m_lightbufferTexCoordsBias = Vec2(-F32(faceIdx), 0.0f);
		dsInfo.m_lightbufferTexCoordsScale = Vec2(1.0f / F32(m_tileSize), 1.0f / F32(m_tileSize));
		dsInfo.m_cameraNear = rqueue.m_cameraNear;
		dsInfo.m_cameraFar = rqueue.m_cameraFar;
		dsInfo.m_directionalLight = (rqueue.m_directionalLight.isEnabled()) ? &rqueue.m_directionalLight : nullptr;
		dsInfo.m_pointLights = rqueue.m_pointLights;
		dsInfo.m_spotLights = rqueue.m_spotLights;
		dsInfo.m_commandBuffer = cmdb;
		dsInfo.m_gbufferRenderTargets[0] = giCtx.m_gbufferColorRts[0];
		dsInfo.m_gbufferRenderTargets[1] = giCtx.m_gbufferColorRts[1];
		dsInfo.m_gbufferRenderTargets[2] = giCtx.m_gbufferColorRts[2];
		dsInfo.m_gbufferDepthRenderTarget = giCtx.m_gbufferDepthRt;
		if(dsInfo.m_directionalLight && dsInfo.m_directionalLight->hasShadow())
		{
			dsInfo.m_directionalLightShadowmapRenderTarget = giCtx.m_shadowsRt;
		}
		dsInfo.m_renderpassContext = &rgraphCtx;
		dsInfo.m_skybox = &giCtx.m_ctx->m_renderQueue->m_skybox;

		m_lightShading.m_deferred.drawLights(dsInfo);
	}
}

void IndirectDiffuseProbes::runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx)
{
	ANKI_TRACE_SCOPED_EVENT(RIndirectDiffuse);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	ANKI_ASSERT(giCtx.m_probeToUpdateThisFrame);
	const GlobalIlluminationProbeQueueElementForRefresh& probe = *giCtx.m_probeToUpdateThisFrame;

	cmdb->bindShaderProgram(m_irradiance.m_grProg);

	// Bind resources
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindColorTexture(0, 1, giCtx.m_lightShadingRt);

	for(U32 i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
	{
		rgraphCtx.bindColorTexture(0, 2, giCtx.m_gbufferColorRts[i], i);
	}

	rgraphCtx.bindImage(0, 3, giCtx.m_irradianceVolume, TextureSubresourceInfo());

	class
	{
	public:
		IVec3 m_volumeTexel;
		I32 m_nextTexelOffsetInU;
	} unis;

	unis.m_volumeTexel = IVec3(probe.m_cellToRefresh.x(), probe.m_cellToRefresh.y(), probe.m_cellToRefresh.z());
	unis.m_nextTexelOffsetInU = probe.m_cellCounts.x();
	cmdb->setPushConstants(&unis, sizeof(unis));

	// Dispatch
	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
