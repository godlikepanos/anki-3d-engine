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
	const Bool bQuarterRez = g_cvarRenderReflectionsQuarterResolution;
	const UVec2 operatingRez = (bQuarterRez) ? getRenderer().getInternalResolution() / 2 : getRenderer().getInternalResolution();

	constexpr CString kProgFname = "ShaderBinaries/Reflections.ankiprogbin";
	ShaderProgramResourcePtr prog;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(kProgFname, prog)); // Load it to avoid the loading/unloading of RendererShaderProgram

	Array<SubMutation, 3> mutation = {{{"SSR_SAMPLE_GBUFFER", bSsrSamplesGBuffer},
									   {"INDIRECT_DIFFUSE_CLIPMAPS", isIndirectDiffuseClipmapsEnabled()},
									   {"QUARTER_REZ", bQuarterRez}}};

	// Ray gen and miss
	if(bRtReflections)
	{
		ANKI_CHECK(m_libraryProg.load(kProgFname, mutation, "RtMaterialFetch", ShaderTypeBit::kRayGen));

		m_rayGenShaderGroupIdx = m_libraryProg.getShaderGroupHandleIndex();
		m_shaderGroupHandlesBuff = m_libraryProg.getShaderGroupHandlesBuffer();

		ANKI_CHECK(m_missProg.load("ShaderBinaries/RtMaterialFetchMiss.ankiprogbin", {}, "RtMaterialFetch", ShaderTypeBit::kMiss));
		m_missShaderGroupIdx = m_missProg.getShaderGroupHandleIndex();

		m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
											GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

		ANKI_CHECK(m_rtMaterialFetchInlineRtProg.load(kProgFname, mutation, "RtMaterialFetchInlineRt"));
	}

	ANKI_CHECK(m_spatialAccumulationProg.load(kProgFname, mutation, "SpatialAccumulation"));
	ANKI_CHECK(m_temporalDenoisingProg.load(kProgFname, mutation, "TemporalDenoise"));
	ANKI_CHECK(m_spatialDenoisingProg.load(kProgFname, mutation, "SpatialDenoise"));
	ANKI_CHECK(m_ssrProg.load(kProgFname, mutation, "Ssr"));
	ANKI_CHECK(m_probeFallbackProg.load(kProgFname, mutation, "ReflectionProbeFallback"));
	ANKI_CHECK(m_tileClassificationProg.load(kProgFname, mutation, "Classification"));
	ANKI_CHECK(m_upscalingProg.load(kProgFname, mutation, "Upscale"));

	m_colorAndDepth1RtDesc =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat, "Reflections: Color+Depth #1");
	m_colorAndDepth1RtDesc.bake();

	m_colorAndDepth2RtDesc =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat, "Reflections: Color+Depth #2");
	m_colorAndDepth2RtDesc.bake();

	m_hitPosAndPdf1RtDesc =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat, "Reflections: Hitpos+Pdf #1");
	m_hitPosAndPdf1RtDesc.bake();

	m_hitPosAndPdf2RtDesc =
		getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR16G16B16A16_Sfloat, "Reflections: Hitpos+Pdf #2");
	m_hitPosAndPdf2RtDesc.bake();

	TextureInitInfo texInit = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, getRenderer().getHdrFormat(), "Reflections: Final");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_tex = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	texInit = getRenderer().create2DRenderTargetDescription(operatingRez.x, operatingRez.y, Format::kR32G32_Sfloat, "Reflections: Moments #1");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_momentsTex[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);
	texInit.setName("Reflections: Moments #2");
	m_momentsTex[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	m_classTileMapRtDesc = getRenderer().create2DRenderTargetDescription(
		(operatingRez.x + kTileSize - 1) / kTileSize, (operatingRez.y + kTileSize - 1) / kTileSize, Format::kR8_Uint, "Reflections: ClassTileMap");
	m_classTileMapRtDesc.bake();

	{
		m_indirectArgsBuffer = getRenderer().getRendedererGpuMemoryPool().allocateStructuredBuffer<DispatchIndirectArgs>(2);

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
	const Bool bQuarterRez = g_cvarRenderReflectionsQuarterResolution;

	// Create or import render targets
	const RenderTargetHandle finalRt = rgraph.importRenderTarget(m_tex.get(), !m_texImportedOnce, TextureUsageBit::kAllSrv);
	const RenderTargetHandle readMomentsRt =
		rgraph.importRenderTarget(m_momentsTex[getRenderer().getFrameCount() & 1].get(), !m_texImportedOnce, TextureUsageBit::kAllSrv);
	const RenderTargetHandle writeMomentsRt =
		rgraph.importRenderTarget(m_momentsTex[(getRenderer().getFrameCount() + 1) & 1].get(), !m_texImportedOnce, TextureUsageBit::kAllSrv);
	m_texImportedOnce = true;

	const RenderTargetHandle colorAndDepth1Rt = rgraph.newRenderTarget(m_colorAndDepth1RtDesc);
	const RenderTargetHandle colorAndDepth2Rt = rgraph.newRenderTarget(m_colorAndDepth2RtDesc);
	const RenderTargetHandle hitPosAndPdf1Rt = rgraph.newRenderTarget(m_hitPosAndPdf1RtDesc);
	const RenderTargetHandle hitPosAndPdf2Rt = rgraph.newRenderTarget(m_hitPosAndPdf2RtDesc);
	const RenderTargetHandle classTileMapRt = rgraph.newRenderTarget(m_classTileMapRtDesc);
	const BufferHandle indirectArgsHandle = rgraph.importBuffer(m_indirectArgsBuffer, BufferUsageBit::kNone);

	ReflectionConstants consts;
	consts.m_ssrStepIncrement = g_cvarRenderReflectionsSsrStepIncrement;
	consts.m_ssrMaxIterations = g_cvarRenderReflectionsSsrMaxIterations;
	consts.m_roughnessCutoffToGiEdges = Vec2(g_cvarRenderReflectionsRoughnessCutoffToGiEdge0, g_cvarRenderReflectionsRoughnessCutoffToGiEdge1);

	// Classification
	{
		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Tile classification");

		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, classTileMapRt, consts, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsClassification);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tileClassificationProg.get());

			rgraphCtx.bindSrv(0, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(1, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
			rgraphCtx.bindUav(0, 0, classTileMapRt);
			cmdb.bindUav(1, 0, m_indirectArgsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, kTileSize, kTileSize, rez.x, rez.y);
		});
	}

	// SSR
	BufferView pixelsFailedSsrBuff;
	{
		U32 pixelCount = getRenderer().getInternalResolution().x * getRenderer().getInternalResolution().y;
		if(bQuarterRez)
		{
			pixelCount /= 4;
		}
		pixelsFailedSsrBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<PixelFailedSsr>(pixelCount);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: SSR");

		rpass.newTextureDependency(getDepthDownscale().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(0), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getDepthDownscale().getNormalsRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getBloom().getPyramidRt(), TextureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);
		rpass.newTextureDependency(getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(colorAndDepth1Rt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndPdf1Rt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork(
			[this, colorAndDepth1Rt, hitPosAndPdf1Rt, pixelsFailedSsrBuff, consts, classTileMapRt, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(ReflectionsSsr);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_ssrProg.get());

				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
				cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());
				cmdb.bindSampler(2, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

				rgraphCtx.bindSrv(0, 0, getGBuffer().getColorRt(0));
				rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(1));
				rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));
				rgraphCtx.bindSrv(3, 0, getDepthDownscale().getNormalsRt());
				rgraphCtx.bindSrv(4, 0, getDepthDownscale().getDepthRt());
				rgraphCtx.bindSrv(5, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
				rgraphCtx.bindSrv(6, 0, getRenderer().getBloom().getPyramidRt());
				cmdb.bindSrv(7, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
				cmdb.bindSrv(8, 0, getClusterBinning().getClustersBuffer());
				rgraphCtx.bindSrv(9, 0, getShadowMapping().getShadowmapRt());
				rgraphCtx.bindSrv(10, 0, classTileMapRt);
				cmdb.bindSrv(11, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));

				rgraphCtx.bindUav(0, 0, colorAndDepth1Rt);
				rgraphCtx.bindUav(1, 0, hitPosAndPdf1Rt);
				cmdb.bindUav(2, 0, pixelsFailedSsrBuff);
				cmdb.bindUav(3, 0, m_indirectArgsBuffer);

				cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

				cmdb.setFastConstants(&consts, sizeof(consts));

				UVec2 rez = getRenderer().getInternalResolution();
				if(bQuarterRez)
				{
					rez /= 2;
				}

				dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
			});
	}

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	if(bRtReflections && !g_cvarRenderReflectionsInlineRt)
	{
		buildShaderBindingTablePass("Reflections: Build SBT", m_shaderGroupHandlesBuff, m_rayGenShaderGroupIdx, m_missShaderGroupIdx, m_sbtRecordSize,
									rgraph, sbtHandle, sbtBuffer);
	}

	// Ray gen
	if(bRtReflections)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Dispatch rays");

		const TextureUsageBit uavTexUsage = (g_cvarRenderReflectionsInlineRt) ? TextureUsageBit::kUavCompute : TextureUsageBit::kUavDispatchRays;
		const BufferUsageBit indirectBuffUsage =
			(g_cvarRenderReflectionsInlineRt) ? BufferUsageBit::kIndirectCompute : BufferUsageBit::kIndirectDispatchRays;
		const TextureUsageBit srvTexUsage = (g_cvarRenderReflectionsInlineRt) ? TextureUsageBit::kSrvCompute : TextureUsageBit::kSrvDispatchRays;

		if(!g_cvarRenderReflectionsInlineRt)
		{
			rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		}
		rpass.newTextureDependency(colorAndDepth1Rt, uavTexUsage);
		rpass.newTextureDependency(hitPosAndPdf1Rt, uavTexUsage);
		rpass.newBufferDependency(indirectArgsHandle, indirectBuffUsage);
		setRgenSpace2Dependencies(rpass, g_cvarRenderReflectionsInlineRt);

		if(bQuarterRez)
		{
			rpass.newTextureDependency(getDepthDownscale().getDepthRt(), srvTexUsage);
		}

		if(isIndirectDiffuseClipmapsEnabled())
		{
			getIndirectDiffuseClipmaps().setDependencies(rpass, srvTexUsage);
		}

		rpass.setWork([this, sbtBuffer, colorAndDepth1Rt, hitPosAndPdf1Rt, pixelsFailedSsrBuff, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsRayGen);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram((g_cvarRenderReflectionsInlineRt) ? m_rtMaterialFetchInlineRtProg.get() : m_libraryProg.get());

			// Space 0 globals
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

			bindRgenSpace2Resources(rgraphCtx);
			if(bQuarterRez)
			{
				rgraphCtx.bindSrv(4, 2, getDepthDownscale().getDepthRt());
			}
			cmdb.bindSrv(7, 2, pixelsFailedSsrBuff);
			rgraphCtx.bindUav(0, 2, colorAndDepth1Rt);
			rgraphCtx.bindUav(1, 2, hitPosAndPdf1Rt);

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
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Probe fallback");

		rpass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		rpass.newTextureDependency(colorAndDepth1Rt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndPdf1Rt, TextureUsageBit::kUavCompute);
		if(getGeneratedSky().isEnabled())
		{
			rpass.newTextureDependency(getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvCompute);
		}

		if(getProbeReflections().getHasCurrentlyRefreshedReflectionRt())
		{
			rpass.newTextureDependency(getProbeReflections().getCurrentlyRefreshedReflectionRt(), TextureUsageBit::kSrvCompute);
		}

		rpass.setWork([this, pixelsFailedSsrBuff, colorAndDepth1Rt, hitPosAndPdf1Rt, bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsProbeFallback);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_probeFallbackProg.get());

			rgraphCtx.bindSrv(0, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
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

			rgraphCtx.bindUav(0, 0, colorAndDepth1Rt);
			rgraphCtx.bindUav(1, 0, hitPosAndPdf1Rt);

			cmdb.dispatchComputeIndirect(BufferView(m_indirectArgsBuffer).incrementOffset(sizeof(DispatchIndirectArgs)));
		});
	}

	// Spatial accumulation
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Spatial accumulation");

		rpass.newTextureDependency(colorAndDepth1Rt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosAndPdf1Rt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getNormalsRt() : getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(colorAndDepth2Rt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndPdf2Rt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, colorAndDepth1Rt, colorAndDepth2Rt, hitPosAndPdf1Rt, hitPosAndPdf2Rt, consts, classTileMapRt,
					   bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsSpatialDenoise);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialAccumulationProg.get());

			rgraphCtx.bindSrv(0, 0, colorAndDepth1Rt);
			rgraphCtx.bindSrv(1, 0, hitPosAndPdf1Rt);
			rgraphCtx.bindSrv(2, 0, (bQuarterRez) ? getDepthDownscale().getDepthRt() : getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(3, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(4, 0, (bQuarterRez) ? getDepthDownscale().getNormalsRt() : getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(5, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, colorAndDepth2Rt);
			rgraphCtx.bindUav(1, 0, hitPosAndPdf2Rt);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Temporal denoising
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Temporal denoise");

		rpass.newTextureDependency(colorAndDepth2Rt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(finalRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(readMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency((bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt(),
								   TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosAndPdf2Rt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(colorAndDepth1Rt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, colorAndDepth1Rt, hitPosAndPdf2Rt, colorAndDepth2Rt, finalRt, readMomentsRt, writeMomentsRt, classTileMapRt, bQuarterRez,
					   consts](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsTemporalDenoise);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_temporalDenoisingProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, colorAndDepth2Rt);
			rgraphCtx.bindSrv(1, 0, finalRt);
			rgraphCtx.bindSrv(2, 0, readMomentsRt);
			rgraphCtx.bindSrv(3, 0,
							  (bQuarterRez) ? getDepthDownscale().getAdjustedMotionVectorsRt() : getMotionVectors().getAdjustedMotionVectorsRt());
			rgraphCtx.bindSrv(4, 0, hitPosAndPdf2Rt);
			rgraphCtx.bindSrv(5, 0, classTileMapRt);
			rgraphCtx.bindSrv(6, 0, getGBuffer().getColorRt(1));

			rgraphCtx.bindUav(0, 0, colorAndDepth1Rt);
			rgraphCtx.bindUav(1, 0, writeMomentsRt);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Spatial denois
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Spatial denoise");

		rpass.newTextureDependency(colorAndDepth1Rt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency((bQuarterRez) ? colorAndDepth2Rt : finalRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, colorAndDepth1Rt, colorAndDepth2Rt, writeMomentsRt, finalRt, consts, classTileMapRt,
					   bQuarterRez](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsBilateral);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialDenoisingProg.get());

			rgraphCtx.bindSrv(0, 0, colorAndDepth1Rt);
			rgraphCtx.bindSrv(1, 0, writeMomentsRt);
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(3, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, (bQuarterRez) ? colorAndDepth2Rt : finalRt);

			cmdb.setFastConstants(&consts, sizeof(consts));

			UVec2 rez = getRenderer().getInternalResolution();
			if(bQuarterRez)
			{
				rez /= 2;
			}

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	// Upscale
	if(bQuarterRez)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Reflections: Upscale");

		rpass.newTextureDependency(colorAndDepth2Rt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(finalRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, colorAndDepth2Rt, finalRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(ReflectionsUpscale);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscalingProg.get());

			rgraphCtx.bindSrv(0, 0, colorAndDepth2Rt);
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));

			rgraphCtx.bindUav(0, 0, finalRt);

			const UVec2 rez = getRenderer().getInternalResolution() / 2;

			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		});
	}

	m_runCtx.m_rt = finalRt;
}

} // end namespace anki
