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
#include <AnKi/Resource/ImageResource.h>

namespace anki {

Error Tonemapping::init()
{
	{
		ANKI_CHECK(m_histogram.m_prog.load("ShaderBinaries/Tonemap.ankiprogbin", {}, "Histogram"));

		m_histogram.m_histogramBuff = getRenderer().getRendedererGpuMemoryPool().allocateStructuredBuffer<U32>(Histogram::kBinCount);
		zeroBuffer(m_histogram.m_histogramBuff);

		m_histogram.m_inputTexMip = (getBloom().getPyramidTextureMipmapCount() > 1) ? 1 : 0;
	}

	{
		// Create program
		ANKI_CHECK(m_expAndAvgLum.m_prog.load("ShaderBinaries/Tonemap.ankiprogbin", {}, "AvgLuminance"));

		// Create the exposure texture. WARNING: Use it only as UAV, including for reads, since every pass that touches it declares a UAV dependency
		const TextureUsageBit usage = TextureUsageBit::kAllUav;
		const TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(1, 1, Format::kR32G32_Sfloat, usage, "ExposureAndAvgLum1x1");
		ClearValue clearValue;
		clearValue.m_colorf = {0.5f, 0.5f, 0.5f, 0.5f};
		m_expAndAvgLum.m_exposureAndAvgLuminance1x1 = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kUavCompute, clearValue);
	}

	{
		ANKI_CHECK(m_tonemapping.m_prog.load("ShaderBinaries/Tonemap.ankiprogbin", {}, "Tonemapping"));

		m_tonemapping.m_rtDesc = getRenderer().create2DRenderTargetDescription(
			getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y,
			(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm : Format::kR8G8B8A8_Unorm,
			"Tonemapped");
		m_tonemapping.m_rtDesc.bake();

		ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/DefaultLut.ankitex", m_tonemapping.m_lut));
		ANKI_ASSERT(m_tonemapping.m_lut->getTexture().getWidth() == m_tonemapping.m_lut->getTexture().getHeight());
		ANKI_ASSERT(m_tonemapping.m_lut->getTexture().getWidth() == m_tonemapping.m_lut->getTexture().getDepth());
	}

	return Error::kNone;
}

void Tonemapping::importRenderTargets()
{
	m_runCtx.m_exposureLuminanceHandle = getRenderingContext().m_renderGraphDescr.importRenderTarget(
		m_expAndAvgLum.m_exposureAndAvgLuminance1x1.get(), !m_expAndAvgLum.m_importedOnce, TextureUsageBit::kUavCompute);
	m_expAndAvgLum.m_importedOnce = true;
}

void Tonemapping::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(Tonemapping);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	BufferHandle histogramHandle = rgraph.importBuffer(m_histogram.m_histogramBuff, BufferUsageBit::kNone);

	// Histogram
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Tonemapping: Histogram");

		pass.newTextureDependency(getRenderer().getBloom().getPyramidRt(), TextureUsageBit::kSrvCompute,
								  TextureSubresourceDesc::surface(m_histogram.m_inputTexMip, 0, 0));
		pass.newBufferDependency(histogramHandle, BufferUsageBit::kUavCompute);

		pass.setWork([this, histogramHandle](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Tonemapping);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_histogram.m_prog.get());
			rgraphCtx.bindSrv(0, 0, getRenderer().getBloom().getPyramidRt(), TextureSubresourceDesc::surface(m_histogram.m_inputTexMip, 0, 0));
			rgraphCtx.bindUav(0, 0, histogramHandle);

			const Vec4 consts(g_cvarRenderTonemappingMinLog2Luminance, g_cvarRenderTonemappingMaxLog2Luminance, 0.0f, 0.0f);
			cmdb.setFastConstants(&consts, sizeof(consts));

			const UVec2 inputTexSize = getRenderer().getInternalResolution() >> (1u + m_histogram.m_inputTexMip);
			dispatchPPCompute(cmdb, 8, 8, inputTexSize.x, inputTexSize.y);
		});
	}

	// Avg luminance
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Tonemapping: AvgLuminance");

		pass.newBufferDependency(histogramHandle, BufferUsageBit::kUavCompute);
		pass.newTextureDependency(m_runCtx.m_exposureLuminanceHandle, TextureUsageBit::kUavCompute);

		pass.setWork([this, histogramHandle](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Tonemapping);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_expAndAvgLum.m_prog.get());

			// The fraction of the way towards the metered luminance to travel this frame. Derived from the frame time so that the adaptation takes
			// the same wall clock time regardless of the frame rate. See https://bruop.github.io/exposure/
			const F32 timeCoeff = saturate(1.0f - std::exp(-F32(getRenderingContext().m_dt) * F32(g_cvarRenderTonemappingAdaptationRate)));

			const Array<Vec4, 2> consts = {
				Vec4(g_cvarRenderTonemappingMinLog2Luminance, g_cvarRenderTonemappingMaxLog2Luminance, timeCoeff, 0.0f),
				Vec4(g_cvarRenderTonemappingDarkPixelTrimPercent / 100.0f, g_cvarRenderTonemappingBrightPixelTrimPercent / 100.f, 0.0f, 0.0f)};
			cmdb.setFastConstants(&consts, sizeof(consts));

			rgraphCtx.bindUav(0, 0, histogramHandle);
			rgraphCtx.bindUav(1, 0, m_runCtx.m_exposureLuminanceHandle);

			cmdb.dispatchCompute(1, 1, 1);
		});
	}

	// Tonemapp pass
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_tonemapping.m_rtDesc);
		const RenderTargetHandle inRt = (getTemporalUpscaler().getEnabled()) ? getTemporalUpscaler().getRt() : getTemporalAA().getRt();
		const RenderTargetHandle outRt = m_runCtx.m_rt;

		TextureUsageBit readUsage, writeUsage, exposureUsage;
		RenderPassBase* ppass;
		if(g_cvarRenderPreferCompute)
		{
			NonGraphicsRenderPass& pass = getRenderingContext().m_renderGraphDescr.newNonGraphicsRenderPass("Tonemap");
			ppass = &pass;
			readUsage = TextureUsageBit::kSrvCompute;
			writeUsage = TextureUsageBit::kUavCompute;
			exposureUsage = TextureUsageBit::kUavCompute;
		}
		else
		{
			GraphicsRenderPass& pass = getRenderingContext().m_renderGraphDescr.newGraphicsRenderPass("Tonemap");
			pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(outRt)});
			ppass = &pass;
			readUsage = TextureUsageBit::kSrvPixel;
			writeUsage = TextureUsageBit::kRtvDsvWrite;
			exposureUsage = TextureUsageBit::kUavPixel;
		}

		ppass->newTextureDependency(inRt, readUsage);
		ppass->newTextureDependency(outRt, writeUsage);
		ppass->newTextureDependency(m_runCtx.m_exposureLuminanceHandle, exposureUsage);

		ppass->setWork([this, inRt, outRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(Tonemapping);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			const Bool preferCompute = g_cvarRenderPreferCompute;

			cmdb.bindShaderProgram(m_tonemapping.m_prog.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindSrv(0, 0, inRt);
			cmdb.bindSrv(1, 0, TextureView(&m_tonemapping.m_lut->getTexture(), TextureSubresourceDesc::all()));
			rgraphCtx.bindUav(0, 0, m_runCtx.m_exposureLuminanceHandle);

			if(preferCompute)
			{
				rgraphCtx.bindUav(1, 0, outRt);
				dispatchPPCompute(cmdb, 8, 8, getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y);
			}
			else
			{
				cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x, getRenderer().getPostProcessResolution().y);
				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}
}

} // end namespace anki
