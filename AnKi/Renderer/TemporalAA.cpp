// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

TemporalAA::TemporalAA(Renderer* r)
	: RendererObject(r)
{
}

TemporalAA::~TemporalAA()
{
}

Error TemporalAA::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to init TAA");
	}

	return Error::NONE;
}

Error TemporalAA::initInternal()
{
	ANKI_R_LOGV("Initializing TAA");

	ANKI_CHECK(m_r->getResourceManager().loadResource(
		(getConfig().getRPreferCompute()) ? "Shaders/TemporalAACompute.ankiprog" : "Shaders/TemporalAARaster.ankiprog",
		m_prog));

	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addConstant("VARIANCE_CLIPPING_GAMMA", 2.7f);
		variantInitInfo.addConstant("BLEND_FACTOR", 1.0f / 16.0f);
		variantInitInfo.addMutation("VARIANCE_CLIPPING", 1);
		variantInitInfo.addMutation("YCBCR", 0);

		if(getConfig().getRPreferCompute())
		{
			variantInitInfo.addConstant("FB_SIZE",
										UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()));
		}

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProg = variant->getProgram();
	}

	for(U i = 0; i < 2; ++i)
	{
		TextureUsageBit usage = TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE;
		usage |= (getConfig().getRPreferCompute()) ? TextureUsageBit::IMAGE_COMPUTE_WRITE
												   : TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;

		TextureInitInfo texinit =
			m_r->create2DRenderTargetInitInfo(m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
											  m_r->getHdrFormat(), usage, "TemporalAA");

		texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		m_rtTextures[i] = m_r->createAndClearRenderTarget(texinit);
	}

	m_tonemappedRtDescr = m_r->create2DRenderTargetDescription(
		m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
		(getGrManager().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::R8G8B8_UNORM
																			  : Format::R8G8B8A8_UNORM,
		"TemporalAA Tonemapped");
	m_tonemappedRtDescr.bake();

	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.bake();

	return Error::NONE;
}

void TemporalAA::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const U32 historyRtIdx = (m_r->getFrameCount() + 1) & 1;
	const U32 renderRtIdx = !historyRtIdx;
	const Bool preferCompute = getConfig().getRPreferCompute();

	// Import RTs
	if(ANKI_LIKELY(m_rtTexturesImportedOnce[historyRtIdx]))
	{
		m_runCtx.m_historyRt = rgraph.importRenderTarget(m_rtTextures[historyRtIdx]);
	}
	else
	{
		m_runCtx.m_historyRt = rgraph.importRenderTarget(m_rtTextures[historyRtIdx], TextureUsageBit::SAMPLED_FRAGMENT);
		m_rtTexturesImportedOnce[historyRtIdx] = true;
	}

	m_runCtx.m_renderRt = rgraph.importRenderTarget(m_rtTextures[renderRtIdx], TextureUsageBit::NONE);
	m_runCtx.m_tonemappedRt = rgraph.newRenderTarget(m_tonemappedRtDescr);

	// Create pass
	TextureUsageBit readUsage;
	RenderPassDescriptionBase* prpass;
	if(preferCompute)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("TemporalAA");

		pass.newDependency(RenderPassDependency(m_runCtx.m_renderRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));
		pass.newDependency(RenderPassDependency(m_runCtx.m_tonemappedRt, TextureUsageBit::IMAGE_COMPUTE_WRITE));

		readUsage = TextureUsageBit::SAMPLED_COMPUTE;

		prpass = &pass;
	}
	else
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("TemporalAA");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_renderRt, m_runCtx.m_tonemappedRt});

		pass.newDependency(RenderPassDependency(m_runCtx.m_renderRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));
		pass.newDependency(
			RenderPassDependency(m_runCtx.m_tonemappedRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE));

		readUsage = TextureUsageBit::SAMPLED_FRAGMENT;

		prpass = &pass;
	}

	prpass->newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), readUsage,
											   TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)));
	prpass->newDependency(RenderPassDependency(m_r->getLightShading().getRt(), readUsage));
	prpass->newDependency(RenderPassDependency(m_runCtx.m_historyRt, readUsage));
	prpass->newDependency(RenderPassDependency(m_r->getMotionVectors().getMotionVectorsRt(), readUsage));

	prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);

		cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
		rgraphCtx.bindColorTexture(0, 2, m_r->getLightShading().getRt());
		rgraphCtx.bindColorTexture(0, 3, m_runCtx.m_historyRt);
		rgraphCtx.bindColorTexture(0, 4, m_r->getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindUniformBuffer(0, 5, m_r->getTonemapping().getAverageLuminanceBuffer());

		if(getConfig().getRPreferCompute())
		{
			rgraphCtx.bindImage(0, 6, m_runCtx.m_renderRt, TextureSubresourceInfo());
			rgraphCtx.bindImage(0, 7, m_runCtx.m_tonemappedRt, TextureSubresourceInfo());

			dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
		}
		else
		{
			cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

			cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
		}
	});
}

} // end namespace anki
