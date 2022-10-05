// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/Renderer.h>

namespace anki {

Error Tonemapping::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize tonemapping");
	}

	return err;
}

Error Tonemapping::initInternal()
{
	m_inputTexMip = m_r->getDownscaleBlur().getMipmapCount() - 2;
	const U32 width = m_r->getDownscaleBlur().getPassWidth(m_inputTexMip);
	const U32 height = m_r->getDownscaleBlur().getPassHeight(m_inputTexMip);

	ANKI_R_LOGV("Initializing tonemapping. Resolution %ux%u", width, height);

	// Create program
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/TonemappingAverageLuminance.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("kInputTexSize", UVec2(width, height));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// Create exposure texture.
	// WARNING: Use it only as IMAGE and nothing else. It will not be tracked by the rendergraph. No tracking means no
	// automatic image transitions
	const TextureUsageBit usage = TextureUsageBit::kAllImage;
	const TextureInitInfo texinit =
		m_r->create2DRenderTargetInitInfo(1, 1, Format::kR16G16_Sfloat, usage, "ExposureAndAvgLum1x1");
	ClearValue clearValue;
	clearValue.m_colorf = {0.5f, 0.5f, 0.5f, 0.5f};
	m_exposureAndAvgLuminance1x1 = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kAllImage, clearValue);

	return Error::kNone;
}

void Tonemapping::importRenderTargets(RenderingContext& ctx)
{
	// Just import it. It will not be used in resource tracking
	m_runCtx.m_exposureLuminanceHandle =
		ctx.m_renderGraphDescr.importRenderTarget(m_exposureAndAvgLuminance1x1, TextureUsageBit::kAllImage);
}

void Tonemapping::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create the pass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("AvgLuminance");

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);
		rgraphCtx.bindImage(0, 1, m_runCtx.m_exposureLuminanceHandle);

		TextureSubresourceInfo inputTexSubresource;
		inputTexSubresource.m_firstMipmap = m_inputTexMip;
		rgraphCtx.bindTexture(0, 0, m_r->getDownscaleBlur().getRt(), inputTexSubresource);

		cmdb->dispatchCompute(1, 1, 1);
	});

	TextureSubresourceInfo inputTexSubresource;
	inputTexSubresource.m_firstMipmap = m_inputTexMip;
	pass.newTextureDependency(m_r->getDownscaleBlur().getRt(), TextureUsageBit::kSampledCompute, inputTexSubresource);
}

} // end namespace anki
