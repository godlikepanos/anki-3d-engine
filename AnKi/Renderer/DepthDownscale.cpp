// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Core/CVarSet.h>
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
	const U32 width = getRenderer().getInternalResolution().x() / 2;
	const U32 height = getRenderer().getInternalResolution().y() / 2;

	m_mipCount = 2;

	const UVec2 lastMipSize = UVec2(width, height) >> (m_mipCount - 1);

	ANKI_R_LOGV("Initializing HiZ. Mip count %u, last mip size %ux%u", m_mipCount, lastMipSize.x(), lastMipSize.y());

	const Bool preferCompute = g_preferComputeCVar.get();

	// Create RT descr
	{
		m_rtDescr = getRenderer().create2DRenderTargetDescription(width, height, Format::kR32_Sfloat, "Downscaled depth");
		m_rtDescr.m_mipmapCount = U8(m_mipCount);
		m_rtDescr.bake();
	}

	// Progs
	if(preferCompute)
	{
		ANKI_CHECK(
			loadShaderProgram("ShaderBinaries/DepthDownscaleCompute.ankiprogbin", Array<SubMutation, 1>{{{"WAVE_OPERATIONS", 0}}}, m_prog, m_grProg));
	}
	else
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/DepthDownscaleRaster.ankiprogbin", m_prog, m_grProg));
	}

	// Counter buffer
	if(preferCompute)
	{
		BufferInitInfo buffInit("Depth downscale counter buffer");
		buffInit.m_size = sizeof(U32);
		buffInit.m_usage = BufferUsageBit::kUavComputeWrite | BufferUsageBit::kTransferDestination;
		m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

		// Zero it
		CommandBufferInitInfo cmdbInit;
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		cmdb->fillBuffer(m_counterBuffer.get(), 0, kMaxPtrSize, 0);

		FencePtr fence;
		cmdb->flush({}, &fence);

		fence->clientWait(6.0_sec);
	}

	if(!preferCompute)
	{
		m_fbDescrs.resize(m_mipCount);
		for(U32 mip = 0; mip < m_mipCount; ++mip)
		{
			FramebufferDescription& fbDescr = m_fbDescrs[mip];
			fbDescr.m_colorAttachmentCount = 1;
			fbDescr.m_colorAttachments[0].m_surface.m_level = mip;
			fbDescr.bake();
		}
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

void DepthDownscale::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(DepthDownscale);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	if(g_preferComputeCVar.get())
	{
		// Do it with compute

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Depth downscale");

		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledCompute,
								  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		for(U32 mip = 0; mip < m_mipCount; ++mip)
		{
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = mip;
			pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavComputeWrite, subresource);
		}

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_grProg.get());

			varAU2(dispatchThreadGroupCountXY);
			varAU2(workGroupOffset); // needed if Left and Top are not 0,0
			varAU2(numWorkGroupsAndMips);
			varAU4(rectInfo) = initAU4(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
			SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, m_mipCount);

			DepthDownscaleConstants pc;
			pc.m_threadgroupCount = numWorkGroupsAndMips[0];
			pc.m_mipmapCount = numWorkGroupsAndMips[1];
			pc.m_srcTexSizeOverOne = 1.0f / Vec2(getRenderer().getInternalResolution());

			cmdb.setPushConstants(&pc, sizeof(pc));

			for(U32 mip = 0; mip < kMaxMipsSinglePassDownsamplerCanProduce; ++mip)
			{
				TextureSubresourceInfo subresource;
				if(mip < m_mipCount)
				{
					subresource.m_firstMipmap = mip;
				}
				else
				{
					subresource.m_firstMipmap = 0; // Put something random
				}

				rgraphCtx.bindUavTexture(0, 0, m_runCtx.m_rt, subresource, mip);
			}

			cmdb.bindUavBuffer(0, 1, m_counterBuffer.get(), 0, sizeof(U32));

			cmdb.bindSampler(0, 2, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindTexture(0, 3, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			cmdb.dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
		});
	}
	else
	{
		// Do it with raster

		for(U32 mip = 0; mip < m_mipCount; ++mip)
		{
			static constexpr Array<CString, 4> passNames = {"Depth downscale #1", "Depth downscale #2", "Depth downscale #3", "Depth downscale #4"};
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[mip]);
			pass.setFramebufferInfo(m_fbDescrs[mip], {m_runCtx.m_rt});

			if(mip == 0)
			{
				pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment,
										  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			}
			else
			{
				TextureSurfaceInfo subresource;
				subresource.m_level = mip - 1;
				pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kSampledFragment, subresource);
			}

			TextureSurfaceInfo subresource;
			subresource.m_level = mip;
			pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite, subresource);

			pass.setWork([this, mip](RenderPassWorkContext& rgraphCtx) {
				CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

				cmdb.bindShaderProgram(m_grProg.get());
				cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());

				if(mip == 0)
				{
					rgraphCtx.bindTexture(0, 0, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
				}
				else
				{
					TextureSubresourceInfo subresource;
					subresource.m_firstMipmap = mip - 1;
					rgraphCtx.bindTexture(0, 0, m_runCtx.m_rt, subresource);
				}

				const UVec2 size = (getRenderer().getInternalResolution() / 2) >> mip;
				cmdb.setViewport(0, 0, size.x(), size.y());
				cmdb.draw(PrimitiveTopology::kTriangles, 3);
			});
		}
	}
}

} // end namespace anki
