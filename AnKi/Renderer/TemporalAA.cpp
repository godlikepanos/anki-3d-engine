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

	return Error::kNone;
}

Error TemporalAA::initInternal()
{
	ANKI_R_LOGV("Initializing TAA");

	ANKI_CHECK(m_r->getResourceManager().loadResource((getConfig().getRPreferCompute())
														  ? "ShaderBinaries/TemporalAACompute.ankiprogbin"
														  : "ShaderBinaries/TemporalAARaster.ankiprogbin",
													  m_prog));

	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addConstant("kVarianceClippingGamma", 2.7f); // Variance clipping paper proposes 1.0
		variantInitInfo.addConstant("kBlendFactor", 1.0f / 16.0f);
		variantInitInfo.addMutation("VARIANCE_CLIPPING", 1);
		variantInitInfo.addMutation("YCBCR", 0);

		if(getConfig().getRPreferCompute())
		{
			variantInitInfo.addConstant("kFramebufferSize",
										UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()));
		}

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProg = variant->getProgram();
	}

	for(U i = 0; i < 2; ++i)
	{
		TextureUsageBit usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute;
		usage |= (getConfig().getRPreferCompute()) ? TextureUsageBit::kImageComputeWrite
												   : TextureUsageBit::kFramebufferWrite;

		TextureInitInfo texinit =
			m_r->create2DRenderTargetInitInfo(m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
											  m_r->getHdrFormat(), usage, "TemporalAA");

		m_rtTextures[i] = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
	}

	m_tonemappedRtDescr = m_r->create2DRenderTargetDescription(
		m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
		(getGrManager().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm
																			  : Format::kR8G8B8A8_Unorm,
		"TemporalAA Tonemapped");
	m_tonemappedRtDescr.bake();

	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.bake();

	return Error::kNone;
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
		m_runCtx.m_historyRt = rgraph.importRenderTarget(m_rtTextures[historyRtIdx], TextureUsageBit::kSampledFragment);
		m_rtTexturesImportedOnce[historyRtIdx] = true;
	}

	m_runCtx.m_renderRt = rgraph.importRenderTarget(m_rtTextures[renderRtIdx], TextureUsageBit::kNone);
	m_runCtx.m_tonemappedRt = rgraph.newRenderTarget(m_tonemappedRtDescr);

	// Create pass
	TextureUsageBit readUsage;
	RenderPassDescriptionBase* prpass;
	if(preferCompute)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("TemporalAA");

		pass.newTextureDependency(m_runCtx.m_renderRt, TextureUsageBit::kImageComputeWrite);
		pass.newTextureDependency(m_runCtx.m_tonemappedRt, TextureUsageBit::kImageComputeWrite);

		readUsage = TextureUsageBit::kSampledCompute;

		prpass = &pass;
	}
	else
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("TemporalAA");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_renderRt, m_runCtx.m_tonemappedRt});

		pass.newTextureDependency(m_runCtx.m_renderRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(m_runCtx.m_tonemappedRt, TextureUsageBit::kFramebufferWrite);

		readUsage = TextureUsageBit::kSampledFragment;

		prpass = &pass;
	}

	prpass->newTextureDependency(m_r->getGBuffer().getDepthRt(), readUsage,
								 TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	prpass->newTextureDependency(m_r->getLightShading().getRt(), readUsage);
	prpass->newTextureDependency(m_runCtx.m_historyRt, readUsage);
	prpass->newTextureDependency(m_r->getMotionVectors().getMotionVectorsRt(), readUsage);

	prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);

		cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 2, m_r->getLightShading().getRt());
		rgraphCtx.bindColorTexture(0, 3, m_runCtx.m_historyRt);
		rgraphCtx.bindColorTexture(0, 4, m_r->getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindImage(0, 5, m_r->getTonemapping().getRt());

		if(getConfig().getRPreferCompute())
		{
			rgraphCtx.bindImage(0, 6, m_runCtx.m_renderRt, TextureSubresourceInfo());
			rgraphCtx.bindImage(0, 7, m_runCtx.m_tonemappedRt, TextureSubresourceInfo());

			dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
		}
		else
		{
			cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

			cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
		}
	});
}

} // end namespace anki
