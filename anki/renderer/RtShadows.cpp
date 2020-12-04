// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/RtShadows.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/AccelerationStructureBuilder.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/resource/ShaderProgramResourceSystem.h>
#include <anki/util/Tracer.h>

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
	// Get the program
	const ShaderProgramResourceSystem& shaderSystem = getResourceManager().getShaderProgramResourceSystem();
	const ShaderProgramRaytracingLibrary* library = nullptr;
	for(const ShaderProgramRaytracingLibrary& lib : shaderSystem.getRayTracingLibraries())
	{
		if(lib.getLibraryName() == "RtShadows")
		{
			library = &lib;
			break;
		}
	}
	ANKI_ASSERT(library);
	m_grProg = library->getShaderProgram();

	// Denoise program
	ANKI_CHECK(getResourceManager().loadResource("shaders/RtShadowsDenoise.ankiprog", m_denoiseProg));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_denoiseProg);
	variantInitInfo.addConstant("OUT_IMAGE_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));
	variantInitInfo.addConstant("SAMPLE_COUNT", 8u);
	variantInitInfo.addConstant("SPIRAL_TURN_COUNT", 27u);
	variantInitInfo.addConstant("PIXEL_RADIUS", 12u);

	const ShaderProgramResourceVariant* variant;
	m_denoiseProg->getOrCreateVariant(variantInitInfo, variant);
	m_grDenoiseProg = variant->getProgram();

	// RTs
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
		m_r->getWidth(), m_r->getHeight(), Format::R8G8B8A8_UNORM,
		TextureUsageBit::ALL_SAMPLED | TextureUsageBit::IMAGE_TRACE_RAYS_WRITE | TextureUsageBit::IMAGE_COMPUTE_WRITE,
		"RtShadows");
	texinit.m_type = TextureType::_2D_ARRAY;
	texinit.m_layerCount = 2;
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_historyAndFinalRt = m_r->createAndClearRenderTarget(texinit);

	m_renderRt = m_r->create2DRenderTargetDescription(m_r->getWidth() / 2, m_r->getHeight() / 2, Format::R8G8B8A8_UNORM,
													  "RtShadowsTmp");
	m_renderRt.m_type = TextureType::_2D_ARRAY;
	m_renderRt.m_layerCount = 2;
	m_renderRt.bake();

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

	// RTs
	if(ANKI_UNLIKELY(!m_runCtx.m_historyAndFinalRtImportedOnce))
	{
		m_runCtx.m_historyAndFinalRt =
			rgraph.importRenderTarget(m_historyAndFinalRt, TextureUsageBit::SAMPLED_FRAGMENT);
		m_runCtx.m_historyAndFinalRtImportedOnce = true;
	}
	else
	{
		m_runCtx.m_historyAndFinalRt = rgraph.importRenderTarget(m_historyAndFinalRt);
	}

	m_runCtx.m_renderRt = rgraph.newRenderTarget(m_renderRt);

	// RT shadows pass
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<RtShadows*>(rgraphCtx.m_userData)->run(rgraphCtx); },
			this, 0);

		rpass.newDependency(RenderPassDependency(m_runCtx.m_historyAndFinalRt, TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_renderRt, TextureUsageBit::IMAGE_TRACE_RAYS_WRITE));
		rpass.newDependency(
			RenderPassDependency(m_r->getAccelerationStructureBuilder().getAccelerationStructureHandle(),
								 AccelerationStructureUsageBit::TRACE_RAYS_READ));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(
			RenderPassDependency(m_r->getGBuffer().getPreviousFrameDepthRt(), TextureUsageBit::SAMPLED_TRACE_RAYS));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_TRACE_RAYS));
	}

	// Denoise pass
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadowsDenoise");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<RtShadows*>(rgraphCtx.m_userData)->runDenoise(rgraphCtx);
			},
			this, 0);

		rpass.newDependency(RenderPassDependency(m_runCtx.m_historyAndFinalRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_renderRt, TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE));
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
			ANKI_ASSERT(rqueue.m_directionalLight.m_shadowLayer < MAX_SHADOW_LAYERS);
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
				ANKI_ASSERT(light.m_shadowLayer < MAX_SHADOW_LAYERS);
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
				ANKI_ASSERT(light.m_shadowLayer < MAX_SHADOW_LAYERS);
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

	cmdb->bindShaderProgram(m_grProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearRepeat);

	TextureSubresourceInfo subresource;
	rgraphCtx.bindImage(0, 1, m_runCtx.m_renderRt, subresource, 0);
	subresource.m_firstLayer = 1;
	rgraphCtx.bindImage(0, 1, m_runCtx.m_renderRt, subresource, 1);

	rgraphCtx.bindColorTexture(0, 2, m_runCtx.m_historyAndFinalRt);
	cmdb->bindSampler(0, 3, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 4, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindTexture(0, 5, m_r->getGBuffer().getPreviousFrameDepthRt(),
						  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 6, m_r->getGBuffer().getColorRt(2));
	rgraphCtx.bindAccelerationStructure(0, 7, m_r->getAccelerationStructureBuilder().getAccelerationStructureHandle());

	bindUniforms(cmdb, 0, 8, ctx.m_lightShadingUniformsToken);

	bindUniforms(cmdb, 0, 9, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 10, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 11, m_r->getShadowMapping().getShadowmapRt());

	bindStorage(cmdb, 0, 12, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 13, rsrc.m_indicesToken);

	cmdb->bindAllBindless(1);

	Array<F32, MAX_SHADOW_LAYERS> rejectFactors;
	for(U32 i = 0; i < MAX_SHADOW_LAYERS; ++i)
	{
		rejectFactors[i] = F32(m_runCtx.m_layersWithRejectedHistory.get(i));
	}
	cmdb->setPushConstants(&rejectFactors, sizeof(rejectFactors));

	cmdb->traceRays(m_runCtx.m_sbtBuffer, m_runCtx.m_sbtOffset, m_sbtRecordSize, m_runCtx.m_hitGroupCount, 1,
					m_r->getWidth() / 2, m_r->getHeight() / 2, 1);
}

void RtShadows::runDenoise(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grDenoiseProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_renderRt);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	TextureSubresourceInfo subresource;
	rgraphCtx.bindImage(0, 4, m_runCtx.m_historyAndFinalRt, subresource, 0);
	subresource.m_firstLayer = 1;
	rgraphCtx.bindImage(0, 4, m_runCtx.m_historyAndFinalRt, subresource, 1);

	class PC
	{
	public:
		Mat4 m_invViewProjMat;
		Vec3 m_padding;
		F32 m_time;
	} pc;

	pc.m_invViewProjMat = m_runCtx.m_ctx->m_matrices.m_viewProjectionJitter.getInverse();
	pc.m_time = F32(m_r->getGlobalTimestamp());
	cmdb->setPushConstants(&pc, sizeof(pc));

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth(), m_r->getHeight());
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
	U8* sbt = allocateStorage<U8*>(m_sbtRecordSize * (instanceCount + extraSbtRecords), token);
	const U8* sbtStart = sbt;
	(void)sbtStart;
	m_runCtx.m_sbtBuffer = token.m_buffer;
	m_runCtx.m_sbtOffset = token.m_offset;

	// Set the miss and ray gen handles
	ConstWeakArray<U8> shaderGroupHandles = m_grProg->getShaderGroupHandles();
	memcpy(sbt, &shaderGroupHandles[0], shaderHandleSize);
	sbt += m_sbtRecordSize;
	memcpy(sbt, &shaderGroupHandles[shaderHandleSize], shaderHandleSize);
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

	for(U32 i = 0; i < m_runCtx.m_shadowLayers.getSize(); ++i)
	{
		ShadowLayer& layer = m_runCtx.m_shadowLayers[i];
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
		m_runCtx.m_shadowLayers[nextBestLayerIdx].m_frameLastUsed = crntFrame;
		m_runCtx.m_shadowLayers[nextBestLayerIdx].m_lightUuid = lightUuid;
		rejectHistoryBuffer = true;
	}

	return layerIdx != MAX_U32;
}

} // end namespace anki
