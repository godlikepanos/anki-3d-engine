// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static NumericCVar<U32> g_indirectDiffuseSsgiSampleCountCVar(CVarSubsystem::kRenderer, "IndirectDiffuseSsgiSampleCount", 8, 1, 1024,
															 "SSGI sample count");
static NumericCVar<F32> g_indirectDiffuseSsgiRadiusCVar(CVarSubsystem::kRenderer, "IndirectDiffuseSsgiRadius", 2.0f, 0.1f, 100.0f,
														"SSGI radius in meters");
static NumericCVar<U32> g_indirectDiffuseDenoiseSampleCountCVar(CVarSubsystem::kRenderer, "IndirectDiffuseDenoiseSampleCount", 4, 1, 128,
																"Indirect diffuse denoise sample count");
static NumericCVar<F32> g_indirectDiffuseSsaoStrengthCVar(CVarSubsystem::kRenderer, "IndirectDiffuseSsaoStrength", 2.5f, 0.1f, 10.0f,
														  "SSAO strength");
static NumericCVar<F32> g_indirectDiffuseSsaoBiasCVar(CVarSubsystem::kRenderer, "IndirectDiffuseSsaoBias", -0.1f, -10.0f, 10.0f, "SSAO bias");
static NumericCVar<F32> g_indirectDiffuseVrsDistanceThresholdCVar(CVarSubsystem::kRenderer, "IndirectDiffuseVrsDistanceThreshold", 0.01f, 0.00001f,
																  10.0f, "The meters that control the VRS SRI generation");

Error IndirectDiffuse::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize indirect diffuse pass");
	}
	return err;
}

Error IndirectDiffuse::initInternal()
{
	const UVec2 size = getRenderer().getInternalResolution() / 2;
	ANKI_ASSERT((getRenderer().getInternalResolution() % 2) == UVec2(0u) && "Needs to be dividable for proper upscaling");

	ANKI_R_LOGV("Initializing indirect diffuse. Resolution %ux%u", size.x(), size.y());

	const Bool preferCompute = g_preferComputeCVar.get();

	// Init textures
	TextureUsageBit usage = TextureUsageBit::kAllSampled;

	usage |= (preferCompute) ? TextureUsageBit::kUavComputeWrite : TextureUsageBit::kFramebufferWrite;
	TextureInitInfo texInit =
		getRenderer().create2DRenderTargetInitInfo(size.x(), size.y(), getRenderer().getHdrFormat(), usage, "IndirectDiffuse #1");
	m_rts[0] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);
	texInit.setName("IndirectDiffuse #2");
	m_rts[1] = getRenderer().createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);

	if(!preferCompute)
	{
		m_main.m_fbDescr.m_colorAttachmentCount = 1;
		m_main.m_fbDescr.bake();
	}

	// Init VRS SRI generation
	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get() && !preferCompute;
	if(enableVrs)
	{
		m_vrs.m_sriTexelDimension = GrManager::getSingleton().getDeviceCapabilities().m_minShadingRateImageTexelSize;
		ANKI_ASSERT(m_vrs.m_sriTexelDimension == 8 || m_vrs.m_sriTexelDimension == 16);

		const UVec2 rez = (size + m_vrs.m_sriTexelDimension - 1) / m_vrs.m_sriTexelDimension;
		m_vrs.m_rtHandle = getRenderer().create2DRenderTargetDescription(rez.x(), rez.y(), Format::kR8_Uint, "IndirectDiffuseVrsSri");
		m_vrs.m_rtHandle.bake();

		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/IndirectDiffuseVrsSriGeneration.ankiprogbin", m_vrs.m_prog));

		ShaderProgramResourceVariantInitInfo variantInit(m_vrs.m_prog);
		variantInit.addMutation("SRI_TEXEL_DIMENSION", m_vrs.m_sriTexelDimension);

		if(m_vrs.m_sriTexelDimension == 16 && GrManager::getSingleton().getDeviceCapabilities().m_minSubgroupSize >= 32)
		{
			// Algorithm's workgroup size is 32, GPU's subgroup size is min 32 -> each workgroup has 1 subgroup -> No
			// need for shared mem
			variantInit.addMutation("SHARED_MEMORY", 0);
		}
		else if(m_vrs.m_sriTexelDimension == 8 && GrManager::getSingleton().getDeviceCapabilities().m_minSubgroupSize >= 16)
		{
			// Algorithm's workgroup size is 16, GPU's subgroup size is min 16 -> each workgroup has 1 subgroup -> No
			// need for shared mem
			variantInit.addMutation("SHARED_MEMORY", 0);
		}
		else
		{
			variantInit.addMutation("SHARED_MEMORY", 1);
		}

		variantInit.addMutation("LIMIT_RATE_TO_2X2", g_vrsLimitTo2x2CVar.get());

		const ShaderProgramResourceVariant* variant;
		m_vrs.m_prog->getOrCreateVariant(variantInit, variant);
		m_vrs.m_grProg.reset(&variant->getProgram());

		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/VrsSriVisualizeRenderTarget.ankiprogbin", m_vrs.m_visualizeProg));
		m_vrs.m_visualizeProg->getOrCreateVariant(variant);
		m_vrs.m_visualizeGrProg.reset(&variant->getProgram());
	}

	// Init SSGI+probes pass
	{
		CString progFname =
			(preferCompute) ? "ShaderBinaries/IndirectDiffuseCompute.ankiprogbin" : "ShaderBinaries/IndirectDiffuseRaster.ankiprogbin";
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(progFname, m_main.m_prog));

		const ShaderProgramResourceVariant* variant;
		m_main.m_prog->getOrCreateVariant(variant);
		m_main.m_grProg.reset(&variant->getProgram());
	}

	// Init denoise
	{
		m_denoise.m_fbDescr.m_colorAttachmentCount = 1;
		m_denoise.m_fbDescr.bake();

		CString progFname =
			(preferCompute) ? "ShaderBinaries/IndirectDiffuseDenoiseCompute.ankiprogbin" : "ShaderBinaries/IndirectDiffuseDenoiseRaster.ankiprogbin";
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(progFname, m_denoise.m_prog));

		ShaderProgramResourceVariantInitInfo variantInit(m_denoise.m_prog);
		variantInit.addMutation("BLUR_ORIENTATION", 0);
		const ShaderProgramResourceVariant* variant;
		m_denoise.m_prog->getOrCreateVariant(variantInit, variant);
		m_denoise.m_grProgs[0].reset(&variant->getProgram());

		variantInit.addMutation("BLUR_ORIENTATION", 1);
		m_denoise.m_prog->getOrCreateVariant(variantInit, variant);
		m_denoise.m_grProgs[1].reset(&variant->getProgram());
	}

	return Error::kNone;
}

void IndirectDiffuse::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();
	const Bool enableVrs = GrManager::getSingleton().getDeviceCapabilities().m_vrs && g_vrsCVar.get() && !preferCompute;
	const Bool fbDescrHasVrs = m_main.m_fbDescr.m_shadingRateAttachmentTexelWidth > 0;

	if(!preferCompute && enableVrs != fbDescrHasVrs)
	{
		// Re-bake the FB descriptor if the VRS state has changed

		if(enableVrs)
		{
			m_main.m_fbDescr.m_shadingRateAttachmentTexelWidth = m_vrs.m_sriTexelDimension;
			m_main.m_fbDescr.m_shadingRateAttachmentTexelHeight = m_vrs.m_sriTexelDimension;
		}
		else
		{
			m_main.m_fbDescr.m_shadingRateAttachmentTexelWidth = 0;
			m_main.m_fbDescr.m_shadingRateAttachmentTexelHeight = 0;
		}

		m_main.m_fbDescr.bake();
	}

	// VRS SRI
	if(enableVrs)
	{
		m_runCtx.m_sriRt = rgraph.newRenderTarget(m_vrs.m_rtHandle);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("IndirectDiffuse VRS SRI gen");

		pass.newTextureDependency(m_runCtx.m_sriRt, TextureUsageBit::kUavComputeWrite);
		pass.newTextureDependency(getRenderer().getDepthDownscale().getRt(), TextureUsageBit::kSampledCompute,
								  DepthDownscale::kQuarterInternalResolution);

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			const UVec2 viewport = getRenderer().getInternalResolution() / 2u;

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_vrs.m_grProg.get());

			rgraphCtx.bindTexture(0, 0, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
			cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_nearestNearestClamp.get());
			rgraphCtx.bindUavTexture(0, 2, m_runCtx.m_sriRt);

			class
			{
			public:
				Vec4 m_v4;
				Mat4 m_invertedProjectionJitter;
			} pc;

			pc.m_v4 = Vec4(1.0f / Vec2(viewport), g_indirectDiffuseVrsDistanceThresholdCVar.get(), 0.0f);
			pc.m_invertedProjectionJitter = ctx.m_matrices.m_invertedProjectionJitter;

			cmdb.setPushConstants(&pc, sizeof(pc));

			dispatchPPCompute(cmdb, m_vrs.m_sriTexelDimension, m_vrs.m_sriTexelDimension, viewport.x(), viewport.y());
		});
	}

	// SSGI+probes
	{
		// Create RTs
		const U32 readRtIdx = getRenderer().getFrameCount() & 1;
		const U32 writeRtIdx = !readRtIdx;
		if(m_rtsImportedOnce) [[likely]]
		{
			m_runCtx.m_mainRtHandles[0] = rgraph.importRenderTarget(m_rts[readRtIdx].get());
			m_runCtx.m_mainRtHandles[1] = rgraph.importRenderTarget(m_rts[writeRtIdx].get());
		}
		else
		{
			m_runCtx.m_mainRtHandles[0] = rgraph.importRenderTarget(m_rts[readRtIdx].get(), TextureUsageBit::kAllSampled);
			m_runCtx.m_mainRtHandles[1] = rgraph.importRenderTarget(m_rts[writeRtIdx].get(), TextureUsageBit::kAllSampled);
			m_rtsImportedOnce = true;
		}

		// Create main pass
		TextureUsageBit readUsage;
		TextureUsageBit writeUsage;
		BufferUsageBit readBufferUsage;
		RenderPassDescriptionBase* prpass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("IndirectDiffuse");
			readUsage = TextureUsageBit::kSampledCompute;
			writeUsage = TextureUsageBit::kUavComputeWrite;
			readBufferUsage = BufferUsageBit::kUavComputeRead;
			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("IndirectDiffuse");
			rpass.setFramebufferInfo(m_main.m_fbDescr, {m_runCtx.m_mainRtHandles[kWrite]}, {}, (enableVrs) ? m_runCtx.m_sriRt : RenderTargetHandle());
			readUsage = TextureUsageBit::kSampledFragment;
			writeUsage = TextureUsageBit::kFramebufferWrite;
			readBufferUsage = BufferUsageBit::kUavFragmentRead;
			prpass = &rpass;

			if(enableVrs)
			{
				prpass->newTextureDependency(m_runCtx.m_sriRt, TextureUsageBit::kFramebufferShadingRate);
			}
		}

		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[kWrite], writeUsage);

		if(getRenderer().getIndirectDiffuseProbes().hasCurrentlyRefreshedVolumeRt())
		{
			prpass->newTextureDependency(getRenderer().getIndirectDiffuseProbes().getCurrentlyRefreshedVolumeRt(), readUsage);
		}

		prpass->newTextureDependency(getRenderer().getGBuffer().getColorRt(2), readUsage);
		prpass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage, DepthDownscale::kQuarterInternalResolution);
		prpass->newTextureDependency(getRenderer().getDownscaleBlur().getRt(), readUsage);
		prpass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);
		prpass->newTextureDependency(getRenderer().getMotionVectors().getHistoryLengthRt(), readUsage);
		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[kRead], readUsage);

		prpass->newBufferDependency(
			getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe),
			readBufferUsage);
		prpass->newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), readBufferUsage);

		prpass->setWork([this, &ctx, enableVrs](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			cmdb.bindShaderProgram(m_main.m_grProg.get());

			BufferOffsetRange buff = getRenderer().getClusterBinning().getClusteredShadingConstants();
			cmdb.bindConstantBuffer(0, 0, buff.m_buffer, buff.m_offset, buff.m_range);

			buff = getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe);
			cmdb.bindUavBuffer(0, 1, buff.m_buffer, buff.m_offset, buff.m_range);

			buff = getRenderer().getClusterBinning().getClustersBuffer();
			cmdb.bindUavBuffer(0, 2, buff.m_buffer, buff.m_offset, buff.m_range);

			cmdb.bindSampler(0, 3, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 4, getRenderer().getGBuffer().getColorRt(2));
			rgraphCtx.bindTexture(0, 5, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);
			rgraphCtx.bindColorTexture(0, 6, getRenderer().getDownscaleBlur().getRt());
			rgraphCtx.bindColorTexture(0, 7, m_runCtx.m_mainRtHandles[kRead]);
			rgraphCtx.bindColorTexture(0, 8, getRenderer().getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindColorTexture(0, 9, getRenderer().getMotionVectors().getHistoryLengthRt());

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUavTexture(0, 10, m_runCtx.m_mainRtHandles[kWrite]);
			}

			cmdb.bindAllBindless(1);

			// Bind uniforms
			IndirectDiffuseConstants unis;
			unis.m_viewportSize = getRenderer().getInternalResolution() / 2u;
			unis.m_viewportSizef = Vec2(unis.m_viewportSize);
			const Mat4& pmat = ctx.m_matrices.m_projection;
			unis.m_projectionMat = Vec4(pmat(0, 0), pmat(1, 1), pmat(2, 2), pmat(2, 3));
			unis.m_radius = g_indirectDiffuseSsgiRadiusCVar.get();
			unis.m_sampleCount = g_indirectDiffuseSsgiSampleCountCVar.get();
			unis.m_sampleCountf = F32(unis.m_sampleCount);
			unis.m_ssaoBias = g_indirectDiffuseSsaoBiasCVar.get();
			unis.m_ssaoStrength = g_indirectDiffuseSsaoStrengthCVar.get();
			cmdb.setPushConstants(&unis, sizeof(unis));

			if(g_preferComputeCVar.get())
			{
				dispatchPPCompute(cmdb, 8, 8, unis.m_viewportSize.x(), unis.m_viewportSize.y());
			}
			else
			{
				cmdb.setViewport(0, 0, unis.m_viewportSize.x(), unis.m_viewportSize.y());

				if(enableVrs)
				{
					cmdb.setVrsRate(VrsRate::k1x1);
				}

				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}

	// Denoise
	for(U32 dir = 0; dir < 2; ++dir)
	{
		const U32 readIdx = (dir == 0) ? kWrite : kRead;

		TextureUsageBit readUsage;
		TextureUsageBit writeUsage;
		RenderPassDescriptionBase* prpass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass((dir == 0) ? "IndirectDiffuseDenoiseH" : "IndirectDiffuseDenoiseV");
			readUsage = TextureUsageBit::kSampledCompute;
			writeUsage = TextureUsageBit::kUavComputeWrite;
			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass((dir == 0) ? "IndirectDiffuseDenoiseH" : "IndirectDiffuseDenoiseV");
			rpass.setFramebufferInfo(m_denoise.m_fbDescr, {m_runCtx.m_mainRtHandles[!readIdx]});
			readUsage = TextureUsageBit::kSampledFragment;
			writeUsage = TextureUsageBit::kFramebufferWrite;
			prpass = &rpass;
		}

		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[readIdx], readUsage);
		prpass->newTextureDependency(getRenderer().getDepthDownscale().getRt(), readUsage, DepthDownscale::kQuarterInternalResolution);
		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[!readIdx], writeUsage);

		prpass->setWork([this, &ctx, dir, readIdx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			cmdb.bindShaderProgram(m_denoise.m_grProgs[dir].get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_mainRtHandles[readIdx]);
			rgraphCtx.bindTexture(0, 2, getRenderer().getDepthDownscale().getRt(), DepthDownscale::kQuarterInternalResolution);

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUavTexture(0, 3, m_runCtx.m_mainRtHandles[!readIdx]);
			}

			IndirectDiffuseDenoiseConstants unis;
			unis.m_invertedViewProjectionJitterMat = ctx.m_matrices.m_invertedViewProjectionJitter;
			unis.m_viewportSize = getRenderer().getInternalResolution() / 2u;
			unis.m_viewportSizef = Vec2(unis.m_viewportSize);
			unis.m_sampleCountDiv2 = F32(g_indirectDiffuseDenoiseSampleCountCVar.get());
			unis.m_sampleCountDiv2 = max(1.0f, std::round(unis.m_sampleCountDiv2 / 2.0f));

			cmdb.setPushConstants(&unis, sizeof(unis));

			if(g_preferComputeCVar.get())
			{
				dispatchPPCompute(cmdb, 8, 8, unis.m_viewportSize.x(), unis.m_viewportSize.y());
			}
			else
			{
				cmdb.setViewport(0, 0, unis.m_viewportSize.x(), unis.m_viewportSize.y());

				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}
}

void IndirectDiffuse::getDebugRenderTarget(CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
										   ShaderProgramPtr& optionalShaderProgram) const
{
	if(rtName == "IndirectDiffuse")
	{
		handles[0] = m_runCtx.m_mainRtHandles[kWrite];
	}
	else
	{
		ANKI_ASSERT(rtName == "IndirectDiffuseVrsSri");
		handles[0] = m_runCtx.m_sriRt;
		optionalShaderProgram = m_vrs.m_visualizeGrProg;
	}
}

} // end namespace anki
