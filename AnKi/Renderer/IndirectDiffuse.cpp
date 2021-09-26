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
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Shaders/Include/SsgiTypes.h>

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
	m_main.m_firstStepPixels = 32;

	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/BlueNoise_Rgba8_16x16.png", m_main.m_noiseImage));

	// Init SSGI
	{
		m_main.m_rtDescr =
			m_r->create2DRenderTargetDescription(size.x(), size.y(), Format::R16G16B16A16_SFLOAT, "IndirectDiffuse");
		m_main.m_rtDescr.bake();

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
		m_runCtx.m_ssgiRtHandle = rgraph.newRenderTarget(m_main.m_rtDescr);

		// Create pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("IndirectDiffuse");

		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_firstMipmap = m_main.m_depthLod;
		rpass.newDependency(RenderPassDependency(m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE,
												 hizSubresource));
		rpass.newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE));
		rpass.newDependency(RenderPassDependency(m_runCtx.m_ssgiRtHandle, TextureUsageBit::IMAGE_COMPUTE_WRITE));

		rpass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->bindShaderProgram(m_main.m_grProg);

			rgraphCtx.bindImage(0, 0, m_runCtx.m_ssgiRtHandle, TextureSubresourceInfo());

			// Bind uniforms
			SsgiUniforms* unis = allocateAndBindUniforms<SsgiUniforms*>(sizeof(SsgiUniforms), cmdb, 0, 1);
			unis->m_depthBufferSize =
				UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()) >> (m_main.m_depthLod + 1);
			unis->m_framebufferSize = m_r->getInternalResolution() / 2u;
			unis->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
			unis->m_projMat = ctx.m_matrices.m_projectionJitter;
			unis->m_prevViewProjMatMulInvViewProjMat =
				ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
			unis->m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
			unis->m_frameCount = m_r->getFrameCount() & MAX_U32;
			unis->m_maxSteps = m_main.m_maxSteps;
			unis->m_firstStepPixels = m_main.m_firstStepPixels;

			// Bind the rest
			cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

			TextureSubresourceInfo hizSubresource;
			hizSubresource.m_firstMipmap = m_main.m_depthLod;
			rgraphCtx.bindTexture(0, 4, m_r->getDepthDownscale().getHiZRt(), hizSubresource);
			rgraphCtx.bindColorTexture(0, 5, m_r->getDownscaleBlur().getRt());

			// Dispatch
			dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x() / 2, m_r->getInternalResolution().y() / 2);
		});
	}
}

} // end namespace anki
