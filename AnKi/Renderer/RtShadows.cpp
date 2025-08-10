// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RtShadows.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/AccelerationStructureBuilder.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>

namespace anki {

Error RtShadows::init()
{
	m_useSvgf = g_rtShadowsSvgfCVar;
	m_atrousPassCount = g_rtShadowsSvgfAtrousPassCountCVar;

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_blueNoiseImage));

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSetupSbtBuild.ankiprogbin", m_setupBuildSbtProg, m_setupBuildSbtGrProg));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSbtBuild.ankiprogbin", m_buildSbtProg, m_buildSbtGrProg));
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtShadows.ankiprogbin", m_rayGenAndMissProg));

	// Ray gen and miss
	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_rayGenAndMissProg);
		variantInitInfo.addMutation("RAYS_PER_PIXEL", g_rtShadowsRaysPerPixelCVar);
		variantInitInfo.requestTechniqueAndTypes(ShaderTypeBit::kRayGen, "RtShadows");
		const ShaderProgramResourceVariant* variant;
		m_rayGenAndMissProg->getOrCreateVariant(variantInitInfo, variant);
		m_rtLibraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();

		ShaderProgramResourceVariantInitInfo variantInitInfo2(m_rayGenAndMissProg);
		variantInitInfo2.addMutation("RAYS_PER_PIXEL", g_rtShadowsRaysPerPixelCVar);
		variantInitInfo2.requestTechniqueAndTypes(ShaderTypeBit::kMiss, "RtShadows");
		m_rayGenAndMissProg->getOrCreateVariant(variantInitInfo2, variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();
	}

	// Denoise program
	if(!m_useSvgf)
	{
		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/RtShadowsDenoise.ankiprogbin", {{"BLUR_ORIENTATION", 0}}, m_denoiseProg, m_grDenoiseHorizontalProg));

		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/RtShadowsDenoise.ankiprogbin", {{"BLUR_ORIENTATION", 1}}, m_denoiseProg, m_grDenoiseVerticalProg));
	}

	// SVGF variance program
	if(m_useSvgf)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSvgfVariance.ankiprogbin", m_svgfVarianceProg, m_svgfVarianceGrProg));
	}

	// SVGF atrous program
	if(m_useSvgf)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSvgfAtrous.ankiprogbin", {{"LAST_PASS", 0}}, m_svgfAtrousProg, m_svgfAtrousGrProg));
		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/RtShadowsSvgfAtrous.ankiprogbin", {{"LAST_PASS", 1}}, m_svgfAtrousProg, m_svgfAtrousLastPassGrProg));
	}

	// Upscale program
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsUpscale.ankiprogbin", m_upscaleProg, m_upscaleGrProg));

	// Quarter rez shadow RT
	{
		TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
			getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR8_Unorm,
			TextureUsageBit::kAllSrv | TextureUsageBit::kUavDispatchRays | TextureUsageBit::kUavCompute, "RtShadows History");
		m_historyRt = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);
	}

	// Temp shadow RT
	{
		m_intermediateShadowsRtDescr = getRenderer().create2DRenderTargetDescription(
			getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR8_Unorm, "RtShadows Tmp");
		m_intermediateShadowsRtDescr.bake();
	}

	// Moments RT
	{
		TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
			getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR32G32_Sfloat,
			TextureUsageBit::kAllSrv | TextureUsageBit::kUavDispatchRays | TextureUsageBit::kUavCompute, "RtShadows Moments #1");
		m_momentsRts[0] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);

		texinit.setName("RtShadows Moments #2");
		m_momentsRts[1] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);
	}

	// Variance RT
	if(m_useSvgf)
	{
		m_varianceRtDescr = getRenderer().create2DRenderTargetDescription(
			getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR32_Sfloat, "RtShadows Variance");
		m_varianceRtDescr.bake();
	}

	// Final RT
	{
		m_upscaledRtDescr = getRenderer().create2DRenderTargetDescription(
			getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR8_Unorm, "RtShadows Upscaled");
		m_upscaledRtDescr.bake();
	}

	{
		TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
			getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR32_Sfloat,
			TextureUsageBit::kAllSrv | TextureUsageBit::kUavDispatchRays | TextureUsageBit::kUavCompute, "RtShadows history len");
		ClearValue clear;
		clear.m_colorf[0] = 1.0f;
		m_dummyHistoryLenTex = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel, clear);
	}

	// Misc
	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment, m_sbtRecordSize);

	return Error::kNone;
}

void RtShadows::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RtShadows);

#define ANKI_DEPTH_DEP \
	getDepthDownscale().getRt(), TextureUsageBit::kSrvDispatchRays | TextureUsageBit::kSrvCompute, DepthDownscale::kQuarterInternalResolution

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Import RTs
	{
		const U32 prevRtIdx = getRenderer().getFrameCount() & 1;

		if(!m_rtsImportedOnce) [[unlikely]]
		{
			m_runCtx.m_historyRt = rgraph.importRenderTarget(m_historyRt.get(), TextureUsageBit::kSrvPixel);

			m_runCtx.m_prevMomentsRt = rgraph.importRenderTarget(m_momentsRts[prevRtIdx].get(), TextureUsageBit::kSrvPixel);

			m_rtsImportedOnce = true;
		}
		else
		{
			m_runCtx.m_historyRt = rgraph.importRenderTarget(m_historyRt.get());
			m_runCtx.m_prevMomentsRt = rgraph.importRenderTarget(m_momentsRts[prevRtIdx].get());
		}

		if((getPassCountWithoutUpscaling() % 2) == 1)
		{
			m_runCtx.m_intermediateShadowsRts[0] = rgraph.newRenderTarget(m_intermediateShadowsRtDescr);
			m_runCtx.m_intermediateShadowsRts[1] = rgraph.newRenderTarget(m_intermediateShadowsRtDescr);
		}
		else
		{
			// We can save a render target if we have even number of renderpasses
			m_runCtx.m_intermediateShadowsRts[0] = rgraph.newRenderTarget(m_intermediateShadowsRtDescr);
			m_runCtx.m_intermediateShadowsRts[1] = m_runCtx.m_historyRt;
		}

		m_runCtx.m_currentMomentsRt = rgraph.importRenderTarget(m_momentsRts[!prevRtIdx].get(), TextureUsageBit::kNone);

		if(m_useSvgf)
		{
			if(m_atrousPassCount > 1)
			{
				m_runCtx.m_varianceRts[0] = rgraph.newRenderTarget(m_varianceRtDescr);
			}
			m_runCtx.m_varianceRts[1] = rgraph.newRenderTarget(m_varianceRtDescr);
		}

		m_runCtx.m_upscaledRt = rgraph.newRenderTarget(m_upscaledRtDescr);
	}

	// Setup build SBT dispatch
	BufferHandle sbtBuildIndirectArgsHandle;
	BufferView sbtBuildIndirectArgsBuffer;
	{
		sbtBuildIndirectArgsBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<DispatchIndirectArgs>(1);
		sbtBuildIndirectArgsHandle = rgraph.importBuffer(sbtBuildIndirectArgsBuffer, BufferUsageBit::kUavCompute);

		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows setup build SBT");

		rpass.newBufferDependency(sbtBuildIndirectArgsHandle, BufferUsageBit::kAccelerationStructureBuild);

		rpass.setWork([this, sbtBuildIndirectArgsBuffer](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_setupBuildSbtGrProg.get());

			cmdb.bindSrv(0, 0, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getBufferView());
			cmdb.bindUav(0, 0, sbtBuildIndirectArgsBuffer);

			cmdb.dispatchCompute(1, 1, 1);
		});
	}

	// Build the SBT
	BufferHandle sbtHandle;
	BufferView sbtBuffer;
	{
		// Allocate SBT
		WeakArray<U32> sbtMem;
		sbtBuffer = RebarTransientMemoryPool::getSingleton().allocateStructuredBuffer(
			(GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount() + 2) * m_sbtRecordSize / sizeof(U32), sbtMem);
		sbtHandle = rgraph.importBuffer(sbtBuffer, BufferUsageBit::kUavCompute);

		// Write the first 2 entries of the SBT
		ConstWeakArray<U8> shaderGroupHandles = m_rtLibraryGrProg->getShaderGroupHandles();
		const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
		memcpy(&sbtMem[0], &shaderGroupHandles[m_rayGenShaderGroupIdx * shaderHandleSize], shaderHandleSize);
		memcpy(&sbtMem[m_sbtRecordSize / sizeof(U32)], &shaderGroupHandles[m_missShaderGroupIdx * shaderHandleSize], shaderHandleSize);

		// Create the pass
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows build SBT");

		AccelerationStructureVisibilityInfo asVis;
		GpuVisibilityLocalLightsOutput localLightsVis;
		getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(asVis, localLightsVis);

		rpass.newBufferDependency(asVis.m_depedency, BufferUsageBit::kSrvCompute);
		rpass.newBufferDependency(sbtBuildIndirectArgsHandle, BufferUsageBit::kIndirectCompute);

		rpass.setWork([this, sbtBuildIndirectArgsBuffer, sbtBuffer,
					   visibleRenderableIndicesBuff = asVis.m_visibleRenderablesBuffer](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_buildSbtGrProg.get());

			cmdb.bindSrv(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
			cmdb.bindSrv(1, 0, BufferView(&GpuSceneBuffer::getSingleton().getBuffer()));
			cmdb.bindSrv(2, 0, visibleRenderableIndicesBuff);
			cmdb.bindSrv(3, 0, BufferView(&m_rtLibraryGrProg->getShaderGroupHandlesGpuBuffer()));
			cmdb.bindUav(0, 0, sbtBuffer);

			RtShadowsSbtBuildConstants consts = {};
			ANKI_ASSERT(m_sbtRecordSize % 4 == 0);
			consts.m_sbtRecordDwordSize = m_sbtRecordSize / 4;
			const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
			ANKI_ASSERT(shaderHandleSize % 4 == 0);
			consts.m_shaderHandleDwordSize = shaderHandleSize / 4;
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.dispatchComputeIndirect(sbtBuildIndirectArgsBuffer);
		});
	}

	// Ray gen
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows");

		rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kSrvDispatchRays);
		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::kUavDispatchRays);
		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kSrvDispatchRays);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSrvDispatchRays);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvDispatchRays);

		rpass.newTextureDependency(m_runCtx.m_prevMomentsRt, TextureUsageBit::kSrvDispatchRays);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kUavDispatchRays);

		rpass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvDispatchRays);

		rpass.setWork([this, sbtBuffer, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_rtLibraryGrProg.get());

			// Allocate, set and bind global uniforms
			{
				MaterialGlobalConstants* globalConstants;
				const BufferView globalConstantsToken = RebarTransientMemoryPool::getSingleton().allocateConstantBuffer(globalConstants);

				memset(globalConstants, 0, sizeof(*globalConstants)); // Don't care for now

				cmdb.bindConstantBuffer(ANKI_MATERIAL_REGISTER_GLOBAL_CONSTANTS, 0, globalConstantsToken);
			}

			// More globals
			cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

			cmdb.bindConstantBuffer(0, 2, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindUav(0, 2, m_runCtx.m_intermediateShadowsRts[0]);
			rgraphCtx.bindSrv(0, 2, m_runCtx.m_historyRt);
			cmdb.bindSampler(1, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(1, 2, getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
			rgraphCtx.bindSrv(2, 2, getRenderer().getMotionVectors().getMotionVectorsRt());
			cmdb.bindSrv(3, 2, TextureView(m_dummyHistoryLenTex.get(), TextureSubresourceDesc::all()));
			rgraphCtx.bindSrv(4, 2, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(5, 2, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());
			rgraphCtx.bindSrv(6, 2, m_runCtx.m_prevMomentsRt);
			rgraphCtx.bindUav(1, 2, m_runCtx.m_currentMomentsRt);
			cmdb.bindSrv(7, 2, TextureView(&m_blueNoiseImage->getTexture(), TextureSubresourceDesc::all()));

			cmdb.dispatchRays(sbtBuffer, m_sbtRecordSize, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
							  getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, 1);
		});
	}

	// Denoise pass horizontal
	if(!m_useSvgf)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows Denoise Horizontal");

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			runDenoise(ctx, rgraphCtx, true);
		});

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::kUavCompute);
	}

	// Denoise pass vertical
	if(!m_useSvgf)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows Denoise Vertical");
		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			runDenoise(ctx, rgraphCtx, false);
		});

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kUavCompute);
	}

	// Variance calculation pass
	if(m_useSvgf)
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows SVGF Variance");

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::kUavCompute);
		rpass.newTextureDependency(m_runCtx.m_varianceRts[1], TextureUsageBit::kUavCompute);

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_svgfVarianceGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, m_runCtx.m_intermediateShadowsRts[0]);
			rgraphCtx.bindSrv(1, 0, m_runCtx.m_currentMomentsRt);
			cmdb.bindSrv(2, 0, TextureView(m_dummyHistoryLenTex.get(), TextureSubresourceDesc::all()));
			rgraphCtx.bindSrv(3, 0, getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);

			rgraphCtx.bindUav(0, 0, m_runCtx.m_intermediateShadowsRts[1]);
			rgraphCtx.bindUav(1, 0, m_runCtx.m_varianceRts[1]);

			const Mat4& invProjMat = ctx.m_matrices.m_projectionJitter.invert();
			cmdb.setFastConstants(&invProjMat, sizeof(invProjMat));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
		});
	}

	// SVGF Atrous
	if(m_useSvgf)
	{
		for(U32 i = 0; i < m_atrousPassCount; ++i)
		{
			const Bool lastPass = i == U32(m_atrousPassCount - 1);
			const U32 readRtIdx = (i + 1) & 1;

			NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows SVGF Atrous");

			rpass.newTextureDependency(ANKI_DEPTH_DEP);
			rpass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
			rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[readRtIdx], TextureUsageBit::kSrvCompute);
			rpass.newTextureDependency(m_runCtx.m_varianceRts[readRtIdx], TextureUsageBit::kSrvCompute);

			if(!lastPass)
			{
				rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[!readRtIdx], TextureUsageBit::kUavCompute);

				rpass.newTextureDependency(m_runCtx.m_varianceRts[!readRtIdx], TextureUsageBit::kUavCompute);
			}
			else
			{
				rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kUavCompute);
			}

			rpass.setWork([this, &ctx, passIdx = i](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(RtShadows);
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				const Bool lastPass = passIdx == U32(m_atrousPassCount - 1);
				const U32 readRtIdx = (passIdx + 1) & 1;

				if(lastPass)
				{
					cmdb.bindShaderProgram(m_svgfAtrousLastPassGrProg.get());
				}
				else
				{
					cmdb.bindShaderProgram(m_svgfAtrousGrProg.get());
				}

				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

				rgraphCtx.bindSrv(0, 0, getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
				rgraphCtx.bindSrv(1, 0, m_runCtx.m_intermediateShadowsRts[readRtIdx]);
				rgraphCtx.bindSrv(2, 0, m_runCtx.m_varianceRts[readRtIdx]);

				if(!lastPass)
				{
					rgraphCtx.bindUav(0, 0, m_runCtx.m_intermediateShadowsRts[!readRtIdx]);
					rgraphCtx.bindUav(1, 0, m_runCtx.m_varianceRts[!readRtIdx]);
				}
				else
				{
					rgraphCtx.bindUav(0, 0, m_runCtx.m_historyRt);
				}

				const Mat4& invProjMat = ctx.m_matrices.m_projectionJitter.invert();
				cmdb.setFastConstants(&invProjMat, sizeof(invProjMat));

				dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
			});
		}
	}

	// Upscale
	{
		NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("RtShadows Upscale");

		rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);

		rpass.newTextureDependency(m_runCtx.m_upscaledRt, TextureUsageBit::kUavCompute);

		rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscaleGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(0, 0, m_runCtx.m_historyRt);
			rgraphCtx.bindUav(0, 0, m_runCtx.m_upscaledRt);
			rgraphCtx.bindSrv(1, 0, getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
			rgraphCtx.bindSrv(2, 0, getGBuffer().getDepthRt());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}
}

void RtShadows::runDenoise(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx, Bool horizontal)
{
	ANKI_TRACE_SCOPED_EVENT(RtShadows);
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	cmdb.bindShaderProgram((horizontal) ? m_grDenoiseHorizontalProg.get() : m_grDenoiseVerticalProg.get());

	cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
	rgraphCtx.bindSrv(0, 0, m_runCtx.m_intermediateShadowsRts[(horizontal) ? 0 : 1]);
	rgraphCtx.bindSrv(1, 0, getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
	rgraphCtx.bindSrv(2, 0, getGBuffer().getColorRt(2));
	rgraphCtx.bindSrv(3, 0, m_runCtx.m_currentMomentsRt);
	cmdb.bindSrv(4, 0, TextureView(m_dummyHistoryLenTex.get(), TextureSubresourceDesc::all()));

	rgraphCtx.bindUav(0, 0, (horizontal) ? m_runCtx.m_intermediateShadowsRts[1] : m_runCtx.m_historyRt);

	RtShadowsDenoiseConstants consts;
	consts.m_invViewProjMat = ctx.m_matrices.m_invertedViewProjectionJitter;
	consts.m_time = F32(GlobalFrameIndex::getSingleton().m_value % 0xFFFFu);
	consts.m_minSampleCount = 8;
	consts.m_maxSampleCount = 32;
	cmdb.setFastConstants(&consts, sizeof(consts));

	dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
}

void RtShadows::getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
									 [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
{
	handles[0] = m_runCtx.m_upscaledRt;
}

} // end namespace anki
