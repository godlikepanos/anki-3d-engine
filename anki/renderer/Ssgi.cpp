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
#include <anki/shaders/include/SsgiTypes.h>

namespace anki
{

static constexpr U32 WRITE = 0;
static constexpr U32 READ = 1;

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
	ANKI_ASSERT((width % 2) == 0 && (height % 2) == 0 && "The algorithms won't work");
	ANKI_R_LOGI("Initializing SSGI pass");
	m_main.m_maxSteps = cfg.getNumberU32("r_ssgiMaxSteps");
	m_main.m_depthLod = min(cfg.getNumberU32("r_ssgiDepthLod"), m_r->getDepthDownscale().getMipmapCount() - 1);
	m_main.m_firstStepPixels = 32;

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseRgb816x16.png", m_main.m_noiseTex));

	// Init main
	{
		m_main.m_rtDescr =
			m_r->create2DRenderTargetDescription(width / 2, height / 2, Format::B10G11R11_UFLOAT_PACK32, "SSGI_tmp");
		m_main.m_rtDescr.bake();

		ANKI_CHECK(getResourceManager().loadResource("shaders/Ssgi.ankiprog", m_main.m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_main.m_prog);

		for(U32 i = 0; i < 4; ++i)
		{
			variantInitInfo.addMutation("VARIANT", i);

			const ShaderProgramResourceVariant* variant;
			m_main.m_prog->getOrCreateVariant(variantInitInfo, variant);
			m_main.m_grProg[i] = variant->getProgram();
		}
	}

	// Init denoise
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/SsgiDenoise.ankiprog", m_denoise.m_prog));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_denoise.m_prog);
		const ShaderProgramResourceVariant* variant;

		variantInitInfo.addConstant("IN_TEXTURE_SIZE", UVec2(width / 2, height / 2));

		for(U32 i = 0; i < 4; ++i)
		{
			variantInitInfo.addMutation("VARIANT", i);

			variantInitInfo.addMutation("SAMPLE_COUNT", 11);
			variantInitInfo.addMutation("ORIENTATION", 0);
			m_denoise.m_prog->getOrCreateVariant(variantInitInfo, variant);
			m_denoise.m_grProg[0][i] = variant->getProgram();

			variantInitInfo.addMutation("SAMPLE_COUNT", 15);
			variantInitInfo.addMutation("ORIENTATION", 1);
			m_denoise.m_prog->getOrCreateVariant(variantInitInfo, variant);
			m_denoise.m_grProg[1][i] = variant->getProgram();
		}
	}

	// Init reconstruction
	{
		ANKI_CHECK(getResourceManager().loadResource("shaders/SsgiReconstruct.ankiprog", m_recontruction.m_prog));
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_recontruction.m_prog);
		variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));
		const ShaderProgramResourceVariant* variant;

		for(U32 i = 0; i < 4; ++i)
		{
			variantInitInfo.addMutation("VARIANT", i);
			m_recontruction.m_prog->getOrCreateVariant(variantInitInfo, variant);
			m_recontruction.m_grProg[i] = variant->getProgram();
		}

		TextureInitInfo initInfo = m_r->create2DRenderTargetInitInfo(
			width, height, Format::B10G11R11_UFLOAT_PACK32,
			TextureUsageBit::ALL_SAMPLED | TextureUsageBit::IMAGE_COMPUTE_WRITE, "SSGI");
		initInfo.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_recontruction.m_rt = m_r->createAndClearRenderTarget(initInfo);
	}

	return Error::NONE;
}

void Ssgi::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Main pass
	{
		// Create RTs
		if(ANKI_LIKELY(m_recontruction.m_rtImportedOnce))
		{
			m_runCtx.m_finalRt = rgraph.importRenderTarget(m_recontruction.m_rt);
		}
		else
		{
			m_runCtx.m_finalRt = rgraph.importRenderTarget(m_recontruction.m_rt, TextureUsageBit::SAMPLED_FRAGMENT);
			m_recontruction.m_rtImportedOnce = true;
		}
		m_runCtx.m_intermediateRts[WRITE] = rgraph.newRenderTarget(m_main.m_rtDescr);
		m_runCtx.m_intermediateRts[READ] = rgraph.newRenderTarget(m_main.m_rtDescr);

		// Create pass
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGI");
		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->run(rgraphCtx); }, this,
			0);

		rpass.newDependency({m_runCtx.m_intermediateRts[WRITE], TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_runCtx.m_finalRt, TextureUsageBit::SAMPLED_COMPUTE});

		TextureSubresourceInfo hizSubresource;
		hizSubresource.m_firstMipmap = m_main.m_depthLod;
		rpass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, hizSubresource});
		rpass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
	}

	// Blur vertical
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGI_blur_v");

		rpass.newDependency({m_runCtx.m_intermediateRts[WRITE], TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_runCtx.m_intermediateRts[READ], TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});

		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->runVBlur(rgraphCtx); },
			this, 0);
	}

	// Blur horizontal
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGI_blur_h");

		rpass.newDependency({m_runCtx.m_intermediateRts[READ], TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_runCtx.m_intermediateRts[WRITE], TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});

		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->runHBlur(rgraphCtx); },
			this, 0);
	}

	// Reconstruction
	{
		ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGI_recon");

		rpass.newDependency({m_runCtx.m_intermediateRts[WRITE], TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_runCtx.m_finalRt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
		rpass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});
		rpass.newDependency({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});

		rpass.setWork(
			[](RenderPassWorkContext& rgraphCtx) {
				static_cast<Ssgi*>(rgraphCtx.m_userData)->runRecontruct(rgraphCtx);
			},
			this, 0);
	}
}

void Ssgi::run(RenderPassWorkContext& rgraphCtx)
{
	RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_main.m_grProg[m_r->getFrameCount() % 4]);

	rgraphCtx.bindImage(0, 0, m_runCtx.m_intermediateRts[WRITE], TextureSubresourceInfo());

	// Bind uniforms
	SsgiUniforms* unis = allocateAndBindUniforms<SsgiUniforms*>(sizeof(SsgiUniforms), cmdb, 0, 1);
	unis->m_depthBufferSize = UVec2(m_r->getWidth(), m_r->getHeight()) >> (m_main.m_depthLod + 1);
	unis->m_framebufferSize = UVec2(m_r->getWidth(), m_r->getHeight());
	unis->m_invProjMat = ctx.m_matrices.m_projectionJitter.getInverse();
	unis->m_projMat = ctx.m_matrices.m_projectionJitter;
	unis->m_prevViewProjMatMulInvViewProjMat =
		ctx.m_prevMatrices.m_viewProjection * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	unis->m_normalMat = Mat3x4(Vec3(0.0f), ctx.m_matrices.m_view.getRotationPart());
	unis->m_frameCount = m_r->getFrameCount() & MAX_U32;
	unis->m_maxSteps = m_main.m_maxSteps;
	unis->m_firstStepPixels = m_main.m_firstStepPixels;

	cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	TextureSubresourceInfo hizSubresource;
	hizSubresource.m_firstMipmap = m_main.m_depthLod;
	rgraphCtx.bindTexture(0, 4, m_r->getDepthDownscale().getHiZRt(), hizSubresource);

	rgraphCtx.bindColorTexture(0, 5, m_r->getDownscaleBlur().getRt());
	rgraphCtx.bindColorTexture(0, 6, m_runCtx.m_finalRt);

	// Dispatch
	dispatchPPCompute(cmdb, 16, 16, m_r->getWidth() / 2, m_r->getHeight() / 2);
}

void Ssgi::runVBlur(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_denoise.m_grProg[0][m_r->getFrameCount() % 4]);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_intermediateRts[WRITE]);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	rgraphCtx.bindImage(0, 4, m_runCtx.m_intermediateRts[READ], TextureSubresourceInfo());

	const Mat4 mat = m_runCtx.m_ctx->m_matrices.m_viewProjectionJitter.getInverse();
	cmdb->setPushConstants(&mat, sizeof(mat));

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth() / 2, m_r->getHeight() / 2);
}

void Ssgi::runHBlur(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_denoise.m_grProg[1][m_r->getFrameCount() % 4]);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_intermediateRts[READ]);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(2));

	rgraphCtx.bindImage(0, 4, m_runCtx.m_intermediateRts[WRITE], TextureSubresourceInfo());

	const Mat4 mat = m_runCtx.m_ctx->m_matrices.m_viewProjectionJitter.getInverse();
	cmdb->setPushConstants(&mat, sizeof(mat));

	dispatchPPCompute(cmdb, 8, 8, m_r->getWidth() / 2, m_r->getHeight() / 2);
}

void Ssgi::runRecontruct(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_recontruction.m_grProg[m_r->getFrameCount() % 4]);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_intermediateRts[WRITE]);
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	rgraphCtx.bindImage(0, 3, m_runCtx.m_finalRt, TextureSubresourceInfo());

	dispatchPPCompute(cmdb, 16, 16, m_r->getWidth(), m_r->getHeight());
}

} // end namespace anki
