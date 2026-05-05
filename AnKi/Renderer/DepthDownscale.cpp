// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/MotionVectors.h>
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

	m_depthRtDesc = getRenderer().create2DRenderTargetDescription(width, height, Format::kR32_Sfloat, "Downscaled depth");
	m_depthRtDesc.m_mipmapCount = m_mipCount;
	m_depthRtDesc.bake();

	m_normalRtDesc = getRenderer().create2DRenderTargetDescription(
		width, height,
		(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Snorm : Format::kR8G8B8A8_Snorm,
		"Downscaled normals");
	m_normalRtDesc.bake();

	m_motionVectorsRtDesc = getRenderer().create2DRenderTargetDescription(width, height, Format::kR16G16_Sfloat, "Downscaled adjusted MVs");
	m_motionVectorsRtDesc.bake();

	Array<SubMutation, 2> mutation = {{{"PIXEL_SHADER_FIRST_DOWNSCALE", 0}, {"WAVE_OPERATIONS", 1}}};
	ANKI_CHECK(m_prog[0].load("ShaderBinaries/DepthDownscale.ankiprogbin", mutation));

	mutation[0].m_value = 1;
	ANKI_CHECK(m_prog[1].load("ShaderBinaries/DepthDownscale.ankiprogbin", mutation));

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

	m_runCtx.m_depthRt = rgraph.newRenderTarget(m_depthRtDesc);
	m_runCtx.m_normalsRt = rgraph.newRenderTarget(m_normalRtDesc);
	m_runCtx.m_motionVectorsRt = rgraph.newRenderTarget(m_motionVectorsRtDesc);

	if(g_cvarRenderPreferCompute)
	{
		// Do it with compute

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Depth downscale");

		pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getMotionVectors().getAdjustedMotionVectorsRt(), TextureUsageBit::kSrvCompute);

		pass.newTextureDependency(m_runCtx.m_depthRt, TextureUsageBit::kUavCompute);
		pass.newTextureDependency(m_runCtx.m_normalsRt, TextureUsageBit::kUavCompute);
		pass.newTextureDependency(m_runCtx.m_motionVectorsRt, TextureUsageBit::kUavCompute);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(DepthDownscale);

			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_prog[0].get());

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

			cmdb.bindUav(0, 0, m_counterBuffer);

			rgraphCtx.bindUav(1, 0, m_runCtx.m_normalsRt);
			rgraphCtx.bindUav(2, 0, m_runCtx.m_motionVectorsRt);

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

				rgraphCtx.bindUav(mip + 3, 0, m_runCtx.m_depthRt, surface);
			}

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
			rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(2));
			rgraphCtx.bindSrv(2, 0, getMotionVectors().getAdjustedMotionVectorsRt());

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

			GraphicsRenderPassTargetDesc rti(m_runCtx.m_depthRt);
			rti.m_subresource.m_mipmap = mip;

			if(mip == 0)
			{
				pass.setRenderpassInfo({rti, m_runCtx.m_normalsRt, m_runCtx.m_motionVectorsRt});
				pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvPixel);
				pass.newTextureDependency(getGBuffer().getColorRt(2), TextureUsageBit::kSrvPixel);
				pass.newTextureDependency(getMotionVectors().getAdjustedMotionVectorsRt(), TextureUsageBit::kSrvPixel);
				pass.newTextureDependency(m_runCtx.m_normalsRt, TextureUsageBit::kRtvDsvWrite);
				pass.newTextureDependency(m_runCtx.m_motionVectorsRt, TextureUsageBit::kRtvDsvWrite);
			}
			else
			{
				pass.newTextureDependency(m_runCtx.m_depthRt, TextureUsageBit::kSrvPixel, TextureSubresourceDesc::surface(mip - 1, 0, 0));
				pass.setRenderpassInfo({rti});
			}

			pass.newTextureDependency(m_runCtx.m_depthRt, TextureUsageBit::kRtvDsvWrite, TextureSubresourceDesc::surface(mip, 0, 0));

			pass.setWork([this, mip](RenderPassWorkContext& rgraphCtx) {
				ANKI_TRACE_SCOPED_EVENT(DepthDownscale);

				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				const Bool firstPass = mip == 0;
				cmdb.bindShaderProgram(m_prog[firstPass].get());
				cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

				if(mip == 0)
				{
					rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
					rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(2));
					rgraphCtx.bindSrv(2, 0, getMotionVectors().getAdjustedMotionVectorsRt());
				}
				else
				{
					rgraphCtx.bindSrv(0, 0, m_runCtx.m_depthRt, TextureSubresourceDesc::surface(mip - 1, 0, 0));
				}

				const UVec2 size = (getRenderer().getInternalResolution() / 2) >> mip;
				cmdb.setViewport(0, 0, size.x, size.y);
				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			});
		}
	}
}

} // end namespace anki
