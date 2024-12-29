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
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/ImageAtlasResource.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>

namespace anki {

Error Reflections::init()
{
	const Bool bRtReflections = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled && g_rtReflectionsCVar;
	const Bool bSsrSamplesGBuffer = bRtReflections;

	std::initializer_list<SubMutation> mutation = {{"SSR_SAMPLE_GBUFFER", bSsrSamplesGBuffer}};

	if(bRtReflections)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtProg, m_sbtBuildGrProg, "SbtBuild"));
	}

	// Ray gen and miss
	if(bRtReflections)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/Reflections.ankiprogbin", m_mainProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_mainProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		variantInitInfo.addMutation("SSR_SAMPLE_GBUFFER", bSsrSamplesGBuffer);
		const ShaderProgramResourceVariant* variant;
		m_mainProg->getOrCreateVariant(variantInitInfo, variant);
		m_libraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

		ShaderProgramResourceVariantInitInfo variantInitInfo2(m_mainProg);
		variantInitInfo2.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtMaterialFetch");
		variantInitInfo2.addMutation("SSR_SAMPLE_GBUFFER", bSsrSamplesGBuffer);
		m_mainProg->getOrCreateVariant(variantInitInfo2, variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();

		m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
											GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_spatialDenoisingGrProg, "SpatialDenoise"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_temporalDenoisingGrProg, "TemporalDenoise"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_verticalBilateralDenoisingGrProg,
								 "BilateralDenoiseVertical"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_horizontalBilateralDenoisingGrProg,
								 "BilateralDenoiseHorizontal"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_ssrGrProg, "Ssr"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_probeFallbackGrProg, "ReflectionProbeFallback"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", mutation, m_mainProg, m_tileClassificationGrProg, "Classification"));

	m_transientRtDesc1 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "Reflections #1");
	m_transientRtDesc1.bake();

	m_transientRtDesc2 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "Reflections #2");
	m_transientRtDesc2.bake();

	m_hitPosAndDepthRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2u, getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "HitPosAndDepth");
	m_hitPosAndDepthRtDesc.bake();

	m_hitPosRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(),
																   getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "HitPos");
	m_hitPosRtDesc.bake();

	TextureInitInfo texInit = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), getRenderer().getHdrFormat(), "ReflectionsMain");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_tex = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	texInit = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
															Format::kR32G32_Sfloat, "ReflectionsMoments #1");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_momentsTextures[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);
	texInit.setName("ReflectionsMoments #2");
	m_momentsTextures[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	m_classTileMapRtDesc = getRenderer().create2DRenderTargetDescription((getRenderer().getInternalResolution().x() + kTileSize - 1) / kTileSize,
																		 (getRenderer().getInternalResolution().y() + kTileSize - 1) / kTileSize,
																		 Format::kR8_Uint, "ReflClassTileMap");
	m_classTileMapRtDesc.bake();

	{
		BufferInitInfo buffInit("ReflRayGenIndirectArgs");
		buffInit.m_size = sizeof(DispatchIndirectArgs) * 2;
		buffInit.m_usage = BufferUsageBit::kAllIndirect | BufferUsageBit::kUavCompute | BufferUsageBit::kCopyDestination;
		m_indirectArgsBuffer = GrManager::getSingleton().newBuffer(buffInit);

		CommandBufferInitInfo cmdbInit("Init buffer");
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		U32 offset = 0;
		cmdb->fillBuffer(BufferView(m_indirectArgsBuffer.get(), offset, sizeof(U32)), 0);
		offset += sizeof(U32);
		cmdb->fillBuffer(BufferView(m_indirectArgsBuffer.get(), offset, 2 * sizeof(U32)), 1);

		offset += sizeof(U32) * 2;
		cmdb->fillBuffer(BufferView(m_indirectArgsBuffer.get(), offset, sizeof(U32)), 0);
		offset += sizeof(U32);
		cmdb->fillBuffer(BufferView(m_indirectArgsBuffer.get(), offset, 2 * sizeof(U32)), 1);

		FencePtr fence;
		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get(), {}, &fence);

		fence->clientWait(16.0_sec);
	}

	return Error::kNone;
}

void Reflections::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const Bool bRtReflections = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled && g_rtReflectionsCVar;

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
		mainRt = rgraph.importRenderTarget(m_tex.get(), TextureUsageBit::kAllCompute);
		readMomentsRt = rgraph.importRenderTarget(m_momentsTextures[getRenderer().getFrameCount() & 1].get(), TextureUsageBit::kAllCompute);
		writeMomentsRt = rgraph.importRenderTarget(m_momentsTextures[(getRenderer().getFrameCount() + 1) & 1].get(), TextureUsageBit::kAllCompute);
		m_texImportedOnce = true;
	}

	const RenderTargetHandle transientRt1 = rgraph.newRenderTarget(m_transientRtDesc1);
	const RenderTargetHandle transientRt2 = rgraph.newRenderTarget(m_transientRtDesc2);
	const RenderTargetHandle hitPosAndDepthRt = rgraph.newRenderTarget(m_hitPosAndDepthRtDesc);
	const RenderTargetHandle hitPosRt = rgraph.newRenderTarget(m_hitPosRtDesc);
	const RenderTargetHandle classTileMapRt = rgraph.newRenderTarget(m_classTileMapRtDesc);
	const BufferHandle indirectArgsHandle = rgraph.importBuffer(BufferView(m_indirectArgsBuffer.get()), BufferUsageBit::kNone);

	ReflectionConstants consts;
	consts.m_ssrStepIncrement = g_ssrStepIncrementCVar;
	consts.m_ssrMaxIterations = g_ssrMaxIterationsCVar;
	consts.m_roughnessCutoffToGiEdges = Vec2(g_roughnessCutoffToGiEdge0, g_roughnessCutoffToGiEdge1);

	// Classification
	{
		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflTileClassification");

		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, classTileMapRt, consts](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tileClassificationGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindUav(0, 0, classTileMapRt);
			cmdb.bindUav(1, 0, BufferView(m_indirectArgsBuffer.get()));

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, kTileSize / 2, kTileSize, getRenderer().getInternalResolution().x() / 2,
							  getRenderer().getInternalResolution().y());
		});
	}

	// SSR
	BufferView pixelsFailedSsrBuff;
	{
		const U32 pixelCount = getRenderer().getInternalResolution().x() / 2 * getRenderer().getInternalResolution().y();
		pixelsFailedSsrBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<PixelFailedSsr>(pixelCount);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SSR");

		rpass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(0), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getBloom().getPyramidRt(), TextureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getRenderer().getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, transientRt1, hitPosAndDepthRt, &ctx, pixelsFailedSsrBuff, consts, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_ssrGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getColorRt(0));
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(2, 0, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(3, 0, getRenderer().getDepthDownscale().getRt());
			rgraphCtx.bindSrv(4, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(5, 0, getRenderer().getBloom().getPyramidRt());
			cmdb.bindSrv(6, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
			cmdb.bindSrv(7, 0, getRenderer().getClusterBinning().getClustersBuffer());
			rgraphCtx.bindSrv(8, 0, getRenderer().getShadowMapping().getShadowmapRt());
			rgraphCtx.bindSrv(9, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, transientRt1);
			rgraphCtx.bindUav(1, 0, hitPosAndDepthRt);
			cmdb.bindUav(2, 0, pixelsFailedSsrBuff);
			cmdb.bindUav(3, 0, BufferView(m_indirectArgsBuffer.get()));

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y());
		});
	}

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	if(bRtReflections)
	{
		BufferHandle visibilityDep;
		BufferView visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff;
		getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(visibilityDep, visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff);

		// Allocate SBT
		U32 sbtAlignment = (GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
							   ? sizeof(U32)
							   : GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
		sbtAlignment = computeCompoundAlignment(sbtAlignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);
		U8* sbtMem;
		sbtBuffer = RebarTransientMemoryPool::getSingleton().allocate(
			(GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount() + 2) * m_sbtRecordSize, sbtAlignment, sbtMem);
		sbtHandle = rgraph.importBuffer(sbtBuffer, BufferUsageBit::kNone);

		// Write the first 2 entries of the SBT
		ConstWeakArray<U8> shaderGroupHandles = m_libraryGrProg->getShaderGroupHandles();
		const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
		memcpy(sbtMem, &shaderGroupHandles[m_rayGenShaderGroupIdx * shaderHandleSize], shaderHandleSize);
		memcpy(sbtMem + m_sbtRecordSize, &shaderGroupHandles[m_missShaderGroupIdx * shaderHandleSize], shaderHandleSize);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflections build SBT");

		rpass.newBufferDependency(visibilityDep, BufferUsageBit::kIndirectCompute | BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, buildSbtIndirectArgsBuff, sbtBuffer, visibleRenderableIndicesBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_sbtBuildGrProg.get());

			cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(1, 0, visibleRenderableIndicesBuff);
			cmdb.bindSrv(2, 0, BufferView(&m_libraryGrProg->getShaderGroupHandlesGpuBuffer()));

			cmdb.bindUav(0, 0, sbtBuffer);

			RtShadowsSbtBuildConstants consts = {};
			ANKI_ASSERT(m_sbtRecordSize % 4 == 0);
			consts.m_sbtRecordDwordSize = m_sbtRecordSize / 4;
			const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
			ANKI_ASSERT(shaderHandleSize % 4 == 0);
			consts.m_shaderHandleDwordSize = shaderHandleSize / 4;
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchComputeIndirect(buildSbtIndirectArgsBuff);
		});
	}

	// Ray gen
	if(bRtReflections)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflections");

		rpass.newBufferDependency(sbtHandle, BufferUsageBit::kShaderBindingTable);
		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavTraceRays);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavTraceRays);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvTraceRays);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvTraceRays);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvTraceRays);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			rpass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvTraceRays);
		}
		rpass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvTraceRays);
		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kTraceRaysSrv);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectTraceRays);

		rpass.setWork([this, sbtBuffer, &ctx, transientRt1, hitPosAndDepthRt, pixelsFailedSsrBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_libraryGrProg.get());

			// More globals
			cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESH_LODS, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

			cmdb.bindConstantBuffer(0, 2, ctx.m_globalRenderingConstantsBuffer);

			rgraphCtx.bindSrv(0, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());
			rgraphCtx.bindSrv(1, 2, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(2, 2, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(3, 2, getRenderer().getGBuffer().getColorRt(2));

			const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			const Bool bSkySolidColor =
				(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
			if(bSkySolidColor)
			{
				cmdb.bindSrv(4, 2, TextureView(&getRenderer().getDummyTexture2d(), TextureSubresourceDesc::all()));
			}
			else if(sky->getSkyboxType() == SkyboxType::kImage2D)
			{
				cmdb.bindSrv(4, 2, TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				rgraphCtx.bindSrv(4, 2, getRenderer().getGeneratedSky().getEnvironmentMapRt());
			}

			cmdb.bindSrv(5, 2, GpuSceneArrays::GlobalIlluminationProbe::getSingleton().getBufferView());
			cmdb.bindSrv(6, 2, pixelsFailedSsrBuff);
			rgraphCtx.bindSrv(7, 2, getRenderer().getShadowMapping().getShadowmapRt());

			rgraphCtx.bindUav(0, 2, transientRt1);
			rgraphCtx.bindUav(1, 2, hitPosAndDepthRt);

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClampShadow.get());

			const Vec4 consts(g_rtReflectionsMaxRayDistanceCVar);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.traceRaysIndirect(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
								   BufferView(m_indirectArgsBuffer.get()).setRange(sizeof(DispatchIndirectArgs)));
		});
	}
	else
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionProbeFallback");

		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newBufferDependency(getRenderer().getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavCompute);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			rpass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvCompute);
		}

		if(getRenderer().getProbeReflections().getHasCurrentlyRefreshedReflectionRt())
		{
			rpass.newTextureDependency(getRenderer().getProbeReflections().getCurrentlyRefreshedReflectionRt(), TextureUsageBit::kSrvCompute);
		}

		rpass.setWork([this, pixelsFailedSsrBuff, &ctx, transientRt1, hitPosAndDepthRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_probeFallbackGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
			cmdb.bindSrv(1, 0, pixelsFailedSsrBuff);
			cmdb.bindSrv(2, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kReflectionProbe));
			cmdb.bindSrv(3, 0, getRenderer().getClusterBinning().getClustersBuffer());
			cmdb.bindSrv(4, 0, BufferView(m_indirectArgsBuffer.get()).setRange(sizeof(U32)));

			const LightComponent* dirLight = SceneGraph::getSingleton().getDirectionalLight();
			const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();
			const Bool bSkySolidColor =
				(!sky || sky->getSkyboxType() == SkyboxType::kSolidColor || (!dirLight && sky->getSkyboxType() == SkyboxType::kGenerated));
			if(bSkySolidColor)
			{
				cmdb.bindSrv(5, 0, TextureView(&getRenderer().getDummyTexture2d(), TextureSubresourceDesc::all()));
			}
			else if(sky->getSkyboxType() == SkyboxType::kImage2D)
			{
				cmdb.bindSrv(5, 0, TextureView(&sky->getImageResource().getTexture(), TextureSubresourceDesc::all()));
			}
			else
			{
				rgraphCtx.bindSrv(5, 0, getRenderer().getGeneratedSky().getEnvironmentMapRt());
			}

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindUav(0, 0, transientRt1);
			rgraphCtx.bindUav(1, 0, hitPosAndDepthRt);

			cmdb.dispatchComputeIndirect(BufferView(m_indirectArgsBuffer.get()).incrementOffset(sizeof(DispatchIndirectArgs)));
		});
	}

	// Spatial denoising
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsSpatialDenoise");

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx, transientRt1, transientRt2, hitPosAndDepthRt, hitPosRt, consts, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt1);
			rgraphCtx.bindSrv(1, 0, hitPosAndDepthRt);
			rgraphCtx.bindSrv(2, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(3, 0, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(4, 0, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(5, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, transientRt2);
			rgraphCtx.bindUav(1, 0, hitPosRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
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

		rpass.setWork([this, &ctx, transientRt1, transientRt2, mainRt, readMomentsRt, writeMomentsRt, hitPosRt,
					   classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);

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

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Hotizontal bilateral filter
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsHorizBilateral");

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kUavCompute);

		rpass.setWork([this, transientRt1, transientRt2, writeMomentsRt, consts, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_horizontalBilateralDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt1);
			rgraphCtx.bindSrv(1, 0, writeMomentsRt);
			rgraphCtx.bindSrv(2, 0, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(3, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, transientRt2);

			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Vertical bilateral filter
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("ReflectionsVertBilateral");

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(mainRt, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(classTileMapRt, TextureUsageBit::kSrvCompute);

		rpass.setWork([this, transientRt2, mainRt, classTileMapRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Reflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_verticalBilateralDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt2);
			rgraphCtx.bindSrv(1, 0, classTileMapRt);

			rgraphCtx.bindUav(0, 0, mainRt);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	m_runCtx.m_rt = mainRt;
}

} // end namespace anki
