// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error Bloom::init()
{
	// Pyramid
	{
		const UVec2 pyramidSize = getRenderer().getInternalResolution() / 2;
		const U8 pyramidMipCount = computeMaxMipmapCount2d(pyramidSize.x, pyramidSize.y, g_cvarRenderBloomPyramidLowLimit);

		const Bool preferCompute = g_cvarRenderPreferCompute;

		// Create the miped texture
		TextureInitInfo texinit =
			getRenderer().create2DRenderTargetDescription(pyramidSize.x, pyramidSize.y, getRenderer().getHdrFormat(), "Bloom pyramid");
		texinit.m_usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kSrvCompute;
		texinit.m_usage |= (preferCompute) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;
		texinit.m_mipmapCount = pyramidMipCount;
		m_pyramidTex = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvCompute);

		// Shader programs
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Bloom.ankiprogbin", {}, m_prog, m_downscaleGrProg, "Downscale"));
	}

	// Exposure
	{
		const UVec2 pyramidSmallerMipSize = UVec2(m_pyramidTex->getWidth(), m_pyramidTex->getHeight()) >> (m_pyramidTex->getMipmapCount() - 1);

		const UVec2 expSize = pyramidSmallerMipSize * 2; // Upacale a bit

		// Create RT info
		m_exposureRtDesc = getRenderer().create2DRenderTargetDescription(expSize.x, expSize.y, getRenderer().getHdrFormat(), "Bloom exposure");
		m_exposureRtDesc.bake();

		// init shaders
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Bloom.ankiprogbin", {}, m_prog, m_exposureGrProg, "Exposure"));
	}

	// Upscale
	{
		const UVec2 size = getRenderer().getPostProcessResolution() / g_cvarRenderBloomUpscaleDivisor;

		// Create RT descr
		m_finalRtDesc = getRenderer().create2DRenderTargetDescription(size.x, size.y, getRenderer().getHdrFormat(), "Bloom final");
		m_finalRtDesc.bake();

		// init shaders
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/Bloom.ankiprogbin", {}, m_prog, m_upscaleGrProg, "Upscale"));

		// Textures
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/LensDirt.ankitex", m_lensDirtImg, false));
	}

	return Error::kNone;
}

void Bloom::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_pyramidRt = rgraph.importRenderTarget(m_pyramidTex.get(), TextureUsageBit::kSrvCompute);
}

void Bloom::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;
	const Bool preferCompute = g_cvarRenderPreferCompute;

	// Pyramid generation
	{
		const U32 passCount = m_pyramidTex->getMipmapCount();
		const RenderTargetHandle inRt = getRenderer().getLightShading().getRt();

		for(U32 i = 0; i < passCount; ++i)
		{
			RenderPassBase* ppass;
			if(preferCompute)
			{
				ppass = &rgraph.newNonGraphicsRenderPass(generateTempPassName("Bloom pyramid %u", i));
			}
			else
			{
				GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(generateTempPassName("Bloom pyramid %u", i));

				GraphicsRenderPassTargetDesc rtInf(m_runCtx.m_pyramidRt);
				rtInf.m_subresource.m_mipmap = U8(i);
				pass.setRenderpassInfo({rtInf});

				ppass = &pass;
			}

			const TextureUsageBit readUsage = (preferCompute) ? TextureUsageBit::kSrvCompute : TextureUsageBit::kSrvPixel;
			const TextureUsageBit writeUsage = (preferCompute) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;

			if(i > 0)
			{
				const TextureSubresourceDesc sampleSubresource = TextureSubresourceDesc::surface(i - 1, 0, 0);
				const TextureSubresourceDesc renderSubresource = TextureSubresourceDesc::surface(i, 0, 0);

				ppass->newTextureDependency(m_runCtx.m_pyramidRt, writeUsage, renderSubresource);
				ppass->newTextureDependency(m_runCtx.m_pyramidRt, readUsage, sampleSubresource);
			}
			else
			{
				ppass->newTextureDependency(m_runCtx.m_pyramidRt, writeUsage, TextureSubresourceDesc::firstSurface());
				ppass->newTextureDependency(inRt, readUsage);
			}

			ppass->setWork([this, passIdx = i](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(BoomPyramid);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_downscaleGrProg.get());

				const U32 vpWidth = m_pyramidTex->getWidth() >> passIdx;
				const U32 vpHeight = m_pyramidTex->getHeight() >> passIdx;

				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

				if(passIdx > 0)
				{
					rgraphCtx.bindSrv(0, 0, m_runCtx.m_pyramidRt, TextureSubresourceDesc::surface(passIdx - 1, 0, 0));
				}
				else
				{
					rgraphCtx.bindSrv(0, 0, getRenderer().getLightShading().getRt());
				}

				if(g_cvarRenderPreferCompute)
				{
					const Vec4 fbSize(F32(vpWidth), F32(vpHeight), 0.0f, 0.0f);
					cmdb.setFastConstants(&fbSize, sizeof(fbSize));

					rgraphCtx.bindUav(1, 0, m_runCtx.m_pyramidRt, TextureSubresourceDesc::surface(passIdx, 0, 0));

					dispatchPPCompute(cmdb, 8, 8, vpWidth, vpHeight);
				}
				else
				{
					cmdb.setViewport(0, 0, vpWidth, vpHeight);

					cmdb.draw(PrimitiveTopology::kTriangles, 3);
				}
			});
		}
	}

	// Exposure
	RenderTargetHandle exposureRt;
	{
		// Ask for render target
		exposureRt = rgraph.newRenderTarget(m_exposureRtDesc);

		// Set the render pass
		const TextureSubresourceDesc inputTexSubresource = TextureSubresourceDesc::surface(m_pyramidTex->getMipmapCount() - 1, 0, 0);

		RenderPassBase* prpass;
		if(preferCompute)
		{
			NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Bloom Main");

			rpass.newTextureDependency(m_runCtx.m_pyramidRt, TextureUsageBit::kSrvCompute, inputTexSubresource);
			rpass.newTextureDependency(exposureRt, TextureUsageBit::kUavCompute);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPass& rpass = rgraph.newGraphicsRenderPass("Bloom Main");
			rpass.setRenderpassInfo({GraphicsRenderPassTargetDesc(exposureRt)});

			rpass.newTextureDependency(m_runCtx.m_pyramidRt, TextureUsageBit::kSrvPixel, inputTexSubresource);
			rpass.newTextureDependency(exposureRt, TextureUsageBit::kRtvDsvWrite);

			prpass = &rpass;
		}

		prpass->setWork([this, exposureRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(BoomExposure);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_exposureGrProg.get());

			const TextureSubresourceDesc inputTexSubresource = TextureSubresourceDesc::surface(m_pyramidTex->getMipmapCount() - 1, 0, 0);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, m_runCtx.m_pyramidRt, inputTexSubresource);
			rgraphCtx.bindUav(0, 0, getRenderer().getTonemapping().getExposureAndAvgLuminanceRt());

			const Vec4 consts(g_cvarRenderBloomThreshold, g_cvarRenderBloomScale, 0.0f, 0.0f);
			cmdb.setFastConstants(&consts, sizeof(consts));

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(1, 0, exposureRt);

				dispatchPPCompute(cmdb, 8, 8, m_exposureRtDesc.m_width, m_exposureRtDesc.m_height);
			}
			else
			{
				cmdb.setViewport(0, 0, m_exposureRtDesc.m_width, m_exposureRtDesc.m_height);

				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}

	// Upscale & SSLF pass
	RenderTargetHandle upscaledRt;
	{
		// Ask for render target
		upscaledRt = rgraph.newRenderTarget(m_finalRtDesc);
		m_runCtx.m_finalRt = upscaledRt;

		// Set the render pass
		RenderPassBase* prpass;
		if(preferCompute)
		{
			NonGraphicsRenderPass& rpass = rgraph.newNonGraphicsRenderPass("Bloom Upscale");

			rpass.newTextureDependency(exposureRt, TextureUsageBit::kSrvCompute);
			rpass.newTextureDependency(upscaledRt, TextureUsageBit::kUavCompute);

			prpass = &rpass;
		}
		else
		{
			GraphicsRenderPass& rpass = rgraph.newGraphicsRenderPass("Bloom Upscale");
			rpass.setRenderpassInfo({GraphicsRenderPassTargetDesc(upscaledRt)});

			rpass.newTextureDependency(exposureRt, TextureUsageBit::kSrvPixel);
			rpass.newTextureDependency(upscaledRt, TextureUsageBit::kRtvDsvWrite);

			prpass = &rpass;
		}

		prpass->setWork([this, exposureRt, upscaledRt](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(BoomUpscale);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_upscaleGrProg.get());

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, exposureRt);
			cmdb.bindSrv(1, 0, TextureView(&m_lensDirtImg->getTexture(), TextureSubresourceDesc::all()));

			if(g_cvarRenderPreferCompute)
			{
				rgraphCtx.bindUav(0, 0, upscaledRt);

				dispatchPPCompute(cmdb, 8, 8, m_finalRtDesc.m_width, m_finalRtDesc.m_height);
			}
			else
			{
				cmdb.setViewport(0, 0, m_finalRtDesc.m_width, m_finalRtDesc.m_height);

				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			}
		});
	}
}

} // end namespace anki
