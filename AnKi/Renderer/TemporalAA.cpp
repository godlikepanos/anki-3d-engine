// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_CHECK(ResourceManager::getSingleton().loadResource((ConfigSet::getSingleton().getRPreferCompute())
																? "ShaderBinaries/TemporalAACompute.ankiprogbin"
																: "ShaderBinaries/TemporalAARaster.ankiprogbin",
															m_prog));

	{
		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addConstant("kVarianceClippingGamma", 2.7f); // Variance clipping paper proposes 1.0
		variantInitInfo.addConstant("kBlendFactor", 1.0f / 16.0f);
		variantInitInfo.addMutation("VARIANCE_CLIPPING", 1);
		variantInitInfo.addMutation("YCBCR", 0);

		if(ConfigSet::getSingleton().getRPreferCompute())
		{
			variantInitInfo.addConstant("kFramebufferSize", UVec2(getRenderer().getInternalResolution().x(),
																  getRenderer().getInternalResolution().y()));
		}

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProg = variant->getProgram();
	}

	for(U i = 0; i < 2; ++i)
	{
		TextureUsageBit usage = TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute;
		usage |= (ConfigSet::getSingleton().getRPreferCompute()) ? TextureUsageBit::kImageComputeWrite
																 : TextureUsageBit::kFramebufferWrite;

		TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
			getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
			getRenderer().getHdrFormat(), usage, "TemporalAA");

		m_rtTextures[i] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
	}

	m_tonemappedRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
		(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm
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

	const U32 historyRtIdx = (getRenderer().getFrameCount() + 1) & 1;
	const U32 renderRtIdx = !historyRtIdx;
	const Bool preferCompute = ConfigSet::getSingleton().getRPreferCompute();

	// Import RTs
	if(m_rtTexturesImportedOnce[historyRtIdx]) [[likely]]
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

	prpass->newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage,
								 TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
	prpass->newTextureDependency(getRenderer().getLightShading().getRt(), readUsage);
	prpass->newTextureDependency(m_runCtx.m_historyRt, readUsage);
	prpass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);

	prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);

		cmdb->bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp);
		rgraphCtx.bindColorTexture(0, 1, getRenderer().getLightShading().getRt());
		rgraphCtx.bindColorTexture(0, 2, m_runCtx.m_historyRt);
		rgraphCtx.bindColorTexture(0, 3, getRenderer().getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindImage(0, 4, getRenderer().getTonemapping().getRt());

		if(ConfigSet::getSingleton().getRPreferCompute())
		{
			rgraphCtx.bindImage(0, 5, m_runCtx.m_renderRt, TextureSubresourceInfo());
			rgraphCtx.bindImage(0, 6, m_runCtx.m_tonemappedRt, TextureSubresourceInfo());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(),
							  getRenderer().getInternalResolution().y());
		}
		else
		{
			cmdb->setViewport(0, 0, getRenderer().getInternalResolution().x(),
							  getRenderer().getInternalResolution().y());

			cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
		}
	});
}

} // end namespace anki
