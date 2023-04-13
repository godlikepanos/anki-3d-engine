// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/HiZ.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Core/ConfigSet.h>

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

Error HiZ::init()
{
	registerDebugRenderTarget("HiZ");

	ANKI_CHECK(
		ResourceManager::getSingleton().loadResource("ShaderBinaries/HiZReprojection.ankiprogbin", m_reproj.m_prog));

	const ShaderProgramResourceVariant* variant;
	m_reproj.m_prog->getOrCreateVariant(variant);
	m_reproj.m_grProg = variant->getProgram();

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ClearTextureCompute.ankiprogbin",
															m_clearHiZ.m_prog));
	ShaderProgramResourceVariantInitInfo variantInit(m_clearHiZ.m_prog);
	variantInit.addMutation("TEXTURE_DIMENSIONS", 2);
	variantInit.addMutation("COMPONENT_TYPE", 1);
	m_clearHiZ.m_prog->getOrCreateVariant(variantInit, variant);
	m_clearHiZ.m_grProg = variant->getProgram();

	ANKI_CHECK(
		ResourceManager::getSingleton().loadResource("ShaderBinaries/HiZGenPyramid.ankiprogbin", m_mipmapping.m_prog));
	m_mipmapping.m_prog->getOrCreateVariant(variant);
	m_mipmapping.m_grProg = variant->getProgram();

	m_hiZRtDescr = getRenderer().create2DRenderTargetDescription(ConfigSet::getSingleton().getRHiZWidth(),
																 ConfigSet::getSingleton().getRHiZHeight(),
																 Format::kR32_Uint, "HiZ U32");
	m_hiZRtDescr.m_mipmapCount = U8(computeMaxMipmapCount2d(m_hiZRtDescr.m_width, m_hiZRtDescr.m_height, 16));
	m_hiZRtDescr.bake();

	BufferInitInfo buffInit("HiZCounterBuffer");
	buffInit.m_size = sizeof(U32);
	buffInit.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kTransferDestination;
	m_mipmapping.m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

	return Error::kNone;
}

void HiZ::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_hiZRt = rgraph.newRenderTarget(m_hiZRtDescr);
	TextureSubresourceInfo firstMipSubresource;

	// Clear RT
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HiZ clear");
		pass.newTextureDependency(m_runCtx.m_hiZRt, TextureUsageBit::kImageComputeWrite, firstMipSubresource);

		pass.setWork([this](RenderPassWorkContext& rctx) {
			CommandBufferPtr& cmdb = rctx.m_commandBuffer;

			cmdb->bindShaderProgram(m_clearHiZ.m_grProg);

			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 0, m_runCtx.m_hiZRt, firstMipSubresource);

			UVec4 clearColor(0u);
			cmdb->setPushConstants(&clearColor, sizeof(clearColor));

			dispatchPPCompute(cmdb, 8, 8, 1, m_hiZRtDescr.m_width, m_hiZRtDescr.m_height, 1);
		});
	}

	// Reproject
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HiZ reprojection");
		pass.newTextureDependency(m_runCtx.m_hiZRt, TextureUsageBit::kImageComputeWrite, firstMipSubresource);
		pass.newTextureDependency(getRenderer().getGBuffer().getPreviousFrameDepthRt(),
								  TextureUsageBit::kSampledCompute);

		pass.setWork([this, &ctx](RenderPassWorkContext& rctx) {
			CommandBufferPtr& cmdb = rctx.m_commandBuffer;

			cmdb->bindShaderProgram(m_reproj.m_grProg);

			rctx.bindTexture(0, 0, getRenderer().getGBuffer().getPreviousFrameDepthRt(),
							 TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 1, m_runCtx.m_hiZRt, firstMipSubresource);

			cmdb->setPushConstants(&ctx.m_matrices.m_reprojection, sizeof(Mat4));

			dispatchPPCompute(cmdb, 8, 8, 1, getRenderer().getInternalResolution().x(),
							  getRenderer().getInternalResolution().y(), 1);
		});
	}

	// Mipmap
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HiZ mip gen");

		pass.newTextureDependency(m_runCtx.m_hiZRt, TextureUsageBit::kSampledCompute, firstMipSubresource);

		for(U32 mip = 1; mip < m_hiZRtDescr.m_mipmapCount; ++mip)
		{
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = mip;
			pass.newTextureDependency(m_runCtx.m_hiZRt, TextureUsageBit::kImageComputeWrite, subresource);
		}

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

			TextureSubresourceInfo firstMipSubresource;
			const U32 mipsToCompute = m_hiZRtDescr.m_mipmapCount - 1;

			// Zero the counter buffer once before everything else
			if(!m_mipmapping.m_counterBufferZeroed) [[unlikely]]
			{
				m_mipmapping.m_counterBufferZeroed = true;

				cmdb->fillBuffer(m_mipmapping.m_counterBuffer, 0, kMaxPtrSize, 0);

				const BufferBarrierInfo barrier = {m_mipmapping.m_counterBuffer.get(),
												   BufferUsageBit::kTransferDestination,
												   BufferUsageBit::kStorageComputeWrite, 0, kMaxPtrSize};
				cmdb->setPipelineBarrier({}, {&barrier, 1}, {});
			}

			cmdb->bindShaderProgram(m_mipmapping.m_grProg);

			varAU2(dispatchThreadGroupCountXY);
			varAU2(workGroupOffset); // needed if Left and Top are not 0,0
			varAU2(numWorkGroupsAndMips);
			varAU4(rectInfo) = initAU4(0, 0, m_hiZRtDescr.m_width, m_hiZRtDescr.m_height);
			SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, mipsToCompute);

			struct Uniforms
			{
				U32 m_threadGroupCount;
				U32 m_mipmapCount;
				U32 m_padding0;
				U32 m_padding1;
			} pc;

			pc.m_threadGroupCount = numWorkGroupsAndMips[0];
			pc.m_mipmapCount = numWorkGroupsAndMips[1];

			cmdb->setPushConstants(&pc, sizeof(pc));

			constexpr U32 maxMipsSpdCanProduce = 12;
			for(U32 mip = 0; mip < maxMipsSpdCanProduce; ++mip)
			{
				TextureSubresourceInfo subresource;
				if(mip < mipsToCompute)
				{
					subresource.m_firstMipmap = mip + 1;
				}
				else
				{
					subresource.m_firstMipmap = 1;
				}

				rgraphCtx.bindImage(0, 0, m_runCtx.m_hiZRt, subresource, mip);
			}

			if(mipsToCompute >= 5)
			{
				TextureSubresourceInfo subresource;
				subresource.m_firstMipmap = 4;
				rgraphCtx.bindImage(0, 1, m_runCtx.m_hiZRt, subresource);
			}
			else
			{
				// Bind something random that is not the 1st mip
				TextureSubresourceInfo subresource;
				subresource.m_firstMipmap = 1;
				rgraphCtx.bindImage(0, 1, m_runCtx.m_hiZRt, subresource);
			}

			cmdb->bindStorageBuffer(0, 2, m_mipmapping.m_counterBuffer, 0, kMaxPtrSize);

			rgraphCtx.bindTexture(0, 3, m_runCtx.m_hiZRt, firstMipSubresource);

			cmdb->dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
		});
	}
}

} // end namespace anki
