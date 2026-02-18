// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Reflections.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Sky.h>
#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/ProbeReflections.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/ImageAtlasResource.h>

namespace anki {

Error Reflections::init()
{
	ANKI_CHECK(RtMaterialFetchRendererObject::init());

	const Bool bRtReflections = GrManager::getSingleton().getDeviceCapabilities().m_rayTracing && g_cvarRenderReflectionsRt;
	const Bool bSsrSamplesGBuffer = bRtReflections;

	std::initializer_list<SubMutation> mutation = {{"SSR_SAMPLE_GBUFFER", bSsrSamplesGBuffer},
												   {"INDIRECT_DIFFUSE_CLIPMAPS", isIndirectDiffuseClipmapsEnabled()}};
	constexpr CString kProgFname = "ShaderBinaries/Reflections.ankiprogbin";
	// Ray gen and miss
	if(bRtReflections)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(kProgFname, m_mainProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_mainProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		variantInitInfo.addMutation("SSR_SAMPLE_GBUFFER", bSsrSamplesGBuffer);
		variantInitInfo.addMutation("INDIRECT_DIFFUSE_CLIPMAPS", isIndirectDiffuseClipmapsEnabled());
		const ShaderProgramResourceVariant* variant;
		m_mainProg->getOrCreateVariant(variantInitInfo, variant);
		m_libraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

		m_shaderGroupHandlesBuff = variant->getShaderGroupHandlesBuffer();

		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", m_missProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo2(m_missProg);
		variantInitInfo2.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtMaterialFetch");
		m_missProg->getOrCreateVariant(variantInitInfo2, variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();

		m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
											GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

		ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_rtMaterialFetchInlineRtGrProg, "RtMaterialFetchInlineRt"));
	}

	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_spatialDenoisingGrProg, "SpatialDenoise"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_temporalDenoisingGrProg, "TemporalDenoise"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_verticalBilateralDenoisingGrProg, "BilateralDenoiseVertical"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_horizontalBilateralDenoisingGrProg, "BilateralDenoiseHorizontal"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_ssrGrProg, "Ssr"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_probeFallbackGrProg, "ReflectionProbeFallback"));
	ANKI_CHECK(loadShaderProgram(kProgFname, mutation, m_mainProg, m_tileClassificationGrProg, "Classification"));

	m_transientRtDesc1 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR16G16B16A16_Sfloat, "Reflections #1");
	m_transientRtDesc1.bake();

	m_transientRtDesc2 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR16G16B16A16_Sfloat, "Reflections #2");
	m_transientRtDesc2.bake();

	m_hitPosAndDepthRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x / 2u, getRenderer().getInternalResolution().y, Format::kR16G16B16A16_Sfloat, "HitPosAndDepth");
	m_hitPosAndDepthRtDesc.bake();

	m_hitPosRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
																   Format::kR16G16B16A16_Sfloat, "HitPos");
	m_hitPosRtDesc.bake();

	TextureInitInfo texInit = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, getRenderer().getHdrFormat(), "ReflectionsMain");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_tex = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	texInit = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
															Format::kR32G32_Sfloat, "ReflectionsMoments #1");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_momentsTextures[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);
	texInit.setName("ReflectionsMoments #2");
	m_momentsTextures[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	m_classTileMapRtDesc = getRenderer().create2DRenderTargetDescription((getRenderer().getInternalResolution().x + kTileSize - 1) / kTileSize,
																		 (getRenderer().getInternalResolution().y + kTileSize - 1) / kTileSize,
																		 Format::kR8_Uint, "ReflClassTileMap");
	m_classTileMapRtDesc.bake();

	{
		m_indirectArgsBuffer = TextureMemoryPool::getSingleton().allocateStructuredBuffer<DispatchIndirectArgs>(2);

		Array<DispatchIndirectArgs, 2> args;
		args[0].m_threadGroupCountX = 0;
		args[0].m_threadGroupCountY = args[0].m_threadGroupCountZ = 1;
		args[1].m_threadGroupCountX = 0;
		args[1].m_threadGroupCountY = args[1].m_threadGroupCountZ = 1;

		fillBuffer(ConstWeakArray<U8>(reinterpret_cast<const U8*>(args.getBegin()), args.getSizeInBytes()), m_indirectArgsBuffer);
	}

	return Error::kNone;
}

void Reflections::populateRenderGraph()
{
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const Bool bRtReflections = GrManager::getSingleton().getDeviceCapabilities().m_rayTracing && g_cvarRenderReflectionsRt;

	// Create or import render targets
	RenderTargetHandle mainRt;
	RenderTargetHandle readMomentsRt;
	RenderTargetHandle writeMomentsRt;

	if(m_texImportedOnce)
	{
		mainRt = rgraph.importRenderTarget(m_tex.get());
		readMomentsRt = rgraph.importRenderTarget(m_momentsTextures[getRenderer().getFrameCount() & 1].get());
		writeMomentsRt = rgraph.importRenderTarget(m_momentsTextures[(getRenderer().getFrameCount() + 1) & 1].get());
	}
	else
	{
		mainRt = rgraph.importRenderTarget(m_tex.get(), TextureUsageBit::kAllSrv);
		readMomentsRt = rgraph.importRenderTarget(m_momentsTextures[getRenderer().getFrameCount() & 1].get(), TextureUsageBit::kAllSrv);
		writeMomentsRt = rgraph.importRenderTarget(m_momentsTextures[(getRenderer().getFrameCount() + 1) & 1].get(), TextureUsageBit::kAllSrv);
		m_texImportedOnce = true;
	}

	const RenderTargetHandle transientRt1 = rgraph.newRenderTarget(m_transientRtDesc1);
	const RenderTargetHandle transientRt2 = rgraph.newRenderTarget(m_transientRtDesc2);
	const RenderTargetHandle hitPosAndDepthRt = rgraph.newRenderTarget(m_hitPosAndDepthRtDesc);
	const RenderTargetHandle hitPosRt = rgraph.newRenderTarget(m_hitPosRtDesc);
	const RenderTargetHandle classTileMapRt = rgraph.newRenderTarget(m_classTileMapRtDesc);
	const BufferHandle indirectArgsHandle = rgraph.importBuffer(m_indirectArgsBuffer, BufferUsageBit::kNone);

	ReflectionConstants consts;
	consts.m_ssrStepIncrement = g_cvarRenderReflectionsSsrStepIncrement;
	consts.m_ssrMaxIterations = g_cvarRenderReflectionsSsrMaxIterations;
	consts.m_roughnessCutoffToGiEdges = Vec2(g_cvarRenderReflectionsRoughnessCutoffToGiEdge0, g_cvarRenderReflectionsRoughnessCutoffToGiEdge1);

	// Classification
	{
		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflTileClassification");

		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, classTileMapRt, consts](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsClassification);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tileClassificationGrProg.get());

			rgraphCtx.bindSrv(0, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindUav(0, 0, classTileMapRt);
			cmdb.bindUav(1, 0, m_indirectArgsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, kTileSize / 2, kTileSize, getRenderer().getInternalResolution().x / 2, getRenderer().getInternalResolution().y);
		});
	}

	// SSR
	BufferView pixelsFailedSsrBuff;
	{
		const U32 pixelCount = getRenderer().getInternalResolution().x / 2 * getRenderer().getInternalResolution().y;
		pixelsFailedSsrBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<PixelFailedSsr>(pixelCount);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SSR");

		rpass.newTextureDependency(getDepthDownscale().getRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(0), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getBloom().getPyramidRt(), TextureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);
		rpass.newTextureDependency(getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, transientRt1, hitPosAndDepthRt, pixelsFailedSsrBuff, consts, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsSsr);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_ssrGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());
			cmdb.bindSampler(2, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindSrv(0, 0, getGBuffer().getColorRt(0));
			rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(3, 0, getDepthDownscale().getRt());
			rgraphCtx.bindSrv(4, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(5, 0, getRenderer().getBloom().getPyramidRt());
			cmdb.bindSrv(6, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
			cmdb.bindSrv(7, 0, getClusterBinning().getClustersBuffer());
			rgraphCtx.bindSrv(8, 0, getShadowMapping().getShadowmapRt());
			rgraphCtx.bindSrv(9, 0, classTileMapRt);
			cmdb.bindSrv(10, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));

			rgraphCtx.bindUav(0, 0, transientRt1);
			rgraphCtx.bindUav(1, 0, hitPosAndDepthRt);
			cmdb.bindUav(2, 0, pixelsFailedSsrBuff);
			cmdb.bindUav(3, 0, m_indirectArgsBuffer);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x / 2, getRenderer().getInternalResolution().y);
		});
	}

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	if(bRtReflections && !g_cvarRenderReflectionsInlineRt)
	{
		buildShaderBindingTablePass("RtReflections: Build SBT", m_shaderGroupHandlesBuff, m_rayGenShaderGroupIdx, m_missShaderGroupIdx,
									m_sbtRecordSize, rgraph, sbtHandle, sbtBuffer);
	}

	// Ray gen
	if(bRtReflections)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflections");

		const TextureUsageBit uavTexUsage = (g_cvarRenderReflectionsInlineRt) ? TextureUsageBit::kUavCompute : TextureUsageBit::kUavDispatchRays;
		const BufferUsageBit indirectBuffUsage =
			(g_cvarRenderReflectionsInlineRt) ? BufferUsageBit::kIndirectCompute : BufferUsageBit::kIndirectDispatchRays;
		const TextureUsageBit srvTexUsage = (g_cvarRenderReflectionsInlineRt) ? TextureUsageBit::kSrvCompute : TextureUsageBit::kSrvDispatchRays;

		if(!g_cvarRenderReflectionsInlineRt)
		{
			rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		}
		rpass.newTextureDependency(transientRt1, uavTexUsage);
		rpass.newTextureDependency(hitPosAndDepthRt, uavTexUsage);
		rpass.newBufferDependency(indirectArgsHandle, indirectBuffUsage);
		setRgenSpace2Dependencies(rpass, g_cvarRenderReflectionsInlineRt);

		if(isIndirectDiffuseClipmapsEnabled())
		{
			getIndirectDiffuseClipmaps().setDependencies(rpass, srvTexUsage);
		}

		rpass.setWork([this, sbtBuffer, transientRt1, hitPosAndDepthRt, pixelsFailedSsrBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsRayGen);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram((g_cvarRenderReflectionsInlineRt) ? m_rtMaterialFetchInlineRtGrProg.get() : m_libraryGrProg.get());

			// Space 0 globals
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

			bindRgenSpace2Resources(rgraphCtx);
			cmdb.bindSrv(7, 2, pixelsFailedSsrBuff);
			rgraphCtx.bindUav(0, 2, transientRt1);
			rgraphCtx.bindUav(1, 2, hitPosAndDepthRt);

			struct Consts
			{
				F32 m_maxRayT;
				U32 m_giProbeCount;
				F32 m_padding1;
				F32 m_padding2;

				Vec4 m_padding[2];
			} consts = {g_cvarRenderReflectionsRtMaxRayDistance, GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getElementCount(), 0, 0, {}};

			cmdb.setFastConstants(&consts, sizeof(consts));

			if(g_cvarRenderReflectionsInlineRt)
			{
				cmdb.dispatchComputeIndirect(BufferView(m_indirectArgsBuffer).incrementOffset(sizeof(DispatchIndirectArgs)));
			}
			else
			{
				cmdb.dispatchRaysIndirect(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
										  BufferView(m_indirectArgsBuffer).setRange(sizeof(DispatchIndirectArgs)));
			}
		});
	}
	else
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionProbeFallback");

		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavCompute);
		if(getGeneratedSky().isEnabled())
		{
			rpass.newTextureDependency(getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvCompute);
		}

		if(getProbeReflections().getHasCurrentlyRefreshedReflectionRt())
		{
			rpass.newTextureDependency(getProbeReflections().getCurrentlyRefreshedReflectionRt(), TextureUsageBit::kSrvCompute);
		}

		rpass.setWork([this, pixelsFailedSsrBuff, transientRt1, hitPosAndDepthRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsProbeFallback);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_probeFallbackGrProg.get());

			rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
			cmdb.bindSrv(1, 0, pixelsFailedSsrBuff);
			cmdb.bindSrv(2, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kReflectionProbe));
			cmdb.bindSrv(3, 0, getClusterBinning().getClustersBuffer());
			cmdb.bindSrv(4, 0, BufferView(m_indirectArgsBuffer).setRange(sizeof(U32)));

			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			if(getGeneratedSky().isEnabled())
			{
				rgraphCtx.bindSrv(5, 0, getGeneratedSky().getEnvironmentMapRt());
			}
			else if(sky && sky->getSkyboxComponentType() == SkyboxComponentType::kImage2D)
			{
				cmdb.bindSrv(5, 0, TextureView(&sky->getSkyTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				cmdb.bindSrv(5, 0, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
			}

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindUav(0, 0, transientRt1);
			rgraphCtx.bindUav(1, 0, hitPosAndDepthRt);

			cmdb.dispatchComputeIndirect(BufferView(m_indirectArgsBuffer).incrementOffset(sizeof(DispatchIndirectArgs)));
		});
	}

	// Spatial denoising
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsSpatialDenoise");

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, transientRt1, transientRt2, hitPosAndDepthRt, hitPosRt, consts, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsSpatialDenoise);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt1);
			rgraphCtx.bindSrv(1, 0, hitPosAndDepthRt);
			rgraphCtx.bindSrv(2, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(3, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(4, 0, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(5, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, transientRt2);
			rgraphCtx.bindUav(1, 0, hitPosRt);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		});
	}

	// Temporal denoising
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsTemporalDenoise");

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(mainRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(readMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kUavCompute);

		rpass.setWork(
			[this, transientRt1, transientRt2, mainRt, readMomentsRt, writeMomentsRt, hitPosRt, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ReflectionsTemporalDenoise);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_temporalDenoisingGrProg.get());

				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

				rgraphCtx.bindSrv(0, 0, transientRt2);
				rgraphCtx.bindSrv(1, 0, mainRt);
				rgraphCtx.bindSrv(2, 0, readMomentsRt);
				rgraphCtx.bindSrv(3, 0, getRenderer().getMotionVectors().getMotionVectorsRt());
				rgraphCtx.bindSrv(4, 0, hitPosRt);
				rgraphCtx.bindSrv(5, 0, classTileMapRt);

				rgraphCtx.bindUav(0, 0, transientRt1);
				rgraphCtx.bindUav(1, 0, writeMomentsRt);

				cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

				dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
			});
	}

	// Hotizontal bilateral filter
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsHorizBilateral");

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kUavCompute);

		rpass.setWork([this, transientRt1, transientRt2, writeMomentsRt, consts, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsHorizontalDenoise);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_horizontalBilateralDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt1);
			rgraphCtx.bindSrv(1, 0, writeMomentsRt);
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(3, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, transientRt2);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		});
	}

	// Vertical bilateral filter
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsVertBilateral");

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(mainRt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.setWork([this, transientRt2, mainRt, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsVerticalDenoise);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_verticalBilateralDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt2);
			rgraphCtx.bindSrv(1, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, mainRt);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		});
	}

	m_runCtx.m_rt = mainRt;
}

} // end namespace anki
