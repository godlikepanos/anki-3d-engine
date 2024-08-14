// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	return Error::kNone;
}

Error Bloom::initExposure()
{
	const U32 width = getRenderer().getDownscaleBlur().getPassWidth(kMaxU32) * 2;
	const U32 height = getRenderer().getDownscaleBlur().getPassHeight(kMaxU32) * 2;

	// Create RT info
	m_exposure.m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, getRenderer().getHdrFormat(), "Bloom Exp");
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
	m_upscale.m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, getRenderer().getHdrFormat(), "Bloom Upscale");
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

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_preferComputeCVar.get();

	// Main pass
	{
		// Ask for render target
		m_runCtx.m_exposureRt = rgraph.newRenderTarget(m_exposure.m_rtDescr);

		// Set the render pass
		const TextureSubresourceDesc inputTexSubresource =
			TextureSubresourceDesc::surface(getRenderer().getDownscaleBlur().getMipmapCount() - 1, 0, 0);

		RenderPassBase* prpass;
		if(preferCompute)
		{
			NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Bloom Main");

			rpass.newTextureDependency(getRenderer().getDownscaleBlur().getRt(), TextureUsageBit::kSrvCompute, inputTexSubresource);
			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kUavCompute);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPass& rpass = rgraph.newGraphicsRenderPass("Bloom Main");
			rpass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_exposureRt)});

			rpass.newTextureDependency(getRenderer().getDownscaleBlur().getRt(), TextureUsageBit::kSrvFragment, inputTexSubresource);
			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kRtvDsvWrite);

			prpass = &rpass;
		}

		prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_exposure.m_grProg.get());

			const TextureSubresourceDesc inputTexSubresource =
				TextureSubresourceDesc::surface(getRenderer().getDownscaleBlur().getMipmapCount() - 1, 0, 0);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, getRenderer().getDownscaleBlur().getRt(), inputTexSubresource);

			const Vec4 consts(g_bloomThresholdCVar.get(), g_bloomScaleCVar.get(), 0.0f, 0.0f);
			cmdb.setPushConstants(&consts, sizeof(consts));

			rgraphCtx.bindUav(0, 0, getRenderer().getTonemapping().getRt());

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUav(1, 0, m_runCtx.m_exposureRt);

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
		RenderPassBase* prpass;
		if(preferCompute)
		{
			NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Bloom Upscale");

			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kSrvCompute);
			rpass.newTextureDependency(m_runCtx.m_upscaleRt, TextureUsageBit::kUavCompute);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPass& rpass = rgraph.newGraphicsRenderPass("Bloom Upscale");
			rpass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_upscaleRt)});

			rpass.newTextureDependency(m_runCtx.m_exposureRt, TextureUsageBit::kSrvFragment);
			rpass.newTextureDependency(m_runCtx.m_upscaleRt, TextureUsageBit::kRtvDsvWrite);

			prpass = &rpass;
		}

		prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscale.m_grProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, m_runCtx.m_exposureRt);
			cmdb.bindSrv(1, 0, TextureView(&m_upscale.m_lensDirtImage->getTexture(), TextureSubresourceDesc::all()));

			if(g_preferComputeCVar.get())
			{
				rgraphCtx.bindUav(0, 0, m_runCtx.m_upscaleRt);

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
