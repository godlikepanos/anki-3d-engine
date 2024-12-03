// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/TemporalUpscaler.h>
#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/ImageAtlasResource.h>

namespace anki {

Error Tonemapping::init()
{
	{
		m_expAndAvgLum.m_inputTexMip =
			(getRenderer().getBloom().getPyramidTextureMipmapCount() > 2) ? getRenderer().getBloom().getPyramidTextureMipmapCount() - 2 : 0;

		// Create program
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/TonemappingAverageLuminance.ankiprogbin", m_expAndAvgLum.m_prog, m_expAndAvgLum.m_grProg));

		// Create exposure texture.
		// WARNING: Use it only as IMAGE and nothing else. It will not be tracked by the rendergraph. No tracking means no automatic image transitions
		const TextureUsageBit usage = TextureUsageBit::kAllUav;
		const TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(1, 1, Format::kR16G16_Sfloat, usage, "ExposureAndAvgLum1x1");
		ClearValue clearValue;
		clearValue.m_colorf = {0.5f, 0.5f, 0.5f, 0.5f};
		m_expAndAvgLum.m_exposureAndAvgLuminance1x1 = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kUavCompute, clearValue);
	}

	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Tonemap.ankiprogbin", m_tonemapping.m_prog, m_tonemapping.m_grProg));

		m_tonemapping.m_rtDesc = getRenderer().create2DRenderTargetDescription(
			getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y(),
			(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm : Format::kR8G8B8A8_Unorm,
			"Tonemapped");
		m_tonemapping.m_rtDesc.bake();

		m_tonemapping.m_lut.reset(nullptr);
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/DefaultLut.ankitex", m_tonemapping.m_lut));
		ANKI_ASSERT(m_tonemapping.m_lut->getWidth() == m_tonemapping.m_lut->getHeight());
		ANKI_ASSERT(m_tonemapping.m_lut->getWidth() == m_tonemapping.m_lut->getDepth());
	}

	return Error::kNone;
}

void Tonemapping::importRenderTargets(RenderingContext& ctx)
{
	// Just import it. It will not be used in resource tracking
	m_runCtx.m_exposureLuminanceHandle =
		ctx.m_renderGraphDescr.importRenderTarget(m_expAndAvgLum.m_exposureAndAvgLuminance1x1.get(), TextureUsageBit::kUavCompute);
}

void Tonemapping::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(Tonemapping);
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create avg lum pass
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("AvgLuminance");

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Tonemapping);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_expAndAvgLum.m_grProg.get());
			rgraphCtx.bindUav(0, 0, m_runCtx.m_exposureLuminanceHandle);
			rgraphCtx.bindSrv(0, 0, getRenderer().getBloom().getPyramidRt(), TextureSubresourceDesc::surface(m_expAndAvgLum.m_inputTexMip, 0, 0));

			cmdb.dispatchCompute(1, 1, 1);
		});

		pass.newTextureDependency(getRenderer().getBloom().getPyramidRt(), TextureUsageBit::kSrvCompute,
								  TextureSubresourceDesc::surface(m_expAndAvgLum.m_inputTexMip, 0, 0));
	}

	// Tonemapp pass
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_tonemapping.m_rtDesc);
		const RenderTargetHandle inRt =
			(getRenderer().getTemporalUpscaler().getEnabled()) ? getRenderer().getTemporalUpscaler().getRt() : getRenderer().getTemporalAA().getRt();
		const RenderTargetHandle outRt = m_runCtx.m_rt;

		RenderPassBase* ppass;
		if(g_preferComputeCVar)
		{
			NonGraphicsRenderPass& pass = ctx.m_renderGraphDescr.newNonGraphicsRenderPass("Tonemap");
			pass.newTextureDependency(inRt, TextureUsageBit::kSrvCompute);
			pass.newTextureDependency(outRt, TextureUsageBit::kUavCompute);
			ppass = &pass;
		}
		else
		{
			GraphicsRenderPass& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Tonemap");
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(outRt)});
			pass.newTextureDependency(inRt, TextureUsageBit::kSrvPixel);
			pass.newTextureDependency(outRt, TextureUsageBit::kRtvDsvWrite);
			ppass = &pass;
		}

		ppass->setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Tonemapping);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			const Bool preferCompute = g_preferComputeCVar;
			const RenderTargetHandle inRt = (getRenderer().getTemporalUpscaler().getEnabled()) ? getRenderer().getTemporalUpscaler().getRt()
																							   : getRenderer().getTemporalAA().getRt();
			const RenderTargetHandle outRt = m_runCtx.m_rt;

			cmdb.bindShaderProgram(m_tonemapping.m_grProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
			rgraphCtx.bindSrv(0, 0, inRt);
			cmdb.bindSrv(1, 0, TextureView(&m_tonemapping.m_lut->getTexture(), TextureSubresourceDesc::all()));
			rgraphCtx.bindUav(0, 0, m_runCtx.m_exposureLuminanceHandle);

			if(preferCompute)
			{
				rgraphCtx.bindUav(1, 0, outRt);
				dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
			}
			else
			{
				cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}
}

} // end namespace anki
