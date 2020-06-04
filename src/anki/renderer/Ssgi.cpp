// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssgi.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/core/ConfigSet.h>
#include <shaders/glsl_cpp_common/Ssgi.h>

namespace anki
{

Ssgi::~Ssgi()
{
}

Error Ssgi::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize SSGI pass");
	}
	return err;
}

Error Ssgi::initInternal(const ConfigSet& cfg)
{
	const U32 width = m_r->getWidth();
	const U32 height = m_r->getHeight();
	ANKI_R_LOGI("Initializing SSGI pass (%ux%u)", width, height);
	m_main.m_maxSteps = cfg.getNumberU32("r_ssgiMaxSteps");
	m_main.m_depthLod = min(cfg.getNumberU32("r_ssgiDepthLod"), m_r->getDepthDownscale().getMipmapCount() - 1);
	m_main.m_firstStepPixels = 32;

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseRgb816x16.png", m_main.m_noiseTex));

	// Create RTs
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(width,
		height,
		Format::R16G16B16A16_SFLOAT,
		TextureUsageBit::IMAGE_COMPUTE_WRITE | TextureUsageBit::SAMPLED_ALL,
		"SSGI");
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_main.m_rts[0] = m_r->createAndClearRenderTarget(texinit);
	m_main.m_rts[1] = m_r->createAndClearRenderTarget(texinit);

	// Create main shaders
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/Ssgi.ankiprog", m_main.m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_main.m_prog);
		variantInitInfo.addMutation("VARIANT", 0);

		const ShaderProgramResourceVariant* variant;
		m_main.m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_main.m_grProg[0] = variant->getProgram();

		variantInitInfo.addMutation("VARIANT", 1);
		m_main.m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_main.m_grProg[1] = variant->getProgram();
	}

	// Init denoise
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/DepthAwareBlurCompute.ankiprog", m_denoise.m_prog));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_denoise.m_prog);
		const ShaderProgramResourceVariant* variant;

		variantInitInfo.addConstant("TEXTURE_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));

		variantInitInfo.addMutation("SAMPLE_COUNT", 15);
		variantInitInfo.addMutation("COLOR_COMPONENTS", 4);
		variantInitInfo.addMutation("ORIENTATION", 0);
		m_denoise.m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_denoise.m_grProg[0] = variant->getProgram();

		variantInitInfo.addMutation("ORIENTATION", 1);
		m_denoise.m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_denoise.m_grProg[1] = variant->getProgram();
	}

	return Error::NONE;
}

void Ssgi::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;
	m_main.m_writeRtIdx = (m_main.m_writeRtIdx + 1) % 2;

	// Main pass
	{
		// Create RTs
		if(ANKI_LIKELY(m_main.m_rtImportedOnce))
		{
			m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_main.m_rts[0]);
			m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_main.m_rts[1]);
		}
		else
		{
			m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_main.m_rts[0], TextureUsageBit::SAMPLED_FRAGMENT);
			m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_main.m_rts[1], TextureUsageBit::SAMPLED_FRAGMENT);
			m_main.m_rtImportedOnce = true;
		}

		// Create pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGI");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->run(rgraphCtx); },
			this,
			0);

		rpass.newDependency({m_runCtx.m_rts[m_main.m_writeRtIdx], TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_runCtx.m_rts[!m_main.m_writeRtIdx], TextureUsageBit::SAMPLED_COMPUTE});

		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_firstMipmap = m_main.m_depthLod;
		rpass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, hizSubresource});
		rpass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
	}

	// Blur vertical
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGIBlurV");

		rpass.newDependency({m_runCtx.m_rts[m_main.m_writeRtIdx], TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_runCtx.m_rts[!m_main.m_writeRtIdx], TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});

		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->runVBlur(rgraphCtx); },
			this,
			0);
	}

	// Blur horizontal
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGIBlurH");

		rpass.newDependency({m_runCtx.m_rts[!m_main.m_writeRtIdx], TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_runCtx.m_rts[m_main.m_writeRtIdx], TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});

		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->runHBlur(rgraphCtx); },
			this,
			0);
	}
}

void Ssgi::run(RenderPassWorkContext& rgraphCtx)
{
	RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_main.m_grProg[m_r->getFrameCount() & 1u]);

	rgraphCtx.bindImage(0, 0, m_runCtx.m_rts[m_main.m_writeRtIdx], TextureSubresourceInfo());

	// Bind uniforms
	SsgiUniforms* unis = allocateAndBindUniforms<SsgiUniforms*>(sizeof(SsgiUniforms), cmdb, 0, 1);
	unis->m_depthBufferSize = UVec2(m_r->getWidth(), m_r->getHeight()) >> (m_main.m_depthLod + 1);
	unis->m_framebufferSize = UVec2(m_r->getWidth(), m_r->getHeight());
	unis->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
	unis->m_projMat = ctx.m_matrices.m_projectionJitter;
	unis->m_prevViewProjMatMulInvViewProjMat =
		ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	unis->m_normalMat = Mat3x4(ctx.m_matrices.m_view.getRotationPart());
	unis->m_frameCount = m_r->getFrameCount() & MAX_U32;
	unis->m_maxSteps = m_main.m_maxSteps;
	unis->m_firstStepPixels = m_main.m_firstStepPixels;

	cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	TextureSubresourceInfo hizSubresource;
	hizSubresource.m_firstMipmap = m_main.m_depthLod;
	rgraphCtx.bindTexture(0, 4, m_r->getDepthDownscale().getHiZRt(), hizSubresource);

	rgraphCtx.bindColorTexture(0, 5, m_r->getDownscaleBlur().getRt());
	rgraphCtx.bindColorTexture(0, 6, m_runCtx.m_rts[!m_main.m_writeRtIdx]);

	// Dispatch
	dispatchPPCompute(cmdb, 16, 16, m_r->getWidth() / 2, m_r->getHeight());
}

void Ssgi::runVBlur(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_denoise.m_grProg[0]);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_rts[m_main.m_writeRtIdx]);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	rgraphCtx.bindImage(0, 3, m_runCtx.m_rts[!m_main.m_writeRtIdx], TextureSubresourceInfo());

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth(), m_r->getHeight());
}

void Ssgi::runHBlur(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_denoise.m_grProg[1]);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_rts[!m_main.m_writeRtIdx]);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	rgraphCtx.bindImage(0, 3, m_runCtx.m_rts[m_main.m_writeRtIdx], TextureSubresourceInfo());

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth(), m_r->getHeight());
}

} // end namespace anki
