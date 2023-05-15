// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/HzbHelper.h>
#include <AnKi/Renderer/Renderer.h>

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

Error HzbHelper::init()
{
	if(GrManager::getSingleton().getDeviceCapabilities().m_samplingFilterMinMax)
	{
		SamplerInitInfo sinit("HzbReductionMax");
		sinit.m_addressing = SamplingAddressing::kClamp;
		sinit.m_mipmapFilter = SamplingFilter::kMax;
		sinit.m_minMagFilter = SamplingFilter::kMax;
		m_maxSampler = GrManager::getSingleton().newSampler(sinit);
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/HzbGenPyramid.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInit(m_prog);
	variantInit.addMutation("REDUCTION_TYPE", 1);
	variantInit.addMutation("MIN_MAX_SAMPLER", m_maxSampler.isCreated());
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInit, variant);
	m_grProg.reset(&variant->getProgram());

	BufferInitInfo buffInit("HzbCounterBuffer");
	buffInit.m_size = sizeof(U32);
	buffInit.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kTransferDestination;
	m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

	return Error::kNone;
}

void HzbHelper::populateRenderGraph(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, RenderTargetHandle dstHzbRt, UVec2 dstHzbRtSize,
									RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	TextureSubresourceInfo firstMipSubresource;

	constexpr U32 kMaxSpdMips = 12;
	const U32 hzbMipCount = min(kMaxSpdMips, computeMaxMipmapCount2d(dstHzbRtSize.x(), dstHzbRtSize.y()));

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HZB generation");

	pass.newTextureDependency(srcDepthRt, TextureUsageBit::kSampledCompute, firstMipSubresource);
	pass.newTextureDependency(dstHzbRt, TextureUsageBit::kImageComputeWrite);

	pass.setWork([this, hzbMipCount, srcDepthRt, srcDepthRtSize, dstHzbRt, dstHzbRtSize](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		const U32 mipsToCompute = hzbMipCount;

		// Zero the counter buffer once before everything else
		if(!m_counterBufferZeroed) [[unlikely]]
		{
			m_counterBufferZeroed = true;

			cmdb.fillBuffer(m_counterBuffer.get(), 0, kMaxPtrSize, 0);

			const BufferBarrierInfo barrier = {m_counterBuffer.get(), BufferUsageBit::kTransferDestination, BufferUsageBit::kStorageComputeWrite, 0,
											   kMaxPtrSize};
			cmdb.setPipelineBarrier({}, {&barrier, 1}, {});
		}

		cmdb.bindShaderProgram(m_grProg.get());

		varAU2(dispatchThreadGroupCountXY);
		varAU2(workGroupOffset); // needed if Left and Top are not 0,0
		varAU2(numWorkGroupsAndMips);
		varAU4(rectInfo) = initAU4(0, 0, dstHzbRtSize.x() * 2, dstHzbRtSize.y() * 2);
		SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, mipsToCompute);

		struct Uniforms
		{
			Vec2 m_invSrcTexSize;
			U32 m_threadGroupCount;
			U32 m_mipmapCount;
		} pc;

		pc.m_invSrcTexSize = 1.0f / Vec2(dstHzbRtSize * 2);
		pc.m_threadGroupCount = numWorkGroupsAndMips[0];
		pc.m_mipmapCount = numWorkGroupsAndMips[1];

		cmdb.setPushConstants(&pc, sizeof(pc));

		for(U32 mip = 0; mip < kMaxSpdMips; ++mip)
		{
			TextureSubresourceInfo subresource;
			if(mip < mipsToCompute)
			{
				subresource.m_firstMipmap = mip;
			}
			else
			{
				subresource.m_firstMipmap = 0; // Put something random
			}

			rgraphCtx.bindImage(0, 0, dstHzbRt, subresource, mip);
		}

		cmdb.bindStorageBuffer(0, 1, m_counterBuffer.get(), 0, kMaxPtrSize);
		rgraphCtx.bindTexture(0, 2, srcDepthRt, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		cmdb.bindSampler(0, 3, m_maxSampler.isCreated() ? m_maxSampler.get() : getRenderer().getSamplers().m_trilinearClamp.get());

		cmdb.dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
	});
}

} // end namespace anki
