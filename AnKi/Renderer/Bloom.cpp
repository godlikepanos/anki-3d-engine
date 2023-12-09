// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

NumericCVar<F32> g_bloomThresholdCVar(CVarSubsystem::kRenderer, "BloomThreshold", 2.5f, 0.0f, 256.0f, "Bloom threshold");
static NumericCVar<F32> g_bloomScaleCVar(CVarSubsystem::kRenderer, "BloomScale", 2.5f, 0.0f, 256.0f, "Bloom scale");

Bloom::Bloom()
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
	const U32 width = getRenderer().getDownscaleBlur().getPassWidth(kMaxU32) * 2;
	const U32 height = getRenderer().getDownscaleBlur().getPassHeight(kMaxU32) * 2;

	// Create RT info
	m_exposure.m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, kRtPixelFormat, "Bloom Exp");
	m_exposure.m_rtDescr.bake();

	// init shaders
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/Bloom.ankiprogbin", m_exposure.m_prog, m_exposure.m_grProg));

	return Error::kNone;
}

Error Bloom::initUpscale()
{
	const U32 width = getRenderer().getPostProcessResolution().x() / kBloomFraction;
	const U32 height = getRenderer().getPostProcessResolution().y() / kBloomFraction;

	// Create RT descr
	m_upscale.m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, kRtPixelFormat, "Bloom Upscale");
	m_upscale.m_rtDescr.bake();

	// init shaders
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/BloomUpscale.ankiprogbin", m_upscale.m_prog, m_upscale.m_grProg));

	// Textures
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/LensDirt.ankitex", m_upscale.m_lensDirtImage));

	return Error::kNone;
}

void Bloom::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Bloom);

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	// Main pass
	{
		// Ask for render target
		m_runCtx.m_exposureRt = rgraph.newRenderTarget(m_exposure.m_rtDescr);

		// Set the render pass
		TextureSubresourceInfo inputTexSubresource;
		inputTexSubresource.m_firstMipmap = getRenderer().getDownscaleBlur().getMipmapCount() - 1;

		RenderPassDescriptionBase* prpass;
		if(preferCompute)
		{
			ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("Bloom Main");

			rpass.newTextureDependency(getRenderer().getDownscaleBlur().getRt(), TextureUsageBit::kSampledCompute, inputTexSubresource);
			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kUavComputeWrite);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("Bloom Main");
			rpass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_exposureRt});

			rpass.newTextureDependency(getRenderer().getDownscaleBlur().getRt(), TextureUsageBit::kSampledFragment, inputTexSubresource);
			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kFramebufferWrite);

			prpass = &rpass;
		}

		prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_exposure.m_grProg.get());

			TextureSubresourceInfo inputTexSubresource;
			inputTexSubresource.m_firstMipmap = getRenderer().getDownscaleBlur().getMipmapCount() - 1;

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindTexture(0, 1, getRenderer().getDownscaleBlur().getRt(), inputTexSubresource);

			const Vec4 consts(g_bloomThresholdCVar.get(), g_bloomScaleCVar.get(), 0.0f, 0.0f);
			cmdb.setPushConstants(&consts, sizeof(consts));

			rgraphCtx.bindUavTexture(0, 2, getRenderer().getTonemapping().getRt());

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUavTexture(0, 3, m_runCtx.m_exposureRt, TextureSubresourceInfo());

				dispatchPPCompute(cmdb, 8, 8, m_exposure.m_rtDescr.m_width, m_exposure.m_rtDescr.m_height);
			}
			else
			{
				cmdb.setViewport(0, 0, m_exposure.m_rtDescr.m_width, m_exposure.m_rtDescr.m_height);

				cmdb.draw(PrimitiveTopology::kTriangles, 3);
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
			rpass.newTextureDependency(m_runCtx.m_upscaleRt, TextureUsageBit::kUavComputeWrite);

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
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscale.m_grProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_exposureRt);
			cmdb.bindTexture(0, 2, &m_upscale.m_lensDirtImage->getTextureView());

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUavTexture(0, 3, m_runCtx.m_upscaleRt, TextureSubresourceInfo());

				dispatchPPCompute(cmdb, 8, 8, m_upscale.m_rtDescr.m_width, m_upscale.m_rtDescr.m_height);
			}
			else
			{
				cmdb.setViewport(0, 0, m_upscale.m_rtDescr.m_width, m_upscale.m_rtDescr.m_height);

				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}
}

void Bloom::getDebugRenderTarget([[maybe_unused]] CString rtName, Array<RenderTargetHandle, kMaxDebugRenderTargets>& handles,
								 [[maybe_unused]] ShaderProgramPtr& optionalShaderProgram) const
{
	ANKI_ASSERT(rtName == "Bloom");
	handles[0] = m_runCtx.m_upscaleRt;
}

} // end namespace anki
