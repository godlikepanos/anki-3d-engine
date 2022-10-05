// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Core/ConfigSet.h>

namespace anki {

IndirectDiffuse::~IndirectDiffuse()
{
}

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
	const UVec2 size = m_r->getInternalResolution() / 2;
	ANKI_ASSERT((m_r->getInternalResolution() % 2) == UVec2(0u) && "Needs to be dividable for proper upscaling");

	ANKI_R_LOGV("Initializing indirect diffuse. Resolution %ux%u", size.x(), size.y());

	const Bool preferCompute = getConfig().getRPreferCompute();

	// Init textures
	TextureUsageBit usage = TextureUsageBit::kAllSampled;

	usage |= (preferCompute) ? TextureUsageBit::kImageComputeWrite : TextureUsageBit::kFramebufferWrite;
	TextureInitInfo texInit =
		m_r->create2DRenderTargetInitInfo(size.x(), size.y(), m_r->getHdrFormat(), usage, "IndirectDiffuse #1");
	m_rts[0] = m_r->createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);
	texInit.setName("IndirectDiffuse #2");
	m_rts[1] = m_r->createAndClearRenderTarget(texInit, TextureUsageBit::kAllSampled);

	if(!preferCompute)
	{
		m_main.m_fbDescr.m_colorAttachmentCount = 1;
		m_main.m_fbDescr.bake();
	}

	// Init VRS SRI generation
	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs() && !preferCompute;
	if(enableVrs)
	{
		m_vrs.m_sriTexelDimension = getGrManager().getDeviceCapabilities().m_minShadingRateImageTexelSize;
		ANKI_ASSERT(m_vrs.m_sriTexelDimension == 8 || m_vrs.m_sriTexelDimension == 16);

		const UVec2 rez = (size + m_vrs.m_sriTexelDimension - 1) / m_vrs.m_sriTexelDimension;
		m_vrs.m_rtHandle =
			m_r->create2DRenderTargetDescription(rez.x(), rez.y(), Format::kR8_Uint, "IndirectDiffuseVrsSri");
		m_vrs.m_rtHandle.bake();

		ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/IndirectDiffuseVrsSriGeneration.ankiprogbin",
													 m_vrs.m_prog));

		ShaderProgramResourceVariantInitInfo variantInit(m_vrs.m_prog);
		variantInit.addMutation("SRI_TEXEL_DIMENSION", m_vrs.m_sriTexelDimension);

		if(m_vrs.m_sriTexelDimension == 16 && getGrManager().getDeviceCapabilities().m_minSubgroupSize >= 32)
		{
			// Algorithm's workgroup size is 32, GPU's subgroup size is min 32 -> each workgroup has 1 subgroup -> No
			// need for shared mem
			variantInit.addMutation("SHARED_MEMORY", 0);
		}
		else if(m_vrs.m_sriTexelDimension == 8 && getGrManager().getDeviceCapabilities().m_minSubgroupSize >= 16)
		{
			// Algorithm's workgroup size is 16, GPU's subgroup size is min 16 -> each workgroup has 1 subgroup -> No
			// need for shared mem
			variantInit.addMutation("SHARED_MEMORY", 0);
		}
		else
		{
			variantInit.addMutation("SHARED_MEMORY", 1);
		}

		variantInit.addMutation("LIMIT_RATE_TO_2X2", getConfig().getRVrsLimitTo2x2());

		const ShaderProgramResourceVariant* variant;
		m_vrs.m_prog->getOrCreateVariant(variantInit, variant);
		m_vrs.m_grProg = variant->getProgram();

		ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/VrsSriVisualizeRenderTarget.ankiprogbin",
													 m_vrs.m_visualizeProg));
		m_vrs.m_visualizeProg->getOrCreateVariant(variant);
		m_vrs.m_visualizeGrProg = variant->getProgram();
	}

	// Init SSGI+probes pass
	{
		ANKI_CHECK(getResourceManager().loadResource((preferCompute)
														 ? "ShaderBinaries/IndirectDiffuseCompute.ankiprogbin"
														 : "ShaderBinaries/IndirectDiffuseRaster.ankiprogbin",
													 m_main.m_prog));

		const ShaderProgramResourceVariant* variant;
		m_main.m_prog->getOrCreateVariant(variant);
		m_main.m_grProg = variant->getProgram();
	}

	// Init denoise
	{
		m_denoise.m_fbDescr.m_colorAttachmentCount = 1;
		m_denoise.m_fbDescr.bake();

		ANKI_CHECK(getResourceManager().loadResource((preferCompute)
														 ? "ShaderBinaries/IndirectDiffuseDenoiseCompute.ankiprogbin"
														 : "ShaderBinaries/IndirectDiffuseDenoiseRaster.ankiprogbin",
													 m_denoise.m_prog));

		ShaderProgramResourceVariantInitInfo variantInit(m_denoise.m_prog);
		variantInit.addMutation("BLUR_ORIENTATION", 0);
		const ShaderProgramResourceVariant* variant;
		m_denoise.m_prog->getOrCreateVariant(variantInit, variant);
		m_denoise.m_grProgs[0] = variant->getProgram();

		variantInit.addMutation("BLUR_ORIENTATION", 1);
		m_denoise.m_prog->getOrCreateVariant(variantInit, variant);
		m_denoise.m_grProgs[1] = variant->getProgram();
	}

	return Error::kNone;
}

void IndirectDiffuse::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = getConfig().getRPreferCompute();
	const Bool enableVrs = getGrManager().getDeviceCapabilities().m_vrs && getConfig().getRVrs() && !preferCompute;
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

		pass.newTextureDependency(m_runCtx.m_sriRt, TextureUsageBit::kImageComputeWrite);
		pass.newTextureDependency(m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::kSampledCompute,
								  kHiZHalfSurface);

		pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			const UVec2 viewport = m_r->getInternalResolution() / 2u;

			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			cmdb->bindShaderProgram(m_vrs.m_grProg);

			rgraphCtx.bindTexture(0, 0, m_r->getDepthDownscale().getHiZRt(), kHiZHalfSurface);
			cmdb->bindSampler(0, 1, m_r->getSamplers().m_nearestNearestClamp);
			rgraphCtx.bindImage(0, 2, m_runCtx.m_sriRt);

			class
			{
			public:
				Vec4 m_v4;
				Mat4 m_invertedProjectionJitter;
			} pc;

			pc.m_v4 = Vec4(1.0f / Vec2(viewport), getConfig().getRIndirectDiffuseVrsDistanceThreshold(), 0.0f);
			pc.m_invertedProjectionJitter = ctx.m_matrices.m_invertedProjectionJitter;

			cmdb->setPushConstants(&pc, sizeof(pc));

			dispatchPPCompute(cmdb, m_vrs.m_sriTexelDimension, m_vrs.m_sriTexelDimension, viewport.x(), viewport.y());
		});
	}

	// SSGI+probes
	{
		// Create RTs
		const U32 readRtIdx = m_r->getFrameCount() & 1;
		const U32 writeRtIdx = !readRtIdx;
		if(ANKI_LIKELY(m_rtsImportedOnce))
		{
			m_runCtx.m_mainRtHandles[0] = rgraph.importRenderTarget(m_rts[readRtIdx]);
			m_runCtx.m_mainRtHandles[1] = rgraph.importRenderTarget(m_rts[writeRtIdx]);
		}
		else
		{
			m_runCtx.m_mainRtHandles[0] = rgraph.importRenderTarget(m_rts[readRtIdx], TextureUsageBit::kAllSampled);
			m_runCtx.m_mainRtHandles[1] = rgraph.importRenderTarget(m_rts[writeRtIdx], TextureUsageBit::kAllSampled);
			m_rtsImportedOnce = true;
		}

		// Create main pass
		TextureUsageBit readUsage;
		TextureUsageBit writeUsage;
		RenderPassDescriptionBase* prpass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("IndirectDiffuse");
			readUsage = TextureUsageBit::kSampledCompute;
			writeUsage = TextureUsageBit::kImageComputeWrite;
			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("IndirectDiffuse");
			rpass.setFramebufferInfo(m_main.m_fbDescr, {m_runCtx.m_mainRtHandles[kWrite]}, {},
									 (enableVrs) ? m_runCtx.m_sriRt : RenderTargetHandle());
			readUsage = TextureUsageBit::kSampledFragment;
			writeUsage = TextureUsageBit::kFramebufferWrite;
			prpass = &rpass;

			if(enableVrs)
			{
				prpass->newTextureDependency(m_runCtx.m_sriRt, TextureUsageBit::kFramebufferShadingRate);
			}
		}

		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[kWrite], writeUsage);

		m_r->getIndirectDiffuseProbes().setRenderGraphDependencies(ctx, *prpass, readUsage);
		prpass->newTextureDependency(m_r->getGBuffer().getColorRt(2), readUsage);
		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_mipmapCount = 1;
		prpass->newTextureDependency(m_r->getDepthDownscale().getHiZRt(), readUsage, hizSubresource);
		prpass->newTextureDependency(m_r->getDownscaleBlur().getRt(), readUsage);
		prpass->newTextureDependency(m_r->getMotionVectors().getMotionVectorsRt(), readUsage);
		prpass->newTextureDependency(m_r->getMotionVectors().getHistoryLengthRt(), readUsage);
		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[kRead], readUsage);

		prpass->setWork([this, &ctx, enableVrs](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->bindShaderProgram(m_main.m_grProg);

			const ClusteredShadingContext& binning = ctx.m_clusteredShading;
			bindUniforms(cmdb, 0, 0, binning.m_clusteredShadingUniformsToken);
			m_r->getIndirectDiffuseProbes().bindVolumeTextures(ctx, rgraphCtx, 0, 1);
			bindUniforms(cmdb, 0, 2, binning.m_globalIlluminationProbesToken);
			bindStorage(cmdb, 0, 3, binning.m_clustersToken);

			cmdb->bindSampler(0, 4, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 5, m_r->getGBuffer().getColorRt(2));

			TextureSubresourceInfo hizSubresource;
			hizSubresource.m_mipmapCount = 1;
			rgraphCtx.bindTexture(0, 6, m_r->getDepthDownscale().getHiZRt(), hizSubresource);
			rgraphCtx.bindColorTexture(0, 7, m_r->getDownscaleBlur().getRt());
			rgraphCtx.bindColorTexture(0, 8, m_runCtx.m_mainRtHandles[kRead]);
			rgraphCtx.bindColorTexture(0, 9, m_r->getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindColorTexture(0, 10, m_r->getMotionVectors().getHistoryLengthRt());

			if(getConfig().getRPreferCompute())
			{
				rgraphCtx.bindImage(0, 11, m_runCtx.m_mainRtHandles[kWrite]);
			}

			// Bind uniforms
			IndirectDiffuseUniforms unis;
			unis.m_viewportSize = m_r->getInternalResolution() / 2u;
			unis.m_viewportSizef = Vec2(unis.m_viewportSize);
			const Mat4& pmat = ctx.m_matrices.m_projection;
			unis.m_projectionMat = Vec4(pmat(0, 0), pmat(1, 1), pmat(2, 2), pmat(2, 3));
			unis.m_radius = getConfig().getRIndirectDiffuseSsgiRadius();
			unis.m_sampleCount = getConfig().getRIndirectDiffuseSsgiSampleCount();
			unis.m_sampleCountf = F32(unis.m_sampleCount);
			unis.m_ssaoBias = getConfig().getRIndirectDiffuseSsaoBias();
			unis.m_ssaoStrength = getConfig().getRIndirectDiffuseSsaoStrength();
			cmdb->setPushConstants(&unis, sizeof(unis));

			if(getConfig().getRPreferCompute())
			{
				dispatchPPCompute(cmdb, 8, 8, unis.m_viewportSize.x(), unis.m_viewportSize.y());
			}
			else
			{
				cmdb->setViewport(0, 0, unis.m_viewportSize.x(), unis.m_viewportSize.y());

				if(enableVrs)
				{
					cmdb->setVrsRate(VrsRate::k1x1);
				}

				cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
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
			ComputeRenderPassDescription& rpass =
				rgraph.newComputeRenderPass((dir == 0) ? "IndirectDiffuseDenoiseH" : "IndirectDiffuseDenoiseV");
			readUsage = TextureUsageBit::kSampledCompute;
			writeUsage = TextureUsageBit::kImageComputeWrite;
			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass =
				rgraph.newGraphicsRenderPass((dir == 0) ? "IndirectDiffuseDenoiseH" : "IndirectDiffuseDenoiseV");
			rpass.setFramebufferInfo(m_denoise.m_fbDescr, {m_runCtx.m_mainRtHandles[!readIdx]});
			readUsage = TextureUsageBit::kSampledFragment;
			writeUsage = TextureUsageBit::kFramebufferWrite;
			prpass = &rpass;
		}

		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[readIdx], readUsage);

		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_mipmapCount = 1;
		prpass->newTextureDependency(m_r->getDepthDownscale().getHiZRt(), readUsage, hizSubresource);
		prpass->newTextureDependency(m_runCtx.m_mainRtHandles[!readIdx], writeUsage);

		prpass->setWork([this, &ctx, dir, readIdx](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->bindShaderProgram(m_denoise.m_grProgs[dir]);

			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_mainRtHandles[readIdx]);
			TextureSubresourceInfo hizSubresource;
			hizSubresource.m_mipmapCount = 1;
			rgraphCtx.bindTexture(0, 2, m_r->getDepthDownscale().getHiZRt(), hizSubresource);

			if(getConfig().getRPreferCompute())
			{
				rgraphCtx.bindImage(0, 3, m_runCtx.m_mainRtHandles[!readIdx]);
			}

			IndirectDiffuseDenoiseUniforms unis;
			unis.m_invertedViewProjectionJitterMat = ctx.m_matrices.m_invertedViewProjectionJitter;
			unis.m_viewportSize = m_r->getInternalResolution() / 2u;
			unis.m_viewportSizef = Vec2(unis.m_viewportSize);
			unis.m_sampleCountDiv2 = F32(getConfig().getRIndirectDiffuseDenoiseSampleCount());
			unis.m_sampleCountDiv2 = max(1.0f, std::round(unis.m_sampleCountDiv2 / 2.0f));

			cmdb->setPushConstants(&unis, sizeof(unis));

			if(getConfig().getRPreferCompute())
			{
				dispatchPPCompute(cmdb, 8, 8, unis.m_viewportSize.x(), unis.m_viewportSize.y());
			}
			else
			{
				cmdb->setViewport(0, 0, unis.m_viewportSize.x(), unis.m_viewportSize.y());

				cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
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
