// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

Bloom::Bloom(Renderer* r)
	: RendererObject(r)
{
	registerDebugRenderTarget("Bloom");
}

Bloom::~Bloom()
{
}

Error Bloom::initInternal()
{
	ANKI_R_LOGV("Initializing bloom");

	ANKI_CHECK(initExposure());
	ANKI_CHECK(initUpscale());
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();
	return Error::kNone;
}

Error Bloom::initExposure()
{
	m_exposure.m_width = m_r->getDownscaleBlur().getPassWidth(kMaxU32) * 2;
	m_exposure.m_height = m_r->getDownscaleBlur().getPassHeight(kMaxU32) * 2;

	// Create RT info
	m_exposure.m_rtDescr =
		m_r->create2DRenderTargetDescription(m_exposure.m_width, m_exposure.m_height, kRtPixelFormat, "Bloom Exp");
	m_exposure.m_rtDescr.bake();

	// init shaders
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "ShaderBinaries/BloomCompute.ankiprogbin"
													 : "ShaderBinaries/BloomRaster.ankiprogbin",
												 m_exposure.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_exposure.m_prog);
	if(getConfig().getRPreferCompute())
	{
		variantInitInfo.addConstant("kViewport", UVec2(m_exposure.m_width, m_exposure.m_height));
	}

	const ShaderProgramResourceVariant* variant;
	m_exposure.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_exposure.m_grProg = variant->getProgram();

	return Error::kNone;
}

Error Bloom::initUpscale()
{
	m_upscale.m_width = m_r->getPostProcessResolution().x() / kBloomFraction;
	m_upscale.m_height = m_r->getPostProcessResolution().y() / kBloomFraction;

	// Create RT descr
	m_upscale.m_rtDescr =
		m_r->create2DRenderTargetDescription(m_upscale.m_width, m_upscale.m_height, kRtPixelFormat, "Bloom Upscale");
	m_upscale.m_rtDescr.bake();

	// init shaders
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "ShaderBinaries/BloomUpscaleCompute.ankiprogbin"
													 : "ShaderBinaries/BloomUpscaleRaster.ankiprogbin",
												 m_upscale.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_upscale.m_prog);
	variantInitInfo.addConstant("kInputTextureSize", UVec2(m_exposure.m_width, m_exposure.m_height));
	if(getConfig().getRPreferCompute())
	{
		variantInitInfo.addConstant("kViewport", UVec2(m_upscale.m_width, m_upscale.m_height));
	}

	const ShaderProgramResourceVariant* variant;
	m_upscale.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_upscale.m_grProg = variant->getProgram();

	// Textures
	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/LensDirt.ankitex", m_upscale.m_lensDirtImage));

	return Error::kNone;
}

void Bloom::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = getConfig().getRPreferCompute();

	// Main pass
	{
		// Ask for render target
		m_runCtx.m_exposureRt = rgraph.newRenderTarget(m_exposure.m_rtDescr);

		// Set the render pass
		TextureSubresourceInfo inputTexSubresource;
		inputTexSubresource.m_firstMipmap = m_r->getDownscaleBlur().getMipmapCount() - 1;

		RenderPassDescriptionBase* prpass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Bloom Main");

			rpass.newTextureDependency(m_r->getDownscaleBlur().getRt(), TextureUsageBit::kSampledCompute,
									   inputTexSubresource);
			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kImageComputeWrite);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("Bloom Main");
			rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_exposureRt});

			rpass.newTextureDependency(m_r->getDownscaleBlur().getRt(), TextureUsageBit::kSampledFragment,
									   inputTexSubresource);
			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kFramebufferWrite);

			prpass = &rpass;
		}

		prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			cmdb->bindShaderProgram(m_exposure.m_grProg);

			TextureSubresourceInfo inputTexSubresource;
			inputTexSubresource.m_firstMipmap = m_r->getDownscaleBlur().getMipmapCount() - 1;

			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindTexture(0, 1, m_r->getDownscaleBlur().getRt(), inputTexSubresource);

			const Vec4 uniforms(getConfig().getRBloomThreshold(), getConfig().getRBloomScale(), 0.0f, 0.0f);
			cmdb->setPushConstants(&uniforms, sizeof(uniforms));

			rgraphCtx.bindImage(0, 2, m_r->getTonemapping().getRt());

			if(getConfig().getRPreferCompute())
			{
				rgraphCtx.bindImage(0, 3, m_runCtx.m_exposureRt, TextureSubresourceInfo());

				dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_exposure.m_width,
								  m_exposure.m_height);
			}
			else
			{
				cmdb->setViewport(0, 0, m_exposure.m_width, m_exposure.m_height);

				cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
			}
		});
	}

	// Upscale & SSLF pass
	{
		// Ask for render target
		m_runCtx.m_upscaleRt = rgraph.newRenderTarget(m_upscale.m_rtDescr);

		// Set the render pass
		RenderPassDescriptionBase* prpass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Bloom Upscale");

			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kSampledCompute);
			rpass.newTextureDependency(m_runCtx.m_upscaleRt, TextureUsageBit::kImageComputeWrite);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("Bloom Upscale");
			rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_upscaleRt});

			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kSampledFragment);
			rpass.newTextureDependency(m_runCtx.m_upscaleRt, TextureUsageBit::kFramebufferWrite);

			prpass = &rpass;
		}

		prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			cmdb->bindShaderProgram(m_upscale.m_grProg);

			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_exposureRt);
			cmdb->bindTexture(0, 2, m_upscale.m_lensDirtImage->getTextureView());

			if(getConfig().getRPreferCompute())
			{
				rgraphCtx.bindImage(0, 3, m_runCtx.m_upscaleRt, TextureSubresourceInfo());

				dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_upscale.m_width, m_upscale.m_height);
			}
			else
			{
				cmdb->setViewport(0, 0, m_upscale.m_width, m_upscale.m_height);

				cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
			}
		});
	}
}

void Bloom::getDebugRenderTarget([[maybe_unused]] CString rtName,
								 Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
								 [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
{
	ANKI_ASSERT(rtName == "Bloom");
	handles[0] = m_runCtx.m_upscaleRt;
}

} // end namespace anki
