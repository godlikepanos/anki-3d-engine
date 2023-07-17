// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Shaders/Include/TraditionalDeferredShadingTypes.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>

namespace anki {

static NumericCVar<U32> g_probeReflectionIrradianceResolutionCVar(CVarSubsystem::kRenderer, "ProbeReflectionIrradianceResolution", 16, 4, 2048,
																  "Reflection probe irradiance resolution");
static NumericCVar<U32> g_probeReflectionShadowMapResolutionCVar(CVarSubsystem::kRenderer, "ProbeReflectionShadowMapResolution", 64, 4, 2048,
																 "Reflection probe shadow resolution");

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
		RenderTargetDescription texinit = getRenderer().create2DRenderTargetDescription(m_gbuffer.m_tileSize * 6, m_gbuffer.m_tileSize,
																						kGBufferColorRenderTargetFormats[0], "CubeRefl GBuffer");

		// Create color RT descriptions
		for(U32 i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			texinit.m_format = kGBufferColorRenderTargetFormats[i];
			m_gbuffer.m_colorRtDescrs[i] = texinit;
			m_gbuffer.m_colorRtDescrs[i].setName(RendererString().sprintf("CubeRefl GBuff Col #%u", i));
			m_gbuffer.m_colorRtDescrs[i].bake();
		}

		// Create depth RT
		texinit.m_format = getRenderer().getDepthNoStencilFormat();
		texinit.setName("CubeRefl GBuff Depth");
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

Error ProbeReflections::initLightShading()
{
	m_lightShading.m_tileSize = g_reflectionProbeResolutionCVar.get();
	m_lightShading.m_mipCount = computeMaxMipmapCount2d(m_lightShading.m_tileSize, m_lightShading.m_tileSize, 8);

	for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
	{
		// Light pass FB
		FramebufferDescription& fbDescr = m_lightShading.m_fbDescr[faceIdx];
		ANKI_ASSERT(!fbDescr.isBacked());
		fbDescr.m_colorAttachmentCount = 1;
		fbDescr.m_colorAttachments[0].m_surface.m_face = faceIdx;
		fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
		fbDescr.bake();
	}

	// Init deferred
	ANKI_CHECK(m_lightShading.m_deferred.init());

	return Error::kNone;
}

Error ProbeReflections::initIrradiance()
{
	m_irradiance.m_workgroupSize = g_probeReflectionIrradianceResolutionCVar.get();

	// Create prog
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IrradianceDice.ankiprogbin", m_irradiance.m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_irradiance.m_prog);

		variantInitInfo.addMutation("WORKGROUP_SIZE_XY", U32(m_irradiance.m_workgroupSize));
		variantInitInfo.addMutation("LIGHT_SHADING_TEX", 1);
		variantInitInfo.addMutation("STORE_LOCATION", 1);
		variantInitInfo.addMutation("SECOND_BOUNCE", 0);

		const ShaderProgramResourceVariant* variant;
		m_irradiance.m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_irradiance.m_grProg.reset(&variant->getProgram());
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
		getRenderer().create2DRenderTargetDescription(resolution * 6, resolution, getRenderer().getDepthNoStencilFormat(), "CubeRefl SM");
	m_shadowMapping.m_rtDescr.bake();

	// FB descr
	m_shadowMapping.m_fbDescr.m_colorAttachmentCount = 0;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0f;
	m_shadowMapping.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
	m_shadowMapping.m_fbDescr.bake();

	return Error::kNone;
}

void ProbeReflections::runGBuffer(const Array<GpuVisibilityOutput, 6>& visOuts, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);
	const ReflectionProbeQueueElementForRefresh& probe = *m_ctx.m_probe;
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	const U32 faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

	const U32 viewportX = faceIdx * m_gbuffer.m_tileSize;
	cmdb.setViewport(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);
	cmdb.setScissor(viewportX, 0, m_gbuffer.m_tileSize, m_gbuffer.m_tileSize);

	const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];

	RenderableDrawerArguments args;
	args.m_viewMatrix = rqueue.m_viewMatrix;
	args.m_cameraTransform = rqueue.m_cameraTransform;
	args.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
	args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care about prev mats
	args.m_sampler = getRenderer().getSamplers().m_trilinearRepeat.get();
	args.m_renderingTechinuqe = RenderingTechnique::kGBuffer;
	args.fillMdi(visOuts[faceIdx]);

	getRenderer().getSceneDrawer().drawMdi(args, cmdb);
}

void ProbeReflections::runLightShading(U32 faceIdx, const RenderingContext& rctx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx <= 6);
	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	ANKI_ASSERT(m_ctx.m_probe);
	const ReflectionProbeQueueElementForRefresh& probe = *m_ctx.m_probe;
	const RenderQueue& rqueue = *probe.m_renderQueues[faceIdx];
	const Bool hasDirLight = probe.m_renderQueues[0]->m_directionalLight.m_uuid;

	TraditionalDeferredLightShadingDrawInfo dsInfo;
	dsInfo.m_viewProjectionMatrix = rqueue.m_viewProjectionMatrix;
	dsInfo.m_invViewProjectionMatrix = rqueue.m_viewProjectionMatrix.getInverse();
	dsInfo.m_cameraPosWSpace = rqueue.m_cameraTransform.getTranslationPart().xyz1();
	dsInfo.m_viewport = UVec4(0, 0, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
	dsInfo.m_gbufferTexCoordsScale = Vec2(1.0f / F32(m_lightShading.m_tileSize * 6), 1.0f / F32(m_lightShading.m_tileSize));
	dsInfo.m_gbufferTexCoordsBias = Vec2(F32(faceIdx) * (1.0f / 6.0f), 0.0f);
	dsInfo.m_lightbufferTexCoordsScale = Vec2(1.0f / F32(m_lightShading.m_tileSize), 1.0f / F32(m_lightShading.m_tileSize));
	dsInfo.m_lightbufferTexCoordsBias = Vec2(0.0f, 0.0f);
	dsInfo.m_cameraNear = probe.m_renderQueues[faceIdx]->m_cameraNear;
	dsInfo.m_cameraFar = probe.m_renderQueues[faceIdx]->m_cameraFar;
	dsInfo.m_directionalLight = (hasDirLight) ? &probe.m_renderQueues[faceIdx]->m_directionalLight : nullptr;
	dsInfo.m_pointLights = rqueue.m_pointLights;
	dsInfo.m_spotLights = rqueue.m_spotLights;
	dsInfo.m_commandBuffer = &cmdb;
	dsInfo.m_gbufferRenderTargets[0] = m_ctx.m_gbufferColorRts[0];
	dsInfo.m_gbufferRenderTargets[1] = m_ctx.m_gbufferColorRts[1];
	dsInfo.m_gbufferRenderTargets[2] = m_ctx.m_gbufferColorRts[2];
	dsInfo.m_gbufferDepthRenderTarget = m_ctx.m_gbufferDepthRt;
	if(hasDirLight && dsInfo.m_directionalLight->hasShadow())
	{
		dsInfo.m_directionalLightShadowmapRenderTarget = m_ctx.m_shadowMapRt;
	}
	dsInfo.m_renderpassContext = &rgraphCtx;
	dsInfo.m_skybox = &rctx.m_renderQueue->m_skybox;

	m_lightShading.m_deferred.drawLights(dsInfo);
}

void ProbeReflections::runMipmappingOfLightShading(U32 faceIdx, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(faceIdx < 6);

	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);

	TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, 0));
	subresource.m_mipmapCount = m_lightShading.m_mipCount;

	Texture* texToBind;
	rgraphCtx.getRenderTargetState(m_ctx.m_lightShadingRt, subresource, texToBind);

	TextureViewInitInfo viewInit(texToBind, subresource);
	TextureViewPtr view = GrManager::getSingleton().newTextureView(viewInit);
	rgraphCtx.m_commandBuffer->generateMipmaps2d(view.get());
}

void ProbeReflections::runIrradiance(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram(m_irradiance.m_grProg.get());

	// Bind stuff
	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

	TextureSubresourceInfo subresource;
	subresource.m_faceCount = 6;
	rgraphCtx.bindTexture(0, 1, m_ctx.m_lightShadingRt, subresource);

	cmdb.bindStorageBuffer(0, 3, m_irradiance.m_diceValuesBuff.get(), 0, m_irradiance.m_diceValuesBuff->getSize());

	// Draw
	cmdb.dispatchCompute(1, 1, 1);
}

void ProbeReflections::runIrradianceToRefl(RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram(m_irradianceToRefl.m_grProg.get());

	// Bind resources
	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

	rgraphCtx.bindColorTexture(0, 1, m_ctx.m_gbufferColorRts[0], 0);
	rgraphCtx.bindColorTexture(0, 1, m_ctx.m_gbufferColorRts[1], 1);
	rgraphCtx.bindColorTexture(0, 1, m_ctx.m_gbufferColorRts[2], 2);

	cmdb.bindStorageBuffer(0, 2, m_irradiance.m_diceValuesBuff.get(), 0, m_irradiance.m_diceValuesBuff->getSize());

	for(U8 f = 0; f < 6; ++f)
	{
		TextureSubresourceInfo subresource;
		subresource.m_faceCount = 1;
		subresource.m_firstFace = f;
		rgraphCtx.bindImage(0, 3, m_ctx.m_lightShadingRt, subresource, f);
	}

	dispatchPPCompute(cmdb, 8, 8, m_lightShading.m_tileSize, m_lightShading.m_tileSize);
}

void ProbeReflections::populateRenderGraph(RenderingContext& rctx)
{
	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);

	if(rctx.m_renderQueue->m_reflectionProbeForRefresh == nullptr) [[likely]]
	{
		// Early exit
		m_ctx.m_lightShadingRt = {};
		return;
	}

#if ANKI_EXTRA_CHECKS
	m_ctx = {};
#endif

	m_ctx.m_probe = rctx.m_renderQueue->m_reflectionProbeForRefresh;

	RenderGraphDescription& rgraph = rctx.m_renderGraphDescr;

	// GBuffer visibility
	Array<GpuVisibilityOutput, 6> visOuts;
	for(U32 i = 0; i < 6; ++i)
	{
		const RenderQueue& queue = *m_ctx.m_probe->m_renderQueues[i];
		Array<F32, kMaxLodCount - 1> lodDistances = {1000.0f, 1001.0f}; // Something far to force detailed LODs

		GpuVisibilityInput visIn;
		visIn.m_passesName = "Cube refl GBuffer visibility";
		visIn.m_technique = RenderingTechnique::kGBuffer;
		visIn.m_viewProjectionMatrix = queue.m_viewProjectionMatrix;
		visIn.m_lodReferencePoint = queue.m_cameraTransform.getTranslationPart().xyz();
		visIn.m_lodDistances = lodDistances;
		visIn.m_rgraph = &rgraph;

		getRenderer().getGpuVisibility().populateRenderGraph(visIn, visOuts[i]);
	}

	// GBuffer pass
	{
		// RTs
		Array<RenderTargetHandle, kMaxColorRenderTargets> rts;
		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			m_ctx.m_gbufferColorRts[i] = rgraph.newRenderTarget(m_gbuffer.m_colorRtDescrs[i]);
			rts[i] = m_ctx.m_gbufferColorRts[i];
		}
		m_ctx.m_gbufferDepthRt = rgraph.newRenderTarget(m_gbuffer.m_depthRtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Cube refl GBuffer");
		pass.setFramebufferInfo(m_gbuffer.m_fbDescr, rts, m_ctx.m_gbufferDepthRt);
		pass.setWork(6, [this, visOuts](RenderPassWorkContext& rgraphCtx) {
			runGBuffer(visOuts, rgraphCtx);
		});

		for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
		{
			pass.newTextureDependency(m_ctx.m_gbufferColorRts[i], TextureUsageBit::kFramebufferWrite);
		}

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(m_ctx.m_gbufferDepthRt, TextureUsageBit::kAllFramebuffer, subresource);

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(),
								 BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead);

		for(U32 i = 0; i < 6; ++i)
		{
			pass.newBufferDependency(visOuts[i].m_mdiDrawCountsHandle, BufferUsageBit::kIndirectDraw);
		}
	}

	// Shadow visibility. Optional
	const Bool doShadows =
		m_ctx.m_probe->m_renderQueues[0]->m_directionalLight.m_uuid && m_ctx.m_probe->m_renderQueues[0]->m_directionalLight.m_shadowCascadeCount > 0;
	Array<GpuVisibilityOutput, 6> shadowVisOuts;
	if(doShadows)
	{
		for(U i = 0; i < 6; ++i)
		{
			const RenderQueue& queue = *m_ctx.m_probe->m_renderQueues[i]->m_directionalLight.m_shadowRenderQueues[0];
			Array<F32, kMaxLodCount - 1> lodDistances = {1000.0f, 1001.0f}; // Something far to force detailed LODs

			GpuVisibilityInput visIn;
			visIn.m_passesName = "Cube refl shadows visibility";
			visIn.m_technique = RenderingTechnique::kDepth;
			visIn.m_viewProjectionMatrix = queue.m_viewProjectionMatrix;
			visIn.m_lodReferencePoint = queue.m_cameraTransform.getTranslationPart().xyz();
			visIn.m_lodDistances = lodDistances;
			visIn.m_rgraph = &rgraph;

			getRenderer().getGpuVisibility().populateRenderGraph(visIn, shadowVisOuts[i]);
		}
	}

	// Shadows. Optional
	if(doShadows)
	{
		// Update light matrices
		for(U i = 0; i < 6; ++i)
		{
			ANKI_ASSERT(m_ctx.m_probe->m_renderQueues[i]->m_directionalLight.m_uuid
						&& m_ctx.m_probe->m_renderQueues[i]->m_directionalLight.m_shadowCascadeCount == 1);

			const F32 xScale = 1.0f / 6.0f;
			const F32 yScale = 1.0f;
			const F32 xOffset = F32(i) * (1.0f / 6.0f);
			const F32 yOffset = 0.0f;
			const Mat4 atlasMtx(xScale, 0.0f, 0.0f, xOffset, 0.0f, yScale, 0.0f, yOffset, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

			Mat4& lightMat = m_ctx.m_probe->m_renderQueues[i]->m_directionalLight.m_textureMatrices[0];
			lightMat = atlasMtx * lightMat;
		}

		// RT
		m_ctx.m_shadowMapRt = rgraph.newRenderTarget(m_shadowMapping.m_rtDescr);

		// Pass
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Cube refl shadows");
		pass.setFramebufferInfo(m_shadowMapping.m_fbDescr, {}, m_ctx.m_shadowMapRt);
		pass.setWork(6, [this, shadowVisOuts](RenderPassWorkContext& rgraphCtx) {
			runShadowMapping(shadowVisOuts, rgraphCtx);
		});

		TextureSubresourceInfo subresource(DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(m_ctx.m_shadowMapRt, TextureUsageBit::kAllFramebuffer, subresource);

		pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(),
								 BufferUsageBit::kStorageGeometryRead | BufferUsageBit::kStorageFragmentRead);

		for(U32 i = 0; i < 6; ++i)
		{
			pass.newBufferDependency(shadowVisOuts[i].m_mdiDrawCountsHandle, BufferUsageBit::kIndirectDraw);
		}
	}
	else
	{
		m_ctx.m_shadowMapRt = {};
	}

	// Light shading passes
	{
		// RT
		m_ctx.m_lightShadingRt = rgraph.importRenderTarget(m_ctx.m_probe->m_reflectionTexture, TextureUsageBit::kNone);

		// Passes
		static constexpr Array<CString, 6> passNames = {"Cube refl light shading #0", "Cube refl light shading #1", "Cube refl light shading #2",
														"Cube refl light shading #3", "Cube refl light shading #4", "Cube refl light shading #5"};
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
			pass.setFramebufferInfo(m_lightShading.m_fbDescr[faceIdx], {m_ctx.m_lightShadingRt});
			pass.setWork([this, faceIdx, &rctx](RenderPassWorkContext& rgraphCtx) {
				runLightShading(faceIdx, rctx, rgraphCtx);
			});

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, 0));
			pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kFramebufferWrite, subresource);

			for(U i = 0; i < kGBufferColorRenderTargetCount; ++i)
			{
				pass.newTextureDependency(m_ctx.m_gbufferColorRts[i], TextureUsageBit::kSampledFragment);
			}
			pass.newTextureDependency(m_ctx.m_gbufferDepthRt, TextureUsageBit::kSampledFragment,
									  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			if(m_ctx.m_shadowMapRt.isValid())
			{
				pass.newTextureDependency(m_ctx.m_shadowMapRt, TextureUsageBit::kSampledFragment);
			}
		}
	}

	// Irradiance passes
	{
		m_ctx.m_irradianceDiceValuesBuffHandle = rgraph.importBuffer(m_irradiance.m_diceValuesBuff.get(), BufferUsageBit::kNone);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cube refl irradiance");

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			runIrradiance(rgraphCtx);
		});

		// Read a cube but only one layer and level
		TextureSubresourceInfo readSubresource;
		readSubresource.m_faceCount = 6;
		pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kSampledCompute, readSubresource);

		pass.newBufferDependency(m_ctx.m_irradianceDiceValuesBuffHandle, BufferUsageBit::kStorageComputeWrite);
	}

	// Write irradiance back to refl
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Cube refl apply indirect");

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			runIrradianceToRefl(rgraphCtx);
		});

		for(U i = 0; i < kGBufferColorRenderTargetCount - 1; ++i)
		{
			pass.newTextureDependency(m_ctx.m_gbufferColorRts[i], TextureUsageBit::kSampledCompute);
		}

		TextureSubresourceInfo subresource;
		subresource.m_faceCount = 6;
		pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kImageComputeRead | TextureUsageBit::kImageComputeWrite, subresource);

		pass.newBufferDependency(m_ctx.m_irradianceDiceValuesBuffHandle, BufferUsageBit::kStorageComputeRead);
	}

	// Mipmapping "passes"
	{
		static constexpr Array<CString, 6> passNames = {"Cube refl gen mips #0", "Cube refl gen mips #1", "Cube refl gen mips #2",
														"Cube refl gen mips #3", "Cube refl gen mips #4", "Cube refl gen mips #5"};
		for(U32 faceIdx = 0; faceIdx < 6; ++faceIdx)
		{
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[faceIdx]);
			pass.setWork([this, faceIdx](RenderPassWorkContext& rgraphCtx) {
				runMipmappingOfLightShading(faceIdx, rgraphCtx);
			});

			TextureSubresourceInfo subresource(TextureSurfaceInfo(0, 0, faceIdx, 0));
			subresource.m_mipmapCount = m_lightShading.m_mipCount;

			pass.newTextureDependency(m_ctx.m_lightShadingRt, TextureUsageBit::kGenerateMipmaps, subresource);
		}
	}
}

void ProbeReflections::runShadowMapping(const Array<GpuVisibilityOutput, 6>& visOuts, RenderPassWorkContext& rgraphCtx)
{
	ANKI_ASSERT(m_ctx.m_probe);
	ANKI_TRACE_SCOPED_EVENT(RCubeRefl);

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
	cmdb.setPolygonOffset(kShadowsPolygonOffsetFactor, kShadowsPolygonOffsetUnits);

	const U32 faceIdx = rgraphCtx.m_currentSecondLevelCommandBufferIndex;

	ANKI_ASSERT(m_ctx.m_probe->m_renderQueues[faceIdx]);
	const RenderQueue& faceRenderQueue = *m_ctx.m_probe->m_renderQueues[faceIdx];
	ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_uuid != 0);
	ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowCascadeCount == 1);
	ANKI_ASSERT(faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0]);
	const RenderQueue& cascadeRenderQueue = *faceRenderQueue.m_directionalLight.m_shadowRenderQueues[0];

	const U32 rez = m_shadowMapping.m_rtDescr.m_height;
	cmdb.setViewport(rez * faceIdx, 0, rez, rez);
	cmdb.setScissor(rez * faceIdx, 0, rez, rez);

	RenderableDrawerArguments args;
	args.m_viewMatrix = cascadeRenderQueue.m_viewMatrix;
	args.m_cameraTransform = Mat3x4::getIdentity(); // Don't care
	args.m_viewProjectionMatrix = cascadeRenderQueue.m_viewProjectionMatrix;
	args.m_previousViewProjectionMatrix = Mat4::getIdentity(); // Don't care
	args.m_sampler = getRenderer().getSamplers().m_trilinearRepeatAniso.get();
	args.m_renderingTechinuqe = RenderingTechnique::kDepth;
	args.fillMdi(visOuts[faceIdx]);

	getRenderer().getSceneDrawer().drawMdi(args, cmdb);
}

} // end namespace anki
