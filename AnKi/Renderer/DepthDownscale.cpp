// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-function"
#	pragma GCC diagnostic ignored "-Wignored-qualifiers"
#elif ANKI_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable : 4505)
#endif
#define A_CPU
#include <ThirdParty/FidelityFX/ffx_a.h>
#include <ThirdParty/FidelityFX/ffx_spd.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#elif ANKI_COMPILER_MSVC
#	pragma warning(pop)
#endif

namespace anki {

DepthDownscale::~DepthDownscale()
{
}

Error DepthDownscale::initInternal()
{
	const U32 width = getRenderer().getInternalResolution().x / 2;
	const U32 height = getRenderer().getInternalResolution().y / 2;

	m_mipCount = 2;

	const Bool preferCompute = g_cvarRenderPreferCompute;

	// Create RT descr
	{
		m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, Format::kR32_Sfloat, "Downscaled depth");
		m_rtDescr.m_mipmapCount = m_mipCount;
		m_rtDescr.bake();
	}

	// Progs
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/DepthDownscale.ankiprogbin", {{"WAVE_OPERATIONS", 1}}, m_prog, m_grProg));

	// Counter buffer
	if(preferCompute)
	{
		m_counterBuffer = getRenderer().getRendedererGpuMemoryPool().allocateStructuredBuffer<U32>(1);
		zeroBuffer(m_counterBuffer);
	}

	return Error::kNone;
}

Error DepthDownscale::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize depth downscale passes");
	}

	return err;
}

void DepthDownscale::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(DepthDownscale);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	if(g_cvarRenderPreferCompute)
	{
		// Do it with compute

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Depth downscale");

		pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

		pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavCompute);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(DepthDownscale);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_grProg.get());

			varAU2(dispatchThreadGroupCountXY);
			varAU2(workGroupOffset); // needed if Left and Top are not 0,0
			varAU2(numWorkGroupsAndMips);
			varAU4(rectInfo) = initAU4(0, 0, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
			SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, m_mipCount);

			DepthDownscaleConstants pc;
			pc.m_threadgroupCount = numWorkGroupsAndMips[0];
			pc.m_mipmapCount = numWorkGroupsAndMips[1];
			pc.m_srcTexSizeOverOne = 1.0f / Vec2(getRenderer().getInternalResolution());

			cmdb.setFastConstants(&pc, sizeof(pc));

			for(U8 mip = 0; mip < kMaxMipsSinglePassDownsamplerCanProduce; ++mip)
			{
				TextureSubresourceDesc surface = TextureSubresourceDesc::firstSurface();
				if(mip < m_mipCount)
				{
					surface.m_mipmap = mip;
				}
				else
				{
					surface.m_mipmap = 0; // Put something random
				}

				rgraphCtx.bindUav(mip + 1, 0, m_runCtx.m_rt, surface);
			}

			cmdb.bindUav(0, 0, m_counterBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());

			cmdb.dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
		});
	}
	else
	{
		// Do it with raster

		for(U8 mip = 0; mip < m_mipCount; ++mip)
		{
			static constexpr Array<CString, 4> passNames = {"Depth downscale #1", "Depth downscale #2", "Depth downscale #3", "Depth downscale #4"};
			GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass(passNames[mip]);

			GraphicsRenderPassTargetDesc rti(m_runCtx.m_rt);
			rti.m_subresource.m_mipmap = mip;
			pass.setRenderpassInfo({rti});

			if(mip == 0)
			{
				pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel);
			}
			else
			{
				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kSrvPixel, TextureSubresourceDesc::surface(mip - 1, 0, 0));
			}

			pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kRtvDsvWrite, TextureSubresourceDesc::surface(mip, 0, 0));

			pass.setWork([this, mip](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(DepthDownscale);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_grProg.get());
				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

				if(mip == 0)
				{
					rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
				}
				else
				{
					rgraphCtx.bindSrv(0, 0, m_runCtx.m_rt, TextureSubresourceDesc::surface(mip - 1, 0, 0));
				}

				const UVec2 size = (getRenderer().getInternalResolution() / 2) >> mip;
				cmdb.setViewport(0, 0, size.x, size.y);
				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			});
		}
	}
}

} // end namespace anki
