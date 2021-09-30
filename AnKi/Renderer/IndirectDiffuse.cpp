// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuse.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/GlobalIllumination.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Shaders/Include/IndirectDiffuseTypes.h>

namespace anki
{

IndirectDiffuse::~IndirectDiffuse()
{
}

Error IndirectDiffuse::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize indirect diffuse pass");
	}
	return err;
}

Error IndirectDiffuse::initInternal(const ConfigSet& cfg)
{
	const UVec2 size = m_r->getInternalResolution() / 2;
	ANKI_ASSERT((m_r->getInternalResolution() % 2) == UVec2(0u) && "Needs to be dividable for proper upscaling");

	// Init textures
	TextureInitInfo texInit = m_r->create2DRenderTargetInitInfo(
		size.x(), size.y(), Format::B10G11R11_UFLOAT_PACK32,
		TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::ALL_SAMPLED, "IndirectDiffuse #1");
	texInit.m_initialUsage = TextureUsageBit::ALL_SAMPLED;
	m_rts[0] = m_r->createAndClearRenderTarget(texInit);
	texInit.setName("IndirectDiffuse #2");
	m_rts[1] = m_r->createAndClearRenderTarget(texInit);

	texInit = m_r->create2DRenderTargetInitInfo(size.x(), size.y(), Format::R16G16B16A16_SFLOAT,
												TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::ALL_SAMPLED,
												"IndirectDiffuseMoments #1");
	texInit.m_initialUsage = TextureUsageBit::ALL_SAMPLED;
	m_momentsAndHistoryLengthRts[0] = m_r->createAndClearRenderTarget(texInit);
	texInit.setName("IndirectDiffuseMoments #2");
	m_momentsAndHistoryLengthRts[1] = m_r->createAndClearRenderTarget(texInit);

	// Init SSGI+probes pass
	{
		m_main.m_maxSteps = cfg.getNumberU32("r_indirectDiffuseSsgiMaxSteps");
		m_main.m_depthLod =
			min(cfg.getNumberU32("r_indirectDiffuseSsgiDepthLod"), m_r->getDepthDownscale().getMipmapCount() - 1);
		m_main.m_stepIncrement = cfg.getNumberU32("r_indirectDiffuseSsgiStepIncrement");

		ANKI_CHECK(getResourceManager().loadResource("Shaders/IndirectDiffuse.ankiprog", m_main.m_prog));

		const ShaderProgramResourceVariant* variant;
		m_main.m_prog->getOrCreateVariant(variant);
		m_main.m_grProg = variant->getProgram();
	}

	// Init denoise
	{
		m_denoise.m_minSampleCount = F32(cfg.getNumberU32("r_indirectDiffuseDenoiseMinSampleCount"));
		m_denoise.m_maxSampleCount =
			max(F32(cfg.getNumberU32("r_indirectDiffuseDenoiseMaxSampleCount")), m_denoise.m_minSampleCount);

		ANKI_CHECK(getResourceManager().loadResource("Shaders/IndirectDiffuseDenoise.ankiprog", m_denoise.m_prog));

		ShaderProgramResourceVariantInitInfo variantInit(m_denoise.m_prog);
		variantInit.addMutation("BLUR_ORIENTATION", 0);
		const ShaderProgramResourceVariant* variant;
		m_denoise.m_prog->getOrCreateVariant(variantInit, variant);
		m_denoise.m_grProgs[0] = variant->getProgram();

		variantInit.addMutation("BLUR_ORIENTATION", 1);
		m_denoise.m_prog->getOrCreateVariant(variantInit, variant);
		m_denoise.m_grProgs[1] = variant->getProgram();
	}

	return Error::NONE;
}

void IndirectDiffuse::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// SSGI+probes
	{
		// Create RTs
		const U32 readRtIdx = m_r->getFrameCount() & 1;
		const U32 writeRtIdx = !readRtIdx;
		if(ANKI_LIKELY(m_rtsImportedOnce))
		{
			m_runCtx.m_mainRtHandles[0] = rgraph.importRenderTarget(m_rts[readRtIdx]);
			m_runCtx.m_mainRtHandles[1] = rgraph.importRenderTarget(m_rts[writeRtIdx]);
			m_runCtx.m_momentsAndHistoryLengthHandles[0] =
				rgraph.importRenderTarget(m_momentsAndHistoryLengthRts[readRtIdx]);
			m_runCtx.m_momentsAndHistoryLengthHandles[1] =
				rgraph.importRenderTarget(m_momentsAndHistoryLengthRts[writeRtIdx]);
		}
		else
		{
			m_runCtx.m_mainRtHandles[0] = rgraph.importRenderTarget(m_rts[readRtIdx], TextureUsageBit::ALL_SAMPLED);
			m_runCtx.m_mainRtHandles[1] = rgraph.importRenderTarget(m_rts[writeRtIdx], TextureUsageBit::ALL_SAMPLED);
			m_runCtx.m_momentsAndHistoryLengthHandles[0] =
				rgraph.importRenderTarget(m_momentsAndHistoryLengthRts[readRtIdx], TextureUsageBit::ALL_SAMPLED);
			m_runCtx.m_momentsAndHistoryLengthHandles[1] =
				rgraph.importRenderTarget(m_momentsAndHistoryLengthRts[writeRtIdx], TextureUsageBit::ALL_SAMPLED);
			m_rtsImportedOnce = true;
		}

		// Create main pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("IndirectDiffuse");

		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_mainRtHandles[WRITE], TextureUsageBit::IMAGE_COMPUTE_WRITE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_momentsAndHistoryLengthHandles[WRITE],
												 TextureUsageBit::IMAGE_COMPUTE_WRITE));

		const TextureUsageBit readUsage = TextureUsageBit::SAMPLED_COMPUTE;
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), readUsage));
		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_firstMipmap = m_main.m_depthLod;
		rpass.newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), readUsage, hizSubresource));
		rpass.newDependency(RenderPassDependency(m_r->getDownscaleBlur().getRt(), readUsage));
		rpass.newDependency(RenderPassDependency(m_r->getMotionVectors().getMotionVectorsRt(), readUsage));
		rpass.newDependency(RenderPassDependency(m_r->getMotionVectors().getRejectionFactorRt(), readUsage));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_mainRtHandles[READ], readUsage));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_momentsAndHistoryLengthHandles[READ], readUsage));

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->bindShaderProgram(m_main.m_grProg);

			const ClusteredShadingContext& binning = ctx.m_clusteredShading;
			bindUniforms(cmdb, 0, 0, binning.m_clusteredShadingUniformsToken);
			m_r->getGlobalIllumination().bindVolumeTextures(ctx, rgraphCtx, 0, 1);
			bindUniforms(cmdb, 0, 2, binning.m_globalIlluminationProbesToken);
			bindStorage(cmdb, 0, 3, binning.m_clustersToken);

			rgraphCtx.bindImage(0, 4, m_runCtx.m_mainRtHandles[WRITE]);
			rgraphCtx.bindImage(0, 5, m_runCtx.m_momentsAndHistoryLengthHandles[WRITE]);

			cmdb->bindSampler(0, 6, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 7, m_r->getGBuffer().getColorRt(2));

			TextureSubresourceInfo hizSubresource;
			hizSubresource.m_firstMipmap = m_main.m_depthLod;
			rgraphCtx.bindTexture(0, 8, m_r->getDepthDownscale().getHiZRt(), hizSubresource);
			rgraphCtx.bindColorTexture(0, 9, m_r->getDownscaleBlur().getRt());
			rgraphCtx.bindColorTexture(0, 10, m_runCtx.m_mainRtHandles[READ]);
			rgraphCtx.bindColorTexture(0, 11, m_r->getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindColorTexture(0, 12, m_r->getMotionVectors().getRejectionFactorRt());
			rgraphCtx.bindColorTexture(0, 13, m_runCtx.m_momentsAndHistoryLengthHandles[READ]);

			// Bind uniforms
			IndirectDiffuseUniforms unis;
			unis.m_depthBufferSize = m_r->getInternalResolution() >> (m_main.m_depthLod + 1);
			unis.m_maxSteps = m_main.m_maxSteps;
			unis.m_stepIncrement = m_main.m_stepIncrement;
			unis.m_viewportSize = m_r->getInternalResolution() / 2u;
			unis.m_viewportSizef = Vec2(unis.m_viewportSize);
			cmdb->setPushConstants(&unis, sizeof(unis));

			// Dispatch
			dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x() / 2, m_r->getInternalResolution().y() / 2);
		});
	}

	// Denoise
	for(U32 dir = 0; dir < 2; ++dir)
	{
		ComputeRenderPassDescription& rpass =
			rgraph.newComputeRenderPass((dir == 0) ? "IndirectDiffuseDenoiseH" : "IndirectDiffuseDenoiseV");

		const TextureUsageBit readUsage = TextureUsageBit::SAMPLED_COMPUTE;
		const U32 readIdx = (dir == 0) ? WRITE : READ;

		rpass.newDependency(RenderPassDependency(m_runCtx.m_mainRtHandles[readIdx], readUsage));

		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_firstMipmap = 0;
		rpass.newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), readUsage, hizSubresource));

		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), readUsage));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_momentsAndHistoryLengthHandles[readIdx], readUsage));

		rpass.newDependency(
			RenderPassDependency(m_runCtx.m_mainRtHandles[!readIdx], TextureUsageBit::IMAGE_COMPUTE_WRITE));

		rpass.setWork([this, &ctx, dir, readIdx](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->bindShaderProgram(m_denoise.m_grProgs[dir]);

			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_mainRtHandles[readIdx]);
			TextureSubresourceInfo hizSubresource;
			hizSubresource.m_firstMipmap = 0;
			rgraphCtx.bindTexture(0, 2, m_r->getDepthDownscale().getHiZRt(), hizSubresource);
			rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));
			rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_momentsAndHistoryLengthHandles[readIdx]);
			rgraphCtx.bindImage(0, 5, m_runCtx.m_mainRtHandles[!readIdx]);

			IndirectDiffuseDenoiseUniforms unis;
			unis.m_invertedViewProjectionJitterMat = ctx.m_matrices.m_invertedViewProjectionJitter;
			unis.m_viewportSize = m_r->getInternalResolution() / 2u;
			unis.m_viewportSizef = Vec2(unis.m_viewportSize);
			unis.m_minSampleCount = m_denoise.m_minSampleCount;
			unis.m_maxSampleCount = m_denoise.m_maxSampleCount;

			cmdb->setPushConstants(&unis, sizeof(unis));

			// Dispatch
			dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x() / 2, m_r->getInternalResolution().y() / 2);
		});
	}
}

} // end namespace anki
