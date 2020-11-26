// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/Tonemapping.h>

namespace anki
{

TemporalAA::TemporalAA(Renderer* r)
	: RendererObject(r)
{
}

TemporalAA::~TemporalAA()
{
}

Error TemporalAA::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing TAA");
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init TAA");
	}

	return Error::NONE;
}

Error TemporalAA::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/TemporalAAResolve.ankiprog", m_prog));

	for(U32 i = 0; i < 2; ++i)
	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addConstant("VARIANCE_CLIPPING_GAMMA", 2.7f);
		variantInitInfo.addConstant("BLEND_FACTOR", 1.0f / 16.0f);
		variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));
		variantInitInfo.addMutation("SHARPEN", i + 1);
		variantInitInfo.addMutation("VARIANCE_CLIPPING", 1);
		variantInitInfo.addMutation("TONEMAP_FIX", 1);
		variantInitInfo.addMutation("YCBCR", 0);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProgs[i] = variant->getProgram();

		m_workgroupSize[0] = variant->getWorkgroupSizes()[0];
		m_workgroupSize[1] = variant->getWorkgroupSizes()[1];
	}

	for(U i = 0; i < 2; ++i)
	{
		TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(
			m_r->getWidth(), m_r->getHeight(), LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE | TextureUsageBit::IMAGE_COMPUTE_WRITE,
			"TemporalAA");

		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		m_rtTextures[i] = m_r->createAndClearRenderTarget(texinit);
	}

	return Error::NONE;
}

void TemporalAA::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProgs[m_r->getFrameCount() & 1]);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 2, m_r->getLightShading().getRt());
	rgraphCtx.bindColorTexture(0, 3, m_runCtx.m_historyRt);
	rgraphCtx.bindColorTexture(0, 4, m_r->getGBuffer().getColorRt(3));
	rgraphCtx.bindImage(0, 5, m_runCtx.m_renderRt, TextureSubresourceInfo());
	rgraphCtx.bindUniformBuffer(0, 6, m_r->getTonemapping().getAverageLuminanceBuffer());

	const Mat4 mat = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection
					 * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	cmdb->setPushConstants(&mat, sizeof(mat));

	dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_r->getWidth(), m_r->getHeight());
}

void TemporalAA::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import RTs
	m_runCtx.m_historyRt =
		rgraph.importRenderTarget(m_rtTextures[(m_r->getFrameCount() + 1) & 1], TextureUsageBit::SAMPLED_FRAGMENT);
	m_runCtx.m_renderRt = rgraph.importRenderTarget(m_rtTextures[m_r->getFrameCount() & 1], TextureUsageBit::NONE);

	// Create pass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("TemporalAA");

	pass.setWork(
		[](RenderPassWorkContext& rgraphCtx) {
			TemporalAA* const self = static_cast<TemporalAA*>(rgraphCtx.m_userData);
			self->run(*self->m_runCtx.m_ctx, rgraphCtx);
		},
		this, 0);

	pass.newDependency({m_runCtx.m_renderRt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	pass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE,
						TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
	pass.newDependency({m_r->getLightShading().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
	pass.newDependency({m_runCtx.m_historyRt, TextureUsageBit::SAMPLED_COMPUTE});
	pass.newDependency({m_r->getGBuffer().getColorRt(3), TextureUsageBit::SAMPLED_COMPUTE});
}

} // end namespace anki
