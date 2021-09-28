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

	m_main.m_maxSteps = cfg.getNumberU32("r_ssgiMaxSteps");
	m_main.m_depthLod = min(cfg.getNumberU32("r_ssgiDepthLod"), m_r->getDepthDownscale().getMipmapCount() - 1);
	m_main.m_stepIncrement = cfg.getNumberU32("r_ssgiStepIncrement");

	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/BlueNoise_Rgba8_16x16.png", m_main.m_noiseImage));

	// Init SSGI+probes pass
	{
		TextureInitInfo texInit = m_r->create2DRenderTargetInitInfo(
			size.x(), size.y(), Format::B10G11R11_UFLOAT_PACK32,
			TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::ALL_SAMPLED, "IndirectDiffuse #1");
		texInit.m_initialUsage = TextureUsageBit::ALL_SAMPLED;

		m_rts[0] = m_r->getGrManager().newTexture(texInit);
		texInit.setName("IndirectDiffuse #2");
		m_rts[1] = m_r->getGrManager().newTexture(texInit);

		ANKI_CHECK(getResourceManager().loadResource("Shaders/IndirectDiffuse.ankiprog", m_main.m_prog));

		const ShaderProgramResourceVariant* variant;
		m_main.m_prog->getOrCreateVariant(variant);
		m_main.m_grProg = variant->getProgram();
	}

	return Error::NONE;
}

void IndirectDiffuse::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// SSGI+probes
	{
		// Create RTs
		if(m_rtsImportedOnce)
		{
			m_runCtx.m_finalRtHandle = rgraph.importRenderTarget(m_rts[m_r->getFrameCount() & 1]);
			m_runCtx.m_historyRtHandle = rgraph.importRenderTarget(m_rts[!(m_r->getFrameCount() & 1)]);
		}
		else
		{
			m_runCtx.m_finalRtHandle =
				rgraph.importRenderTarget(m_rts[m_r->getFrameCount() & 1], TextureUsageBit::ALL_SAMPLED);
			m_runCtx.m_historyRtHandle =
				rgraph.importRenderTarget(m_rts[!(m_r->getFrameCount() & 1)], TextureUsageBit::ALL_SAMPLED);

			m_rtsImportedOnce = true;
		}

		// Create pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("IndirectDiffuse");

		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_firstMipmap = m_main.m_depthLod;
		rpass.newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE,
												 hizSubresource));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_finalRtHandle, TextureUsageBit::IMAGE_COMPUTE_WRITE));
		rpass.newDependency(
			RenderPassDependency(m_r->getMotionVectors().getMotionVectorsRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(
			RenderPassDependency(m_r->getMotionVectors().getRejectionFactorRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_historyRtHandle, TextureUsageBit::SAMPLED_COMPUTE));

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->bindShaderProgram(m_main.m_grProg);

			const ClusteredShadingContext& binning = ctx.m_clusteredShading;
			bindUniforms(cmdb, 0, 0, binning.m_clusteredShadingUniformsToken);
			m_r->getGlobalIllumination().bindVolumeTextures(ctx, rgraphCtx, 0, 1);
			bindUniforms(cmdb, 0, 2, binning.m_globalIlluminationProbesToken);
			bindStorage(cmdb, 0, 3, binning.m_clustersToken);

			rgraphCtx.bindImage(0, 4, m_runCtx.m_finalRtHandle, TextureSubresourceInfo());

			cmdb->bindSampler(0, 5, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 6, m_r->getGBuffer().getColorRt(2));

			TextureSubresourceInfo hizSubresource;
			hizSubresource.m_firstMipmap = m_main.m_depthLod;
			rgraphCtx.bindTexture(0, 7, m_r->getDepthDownscale().getHiZRt(), hizSubresource);
			rgraphCtx.bindColorTexture(0, 8, m_r->getDownscaleBlur().getRt());
			rgraphCtx.bindColorTexture(0, 9, m_runCtx.m_historyRtHandle);
			rgraphCtx.bindColorTexture(0, 10, m_r->getMotionVectors().getMotionVectorsRt());
			rgraphCtx.bindColorTexture(0, 11, m_r->getMotionVectors().getRejectionFactorRt());

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
}

} // end namespace anki
