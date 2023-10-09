// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <AnKi/Core/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Core/GpuMemory/GpuVisibleTransientMemoryPool.h>

namespace anki {

static BoolCVar g_rtShadowsSvgfCVar(CVarSubsystem::kRenderer, "RtShadowsSvgf", false, "Enable or not RT shadows SVGF");
static NumericCVar<U8> g_rtShadowsSvgfAtrousPassCountCVar(CVarSubsystem::kRenderer, "RtShadowsSvgfAtrousPassCount", 3, 1, 20,
														  "Number of atrous passes of SVGF");
static NumericCVar<U32> g_rtShadowsRaysPerPixelCVar(CVarSubsystem::kRenderer, "RtShadowsRaysPerPixel", 1, 1, 8, "Number of shadow rays per pixel");

Error RtShadows::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize ray traced shadows");
	}

	return err;
}

Error RtShadows::initInternal()
{
	ANKI_R_LOGV("Initializing RT shadows");

	m_useSvgf = g_rtShadowsSvgfCVar.get();
	m_atrousPassCount = g_rtShadowsSvgfAtrousPassCountCVar.get();

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_blueNoiseImage));

	// Setup build SBT program
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSetupSbtBuild.ankiprogbin", m_setupBuildSbtProg, m_setupBuildSbtGrProg));

	// Build SBT program
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSbtBuild.ankiprogbin", m_buildSbtProg, m_buildSbtGrProg));

	// Ray gen program
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtShadowsRayGen.ankiprogbin", m_rayGenProg));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_rayGenProg);
		variantInitInfo.addMutation("RAYS_PER_PIXEL", g_rtShadowsRaysPerPixelCVar.get());

		const ShaderProgramResourceVariant* variant;
		m_rayGenProg->getOrCreateVariant(variantInitInfo, variant);
		m_rtLibraryGrProg.reset(&variant->getProgram());
		m_rayGenShaderGroupIdx = variant->getShaderGroupHandleIndex();
	}

	// Miss prog
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtShadowsMiss.ankiprogbin", m_missProg));
		const ShaderProgramResourceVariant* variant;
		m_missProg->getOrCreateVariant(variant);
		m_missShaderGroupIdx = variant->getShaderGroupHandleIndex();
	}

	// Denoise program
	if(!m_useSvgf)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtShadowsDenoise.ankiprogbin", m_denoiseProg));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_denoiseProg);
		variantInitInfo.addConstant("kMinSampleCount", 8u);
		variantInitInfo.addConstant("kMaxSampleCount", 32u);
		variantInitInfo.addMutation("BLUR_ORIENTATION", 0);

		const ShaderProgramResourceVariant* variant;
		m_denoiseProg->getOrCreateVariant(variantInitInfo, variant);
		m_grDenoiseHorizontalProg.reset(&variant->getProgram());

		variantInitInfo.addMutation("BLUR_ORIENTATION", 1);
		m_denoiseProg->getOrCreateVariant(variantInitInfo, variant);
		m_grDenoiseVerticalProg.reset(&variant->getProgram());
	}

	// SVGF variance program
	if(m_useSvgf)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsSvgfVariance.ankiprogbin", m_svgfVarianceProg, m_svgfVarianceGrProg));
	}

	// SVGF atrous program
	if(m_useSvgf)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/RtShadowsSvgfAtrous.ankiprogbin", m_svgfAtrousProg));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_svgfAtrousProg);
		variantInitInfo.addMutation("LAST_PASS", 0);

		const ShaderProgramResourceVariant* variant;
		m_svgfAtrousProg->getOrCreateVariant(variantInitInfo, variant);
		m_svgfAtrousGrProg.reset(&variant->getProgram());

		variantInitInfo.addMutation("LAST_PASS", 1);
		m_svgfAtrousProg->getOrCreateVariant(variantInitInfo, variant);
		m_svgfAtrousLastPassGrProg.reset(&variant->getProgram());
	}

	// Upscale program
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/RtShadowsUpscale.ankiprogbin", m_upscaleProg, m_upscaleGrProg));

	// Quarter rez shadow RT
	{
		TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
			getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, Format::kR8_Unorm,
			TextureUsageBit::kAllSampled | TextureUsageBit::kUavTraceRaysWrite | TextureUsageBit::kUavComputeWrite, "RtShadows History");
		m_historyRt = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
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
			TextureUsageBit::kAllSampled | TextureUsageBit::kUavTraceRaysWrite | TextureUsageBit::kUavComputeWrite, "RtShadows Moments #1");
		m_momentsRts[0] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);

		texinit.setName("RtShadows Moments #2");
		m_momentsRts[1] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
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

	// Misc
	m_sbtRecordSize = getAlignedRoundUp(GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment, m_sbtRecordSize);

	return Error::kNone;
}

void RtShadows::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RtShadows);

#define ANKI_DEPTH_DEP \
	getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSampledTraceRays | TextureUsageBit::kSampledCompute, \
		DepthDownscale::kQuarterInternalResolution

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import RTs
	{
		const U32 prevRtIdx = getRenderer().getFrameCount() & 1;

		if(!m_rtsImportedOnce) [[unlikely]]
		{
			m_runCtx.m_historyRt = rgraph.importRenderTarget(m_historyRt.get(), TextureUsageBit::kSampledFragment);

			m_runCtx.m_prevMomentsRt = rgraph.importRenderTarget(m_momentsRts[prevRtIdx].get(), TextureUsageBit::kSampledFragment);

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
	BufferOffsetRange sbtBuildIndirectArgsBuffer;
	{
		sbtBuildIndirectArgsBuffer = GpuVisibleTransientMemoryPool::getSingleton().allocate(sizeof(DispatchIndirectArgs));
		sbtBuildIndirectArgsHandle = rgraph.importBuffer(BufferUsageBit::kUavComputeWrite, sbtBuildIndirectArgsBuffer);

		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows setup build SBT");

		rpass.newBufferDependency(sbtBuildIndirectArgsHandle, BufferUsageBit::kAccelerationStructureBuild);

		rpass.setWork([this, sbtBuildIndirectArgsBuffer](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_setupBuildSbtGrProg.get());

			cmdb.bindUavBuffer(0, 0, GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 1, sbtBuildIndirectArgsBuffer);

			cmdb.dispatchCompute(1, 1, 1);
		});
	}

	// Build the SBT
	BufferHandle sbtHandle;
	BufferOffsetRange sbtBuffer;
	{
		// Allocate SBT
		U8* sbtMem;
		sbtBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame(
			(GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount() + 2) * m_sbtRecordSize, sbtMem);
		sbtHandle = rgraph.importBuffer(BufferUsageBit::kUavComputeWrite, sbtBuffer);

		// Write the first 2 entries of the SBT
		ConstWeakArray<U8> shaderGroupHandles = m_rtLibraryGrProg->getShaderGroupHandles();
		const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
		memcpy(sbtMem, &shaderGroupHandles[m_rayGenShaderGroupIdx * shaderHandleSize], shaderHandleSize);
		memcpy(sbtMem + m_sbtRecordSize, &shaderGroupHandles[m_missShaderGroupIdx * shaderHandleSize], shaderHandleSize);

		// Create the pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows build SBT");

		BufferHandle visibilityHandle;
		BufferOffsetRange visibleRenderableIndicesBuff;
		getRenderer().getAccelerationStructureBuilder().getVisibilityInfo(visibilityHandle, visibleRenderableIndicesBuff);

		rpass.newBufferDependency(visibilityHandle, BufferUsageBit::kUavComputeRead);
		rpass.newBufferDependency(sbtBuildIndirectArgsHandle, BufferUsageBit::kIndirectCompute);

		rpass.setWork([this, sbtBuildIndirectArgsBuffer, sbtBuffer, visibleRenderableIndicesBuff](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_buildSbtGrProg.get());

			cmdb.bindUavBuffer(0, 0, GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
			cmdb.bindUavBuffer(0, 1, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);
			cmdb.bindUavBuffer(0, 2, visibleRenderableIndicesBuff);
			cmdb.bindUavBuffer(0, 3, &m_rtLibraryGrProg->getShaderGroupHandlesGpuBuffer(), 0, kMaxPtrSize);
			cmdb.bindUavBuffer(0, 4, sbtBuffer);

			RtShadowsSbtBuildConstants unis = {};
			ANKI_ASSERT(m_sbtRecordSize % 4 == 0);
			unis.m_sbtRecordDwordSize = m_sbtRecordSize / 4;
			const U32 shaderHandleSize = GrManager::getSingleton().getDeviceCapabilities().m_shaderGroupHandleSize;
			ANKI_ASSERT(shaderHandleSize % 4 == 0);
			unis.m_shaderHandleDwordSize = shaderHandleSize / 4;
			cmdb.setPushConstants(&unis, sizeof(unis));

			cmdb.dispatchComputeIndirect(sbtBuildIndirectArgsBuffer.m_buffer, sbtBuildIndirectArgsBuffer.m_offset);
		});
	}

	// Ray gen
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows");

		rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kSampledTraceRays);
		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::kUavTraceRaysWrite);
		rpass.newAccelerationStructureDependency(getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle(),
												 AccelerationStructureUsageBit::kTraceRaysRead);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSampledTraceRays);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), TextureUsageBit::kSampledTraceRays);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSampledTraceRays);

		rpass.newTextureDependency(m_runCtx.m_prevMomentsRt, TextureUsageBit::kSampledTraceRays);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kUavTraceRaysWrite);

		rpass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kUavTraceRaysRead);

		rpass.setWork([this, sbtBuffer](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_rtLibraryGrProg.get());

			// Allocate, set and bind global uniforms
			{
				MaterialGlobalConstants* globalUniforms;
				const RebarAllocation globalUniformsToken = RebarTransientMemoryPool::getSingleton().allocateFrame(1, globalUniforms);

				memset(globalUniforms, 0, sizeof(*globalUniforms)); // Don't care for now

				cmdb.bindConstantBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGlobalConstants), globalUniformsToken);
			}

			// More globals
			cmdb.bindAllBindless(U32(MaterialSet::kBindless));
			cmdb.bindSampler(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTrilinearRepeatSampler),
							 getRenderer().getSamplers().m_trilinearRepeat.get());
			cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGpuScene), &GpuSceneBuffer::getSingleton().getBuffer(), 0,
							   kMaxPtrSize);

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) \
	cmdb.bindReadOnlyTextureBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kUnifiedGeometry_##fmt), \
								   &UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize, Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

			constexpr U32 kSet = 2;

			cmdb.bindConstantBuffer(kSet, 0, getRenderer().getClusterBinning().getClusteredShadingConstants());
			cmdb.bindUavBuffer(kSet, 1, getRenderer().getClusterBinning().getClustersBuffer());

			cmdb.bindSampler(kSet, 2, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindUavTexture(kSet, 3, m_runCtx.m_intermediateShadowsRts[0]);
			rgraphCtx.bindColorTexture(kSet, 4, m_runCtx.m_historyRt);
			cmdb.bindSampler(kSet, 5, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindTexture(kSet, 6, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
			rgraphCtx.bindColorTexture(kSet, 7, getRenderer().getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindColorTexture(kSet, 8, getRenderer().getMotionVectors().getHistoryLengthRt());
			rgraphCtx.bindColorTexture(kSet, 9, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindAccelerationStructure(kSet, 10, getRenderer().getAccelerationStructureBuilder().getAccelerationStructureHandle());
			rgraphCtx.bindColorTexture(kSet, 11, m_runCtx.m_prevMomentsRt);
			rgraphCtx.bindUavTexture(kSet, 12, m_runCtx.m_currentMomentsRt);
			cmdb.bindTexture(kSet, 13, &m_blueNoiseImage->getTextureView());

			cmdb.traceRays(sbtBuffer.m_buffer, sbtBuffer.m_offset, m_sbtRecordSize,
						   GpuSceneArrays::RenderableBoundingVolumeRt::getSingleton().getElementCount(), 1,
						   getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2, 1);
		});
	}

	// Denoise pass horizontal
	if(!m_useSvgf)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows Denoise Horizontal");

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			runDenoise(ctx, rgraphCtx, true);
		});

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), TextureUsageBit::kSampledCompute);

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::kUavComputeWrite);
	}

	// Denoise pass vertical
	if(!m_useSvgf)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows Denoise Vertical");
		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			runDenoise(ctx, rgraphCtx, false);
		});

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), TextureUsageBit::kSampledCompute);

		rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kUavComputeWrite);
	}

	// Variance calculation pass
	if(m_useSvgf)
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows SVGF Variance");

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[0], TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(m_runCtx.m_currentMomentsRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);
		rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSampledCompute);

		rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[1], TextureUsageBit::kUavComputeWrite);
		rpass.newTextureDependency(m_runCtx.m_varianceRts[1], TextureUsageBit::kUavComputeWrite);

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_svgfVarianceGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_intermediateShadowsRts[0]);
			rgraphCtx.bindColorTexture(0, 2, m_runCtx.m_currentMomentsRt);
			rgraphCtx.bindColorTexture(0, 3, getRenderer().getMotionVectors().getHistoryLengthRt());
			rgraphCtx.bindTexture(0, 4, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);

			rgraphCtx.bindUavTexture(0, 5, m_runCtx.m_intermediateShadowsRts[1]);
			rgraphCtx.bindUavTexture(0, 6, m_runCtx.m_varianceRts[1]);

			const Mat4& invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
			cmdb.setPushConstants(&invProjMat, sizeof(invProjMat));

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

			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows SVGF Atrous");

			rpass.newTextureDependency(ANKI_DEPTH_DEP);
			rpass.newTextureDependency(getRenderer().getGBuffer().getColorRt(2), TextureUsageBit::kSampledCompute);
			rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[readRtIdx], TextureUsageBit::kSampledCompute);
			rpass.newTextureDependency(m_runCtx.m_varianceRts[readRtIdx], TextureUsageBit::kSampledCompute);

			if(!lastPass)
			{
				rpass.newTextureDependency(m_runCtx.m_intermediateShadowsRts[!readRtIdx], TextureUsageBit::kUavComputeWrite);

				rpass.newTextureDependency(m_runCtx.m_varianceRts[!readRtIdx], TextureUsageBit::kUavComputeWrite);
			}
			else
			{
				rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kUavComputeWrite);
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
				cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());

				rgraphCtx.bindTexture(0, 2, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
				rgraphCtx.bindColorTexture(0, 3, m_runCtx.m_intermediateShadowsRts[readRtIdx]);
				rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_varianceRts[readRtIdx]);

				if(!lastPass)
				{
					rgraphCtx.bindUavTexture(0, 5, m_runCtx.m_intermediateShadowsRts[!readRtIdx]);
					rgraphCtx.bindUavTexture(0, 6, m_runCtx.m_varianceRts[!readRtIdx]);
				}
				else
				{
					rgraphCtx.bindUavTexture(0, 5, m_runCtx.m_historyRt);
				}

				const Mat4& invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
				cmdb.setPushConstants(&invProjMat, sizeof(invProjMat));

				dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
			});
		}
	}

	// Upscale
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("RtShadows Upscale");

		rpass.newTextureDependency(m_runCtx.m_historyRt, TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledCompute);
		rpass.newTextureDependency(ANKI_DEPTH_DEP);

		rpass.newTextureDependency(m_runCtx.m_upscaledRt, TextureUsageBit::kUavComputeWrite);

		rpass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(RtShadows);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscaleGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_historyRt);
			rgraphCtx.bindUavTexture(0, 2, m_runCtx.m_upscaledRt);
			rgraphCtx.bindTexture(0, 3, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
			rgraphCtx.bindTexture(0, 4, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

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
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_intermediateShadowsRts[(horizontal) ? 0 : 1]);
	rgraphCtx.bindTexture(0, 2, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
	rgraphCtx.bindColorTexture(0, 3, getRenderer().getGBuffer().getColorRt(2));
	rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_currentMomentsRt);
	rgraphCtx.bindColorTexture(0, 5, getRenderer().getMotionVectors().getHistoryLengthRt());

	rgraphCtx.bindUavTexture(0, 6, (horizontal) ? m_runCtx.m_intermediateShadowsRts[1] : m_runCtx.m_historyRt);

	RtShadowsDenoiseConstants unis;
	unis.m_invViewProjMat = ctx.m_matrices.m_invertedViewProjectionJitter;
	unis.m_time = F32(GlobalFrameIndex::getSingleton().m_value % 0xFFFFu);
	cmdb.setPushConstants(&unis, sizeof(unis));

	dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x() / 2, getRenderer().getInternalResolution().y() / 2);
}

void RtShadows::getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
									 [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
{
	handles[0] = m_runCtx.m_upscaledRt;
}

} // end namespace anki
