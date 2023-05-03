// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Hzb.h>
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

Error Hzb::init()
{
	registerDebugRenderTarget("Hzb");

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/HzbReprojection.ankiprogbin", m_reproj.m_prog));

	const ShaderProgramResourceVariant* variant;
	m_reproj.m_prog->getOrCreateVariant(variant);
	m_reproj.m_grProg.reset(&variant->getProgram());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ClearTextureCompute.ankiprogbin", m_clearHzb.m_prog));
	ShaderProgramResourceVariantInitInfo variantInit(m_clearHzb.m_prog);
	variantInit.addMutation("TEXTURE_DIMENSIONS", 2);
	variantInit.addMutation("COMPONENT_TYPE", 1);
	m_clearHzb.m_prog->getOrCreateVariant(variantInit, variant);
	m_clearHzb.m_grProg.reset(&variant->getProgram());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/HzbGenPyramid.ankiprogbin", m_mipmapping.m_prog));
	m_mipmapping.m_prog->getOrCreateVariant(variant);
	m_mipmapping.m_grProg.reset(&variant->getProgram());

	m_hzbRtDescr = getRenderer().create2DRenderTargetDescription(ConfigSet::getSingleton().getRHiZWidth(), ConfigSet::getSingleton().getRHiZHeight(),
																 Format::kR32_Uint, "Hzb U32");
	m_hzbRtDescr.m_mipmapCount = U8(computeMaxMipmapCount2d(m_hzbRtDescr.m_width, m_hzbRtDescr.m_height, 1));
	m_hzbRtDescr.bake();

	BufferInitInfo buffInit("HiZCounterBuffer");
	buffInit.m_size = sizeof(U32);
	buffInit.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kTransferDestination;
	m_mipmapping.m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

	return Error::kNone;
}

void Hzb::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_hzbRt = rgraph.newRenderTarget(m_hzbRtDescr);
	TextureSubresourceInfo firstMipSubresource;

	// Clear RT
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Hzb clear");
		pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kImageComputeWrite, firstMipSubresource);

		pass.setWork([this](RenderPassWorkContext& rctx) {
			CommandBuffer& cmdb = *rctx.m_commandBuffer;

			cmdb.bindShaderProgram(m_clearHzb.m_grProg.get());

			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 0, m_runCtx.m_hzbRt, firstMipSubresource);

			UVec4 clearColor(0u);
			cmdb.setPushConstants(&clearColor, sizeof(clearColor));

			dispatchPPCompute(cmdb, 8, 8, 1, m_hzbRtDescr.m_width, m_hzbRtDescr.m_height, 1);
		});
	}

	// Reproject
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Hzb reprojection");
		pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kImageComputeWrite, firstMipSubresource);
		pass.newTextureDependency(getRenderer().getGBuffer().getPreviousFrameDepthRt(), TextureUsageBit::kSampledCompute);

		pass.setWork([this, &ctx](RenderPassWorkContext& rctx) {
			CommandBuffer& cmdb = *rctx.m_commandBuffer;

			cmdb.bindShaderProgram(m_reproj.m_grProg.get());

			rctx.bindTexture(0, 0, getRenderer().getGBuffer().getPreviousFrameDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 1, m_runCtx.m_hzbRt, firstMipSubresource);

			cmdb.setPushConstants(&ctx.m_matrices.m_reprojection, sizeof(Mat4));

			dispatchPPCompute(cmdb, 8, 8, 1, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), 1);
		});
	}

	// Mipmap
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Hzb mip gen");

		pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kSampledCompute, firstMipSubresource);

		for(U32 mip = 1; mip < m_hzbRtDescr.m_mipmapCount; ++mip)
		{
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = mip;
			pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kImageComputeWrite, subresource);
		}

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			TextureSubresourceInfo firstMipSubresource;
			const U32 mipsToCompute = m_hzbRtDescr.m_mipmapCount - 1;

			// Zero the counter buffer once before everything else
			if(!m_mipmapping.m_counterBufferZeroed) [[unlikely]]
			{
				m_mipmapping.m_counterBufferZeroed = true;

				cmdb.fillBuffer(m_mipmapping.m_counterBuffer.get(), 0, kMaxPtrSize, 0);

				const BufferBarrierInfo barrier = {m_mipmapping.m_counterBuffer.get(), BufferUsageBit::kTransferDestination,
												   BufferUsageBit::kStorageComputeWrite, 0, kMaxPtrSize};
				cmdb.setPipelineBarrier({}, {&barrier, 1}, {});
			}

			cmdb.bindShaderProgram(m_mipmapping.m_grProg.get());

			varAU2(dispatchThreadGroupCountXY);
			varAU2(workGroupOffset); // needed if Left and Top are not 0,0
			varAU2(numWorkGroupsAndMips);
			varAU4(rectInfo) = initAU4(0, 0, m_hzbRtDescr.m_width, m_hzbRtDescr.m_height);
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

			cmdb.setPushConstants(&pc, sizeof(pc));

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

				rgraphCtx.bindImage(0, 0, m_runCtx.m_hzbRt, subresource, mip);
			}

			cmdb.bindStorageBuffer(0, 1, m_mipmapping.m_counterBuffer.get(), 0, kMaxPtrSize);
			rgraphCtx.bindTexture(0, 2, m_runCtx.m_hzbRt, firstMipSubresource);

			cmdb.dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
		});
	}
}

} // end namespace anki
