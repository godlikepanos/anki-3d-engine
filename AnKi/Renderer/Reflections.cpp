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

	if(bRtReflections)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtSbtBuild.ankiprogbin", {{"TECHNIQUE", 1}}, m_sbtProg, m_sbtBuildGrProg, "SbtBuild"));
	}

	// Ray gen and miss
	if(bRtReflections)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/Reflections.ankiprogbin", m_rtProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_rtProg);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtMaterialFetch");
		const ShaderProgramResourceVariant* variant;
		m_rtProg->getOrCreateVariant(variantInitInfo, variant);
		m_libraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

		ShaderProgramResourceVariantInitInfo variantInitInfo2(m_rtProg);
		variantInitInfo2.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtMaterialFetch");
		m_rtProg->getOrCreateVariant(variantInitInfo2, variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", {}, m_rtProg, m_spatialDenoisingGrProg, "SpatialDenoise"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", {}, m_rtProg, m_temporalDenoisingGrProg, "TemporalDenoise"));
	ANKI_CHECK(
		loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", {}, m_rtProg, m_verticalBilateralDenoisingGrProg, "BilateralDenoiseVertical"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", {}, m_rtProg, m_horizontalBilateralDenoisingGrProg,
								 "BilateralDenoiseHorizontal"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", {}, m_rtProg, m_ssrGrProg, "Ssr"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Reflections.ankiprogbin", {}, m_rtProg, m_probeFallbackGrProg, "ReflectionProbeFallback"));

	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment,
										GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize + U32(sizeof(UVec4)));

	m_transientRtDesc1 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "RtReflections #1");
	m_transientRtDesc1.bake();

	m_transientRtDesc2 = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "RtReflections #2");
	m_transientRtDesc2.bake();

	m_hitPosAndDepthRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x() / 2u, getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "HitPosAndDepth");
	m_hitPosAndDepthRtDesc.bake();

	m_hitPosRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(),
																   getRenderer().getInternalResolution().y(), Format::kR16G16B16A16_Sfloat, "HitPos");
	m_hitPosRtDesc.bake();

	TextureInitInfo texInit = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), getRenderer().getHdrFormat(), "RtReflectionsMain");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_tex = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

	texInit = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
															Format::kR32G32_Sfloat, "RtReflectionsMoments #1");
	texInit.m_usage = TextureUsageBit::kAllShaderResource | TextureUsageBit::kAllUav;
	m_momentsTextures[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);
	texInit.setName("RtReflectionsMoments #2");
	m_momentsTextures[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kSrvCompute);

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

	// SSR
	BufferView pixelsFailedSsrBuff;
	BufferHandle indirectArgsHandle;
	{
		const U32 pixelCount = getRenderer().getInternalResolution().x() * getRenderer().getInternalResolution().y();
		pixelsFailedSsrBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<PixelFailedSsr>(pixelCount);

		// Yes pixelsFailedSsrBuff has nothing to do with indirect args. We are cheating
		indirectArgsHandle = rgraph.importBuffer(pixelsFailedSsrBuff, BufferUsageBit::kNone);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("SSR");

		rpass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getBloom().getPyramidRt(), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kUavCompute);

		rpass.setWork([this, transientRt1, hitPosAndDepthRt, &ctx, pixelsFailedSsrBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtReflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_ssrGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(2, 0, getRenderer().getDepthDownscale().getRt());
			rgraphCtx.bindSrv(3, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(4, 0, getRenderer().getBloom().getPyramidRt());

			rgraphCtx.bindUav(0, 0, transientRt1);
			rgraphCtx.bindUav(1, 0, hitPosAndDepthRt);
			cmdb.bindUav(2, 0, pixelsFailedSsrBuff);
			cmdb.bindUav(3, 0, BufferView(m_indirectArgsBuffer.get()));

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			SsrConstants2 consts;
			consts.m_stepIncrement = g_ssrStepIncrementCVar;
			consts.m_maxIterations = g_ssrMaxIterationsCVar;
			consts.m_projMat00_11_22_23 = Vec4(ctx.m_matrices.m_projection(0, 0), ctx.m_matrices.m_projection(1, 1),
											   ctx.m_matrices.m_projection(2, 2), ctx.m_matrices.m_projection(2, 3));
			cmdb.setFastConstants(&consts, sizeof(consts));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y());
		});
	}

	// SBT build
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	BufferHandle visibilityDep;
	BufferView visibleRenderableIndicesBuff, buildSbtIndirectArgsBuff;
	if(bRtReflections)
	{
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
			ANKI_TRACE_SCOPED_EVENT(RtReflections);
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
			ANKI_TRACE_SCOPED_EVENT(RtReflections);
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
		rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(indirectArgsHandle, BufferUsageBit::kIndirectCompute);
		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kUavCompute);
		if(getRenderer().getGeneratedSky().isEnabled())
		{
			rpass.newTextureDependency(getRenderer().getGeneratedSky().getEnvironmentMapRt(), TextureUsageBit::kSrvCompute);
		}

		rpass.setWork([this, pixelsFailedSsrBuff, &ctx, transientRt1, hitPosAndDepthRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtReflections);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_probeFallbackGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
			cmdb.bindSrv(1, 0, pixelsFailedSsrBuff);
			cmdb.bindSrv(2, 0,
						 (GpuSceneArrays::ReflectionProbe::getSingleton().getElementCount())
							 ? GpuSceneArrays::ReflectionProbe::getSingleton().getBufferView()
							 : BufferView(&getRenderer().getDummyBuffer()));
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
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflectionsSpatialDenoise");

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosAndDepthRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(hitPosRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx, transientRt1, transientRt2, hitPosAndDepthRt, hitPosRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtReflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_spatialDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt1);
			rgraphCtx.bindSrv(1, 0, hitPosAndDepthRt);
			rgraphCtx.bindSrv(2, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(3, 0, getRenderer().getGBuffer().getColorRt(1));
			rgraphCtx.bindSrv(4, 0, getRenderer().getGBuffer().getColorRt(2));

			rgraphCtx.bindUav(0, 0, transientRt2);
			rgraphCtx.bindUav(1, 0, hitPosRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Temporal denoising
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflectionsTemporalDenoise");

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(mainRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(readMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(hitPosRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx, transientRt1, transientRt2, mainRt, readMomentsRt, writeMomentsRt, hitPosRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtReflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_temporalDenoisingGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, transientRt2);
			rgraphCtx.bindSrv(1, 0, mainRt);
			rgraphCtx.bindSrv(2, 0, readMomentsRt);
			rgraphCtx.bindSrv(3, 0, getRenderer().getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindSrv(4, 0, hitPosRt);

			rgraphCtx.bindUav(0, 0, transientRt1);
			rgraphCtx.bindUav(1, 0, writeMomentsRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Hotizontal bilateral filter
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflectionsHorizBilateral");

		rpass.newTextureDependency(transientRt1, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(writeMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(1), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx, transientRt1, transientRt2, writeMomentsRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtReflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_horizontalBilateralDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt1);
			rgraphCtx.bindSrv(1, 0, writeMomentsRt);
			rgraphCtx.bindSrv(2, 0, getRenderer().getGBuffer().getColorRt(1));

			rgraphCtx.bindUav(0, 0, transientRt2);

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Vertical bilateral filter
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtReflectionsVertBilateral");

		rpass.newTextureDependency(transientRt2, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(mainRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx, transientRt2, mainRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtReflections);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_verticalBilateralDenoisingGrProg.get());

			rgraphCtx.bindSrv(0, 0, transientRt2);

			rgraphCtx.bindUav(0, 0, mainRt);
			cmdb.bindUav(1, 0, BufferView(m_indirectArgsBuffer.get()));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	m_runCtx.m_rt = mainRt;
}

} // end namespace anki
