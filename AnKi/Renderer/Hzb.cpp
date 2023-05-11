// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Hzb.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

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
	ShaderProgramResourceVariantInitInfo variantInit(m_reproj.m_prog);
	for(U32 i = 0; i < m_reproj.m_grProgs.getSize(); ++i)
	{
		variantInit.addMutation("SHADOW_TEXTURE_COUNT", i);
		m_reproj.m_prog->getOrCreateVariant(variantInit, variant);
		m_reproj.m_grProgs[i].reset(&variant->getProgram());
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/ClearTextureCompute.ankiprogbin", m_clearHzb.m_prog));
	ShaderProgramResourceVariantInitInfo variantInit2(m_clearHzb.m_prog);
	variantInit2.addMutation("TEXTURE_DIMENSIONS", 2);
	variantInit2.addMutation("COMPONENT_TYPE", 1);
	m_clearHzb.m_prog->getOrCreateVariant(variantInit2, variant);
	m_clearHzb.m_grProg.reset(&variant->getProgram());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/HzbGenPyramid.ankiprogbin", m_mipmapping.m_prog));
	m_mipmapping.m_prog->getOrCreateVariant(variant);
	m_mipmapping.m_grProg.reset(&variant->getProgram());

	m_hzbRtDescr = getRenderer().create2DRenderTargetDescription(ConfigSet::getSingleton().getRHzbWidth(), ConfigSet::getSingleton().getRHzbHeight(),
																 Format::kR32_Uint, "HZB U32");
	m_hzbRtDescr.m_mipmapCount = U8(computeMaxMipmapCount2d(m_hzbRtDescr.m_width, m_hzbRtDescr.m_height, 1));
	m_hzbRtDescr.bake();

	for(U32 i = 0; i < kMaxShadowCascades; ++i)
	{
		RendererString name;
		name.sprintf("Shadow HZB U32 #%u", i);
		m_hzbShadowRtDescrs[i] = getRenderer().create2DRenderTargetDescription(
			ConfigSet::getSingleton().getRHzbShadowSize(), ConfigSet::getSingleton().getRHzbShadowSize(), Format::kR32_Uint, name);
		m_hzbShadowRtDescrs[i].m_mipmapCount = U8(computeMaxMipmapCount2d(m_hzbShadowRtDescrs[i].m_width, m_hzbShadowRtDescrs[i].m_height, 1));
		m_hzbShadowRtDescrs[i].bake();
	}

	BufferInitInfo buffInit("HiZCounterBuffer");
	buffInit.m_size = sizeof(U32);
	buffInit.m_usage = BufferUsageBit::kStorageComputeWrite | BufferUsageBit::kTransferDestination;
	m_mipmapping.m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

	return Error::kNone;
}

void Hzb::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	const U32 cascadeCount = ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount;
	TextureSubresourceInfo firstMipSubresource;

	// Create RTs
	m_runCtx.m_hzbRt = rgraph.newRenderTarget(m_hzbRtDescr);
	for(U32 i = 0; i < cascadeCount; ++i)
	{
		m_runCtx.m_hzbShadowRts[i] = rgraph.newRenderTarget(m_hzbShadowRtDescrs[i]);
	}

	// Clear main RT
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HZB clear");
		pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kImageComputeWrite, firstMipSubresource);

		pass.setWork([this](RenderPassWorkContext& rctx) {
			CommandBuffer& cmdb = *rctx.m_commandBuffer;

			cmdb.bindShaderProgram(m_clearHzb.m_grProg.get());

			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 0, m_runCtx.m_hzbRt, firstMipSubresource);

			// See the comments in the class on what this -0 means
			const F32 negativeZero = -0.0f;
			U32 negativeZerou;
			memcpy(&negativeZerou, &negativeZero, sizeof(U32));
			ANKI_ASSERT(negativeZerou > 0);
			UVec4 clearColor(negativeZerou);
			cmdb.setPushConstants(&clearColor, sizeof(clearColor));

			dispatchPPCompute(cmdb, 8, 8, m_hzbRtDescr.m_width, m_hzbRtDescr.m_height);
		});
	}

	// Clear SM RTs
	for(U32 i = 0; i < cascadeCount; ++i)
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Shadow HZB clear");
		pass.newTextureDependency(m_runCtx.m_hzbShadowRts[i], TextureUsageBit::kImageComputeWrite, firstMipSubresource);

		pass.setWork([this, i](RenderPassWorkContext& rctx) {
			CommandBuffer& cmdb = *rctx.m_commandBuffer;

			cmdb.bindShaderProgram(m_clearHzb.m_grProg.get());

			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 0, m_runCtx.m_hzbShadowRts[i], firstMipSubresource);

			UVec4 clearColor(1u);
			cmdb.setPushConstants(&clearColor, sizeof(clearColor));

			dispatchPPCompute(cmdb, 8, 8, m_hzbShadowRtDescrs[i].m_width, m_hzbShadowRtDescrs[i].m_height);
		});
	}

	// Reproject
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HZB reprojection");

		pass.newTextureDependency(m_runCtx.m_hzbRt, TextureUsageBit::kImageComputeWrite, firstMipSubresource);
		for(U32 i = 0; i < cascadeCount; ++i)
		{
			pass.newTextureDependency(m_runCtx.m_hzbShadowRts[i], TextureUsageBit::kImageComputeWrite, firstMipSubresource);
		}
		pass.newTextureDependency(getRenderer().getGBuffer().getPreviousFrameDepthRt(), TextureUsageBit::kSampledCompute);

		pass.setWork([this, &ctx](RenderPassWorkContext& rctx) {
			const U32 cascadeCount = ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount;
			CommandBuffer& cmdb = *rctx.m_commandBuffer;

			cmdb.bindShaderProgram(m_reproj.m_grProgs[cascadeCount].get());

			rctx.bindTexture(0, 0, getRenderer().getGBuffer().getPreviousFrameDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			TextureSubresourceInfo firstMipSubresource;
			rctx.bindImage(0, 1, m_runCtx.m_hzbRt, firstMipSubresource);

			HzbUniforms* unis = allocateAndBindUniforms<HzbUniforms*>(sizeof(*unis), cmdb, 0, 3);
			unis->m_reprojectionMatrix = ctx.m_matrices.m_reprojection;
			unis->m_invertedViewProjectionMatrix = ctx.m_matrices.m_invertedViewProjection;
			for(U32 i = 0; i < ctx.m_renderQueue->m_directionalLight.m_shadowCascadeCount; ++i)
			{
				unis->m_shadowCascadeViewProjectionMatrices[i] = ctx.m_renderQueue->m_directionalLight.m_viewProjectionMatrices[i];
			}

			for(U32 i = 0; i < cascadeCount; ++i)
			{
				rctx.bindImage(0, 2, m_runCtx.m_hzbShadowRts[i], firstMipSubresource, i);
			}

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}

	// Mipmap
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HZB mip gen");

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
