// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/HzbGenerator.h>
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

//   7 +----+ 6
//    /|   /|
// 3 +----+2|
//   | *--| + 5
//   |/4  |/
// 0 +----+  1
static constexpr U16 kBoxIndices[] = {1, 2, 5, 2, 6, 5, 0, 4, 3, 4, 7, 3, 3, 7, 2, 7, 6, 2, 0, 1, 4, 1, 5, 4, 0, 3, 1, 3, 2, 1, 4, 5, 7, 5, 6, 7};

Error HzbGenerator::init()
{
	if(GrManager::getSingleton().getDeviceCapabilities().m_samplingFilterMinMax)
	{
		SamplerInitInfo sinit("HzbReductionMax");
		sinit.m_addressing = SamplingAddressing::kClamp;
		sinit.m_mipmapFilter = SamplingFilter::kMax;
		sinit.m_minMagFilter = SamplingFilter::kMax;
		m_maxSampler = GrManager::getSingleton().newSampler(sinit);
	}

	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/HzbGenPyramid.ankiprogbin", m_genPyramidProg));

		ShaderProgramResourceVariantInitInfo variantInit(m_genPyramidProg);
		variantInit.addMutation("REDUCTION_TYPE", 1);
		variantInit.addMutation("MIN_MAX_SAMPLER", m_maxSampler.isCreated());
		const ShaderProgramResourceVariant* variant;
		m_genPyramidProg->getOrCreateVariant(variantInit, variant);
		m_genPyramidGrProg.reset(&variant->getProgram());
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/HzbMaxDepth.ankiprogbin", m_maxDepthProg, m_maxDepthGrProg));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/HzbMaxDepthProject.ankiprogbin", m_maxBoxProg, m_maxBoxGrProg));

	m_counterBufferElementSize = max<U32>(sizeof(U32), GrManager::getSingleton().getDeviceCapabilities().m_uavBufferBindOffsetAlignment);
	BufferInitInfo buffInit("HzbCounterBuffer");
	buffInit.m_size = m_counterBufferElementSize * kCounterBufferElementCount;
	buffInit.m_usage = BufferUsageBit::kUavComputeWrite | BufferUsageBit::kTransferDestination;
	m_counterBuffer = GrManager::getSingleton().newBuffer(buffInit);

	// Zero counter buffer
	{
		CommandBufferInitInfo cmdbInit;
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		cmdb->fillBuffer(m_counterBuffer.get(), 0, kMaxPtrSize, 0);

		FencePtr fence;
		cmdb->flush({}, &fence);

		fence->clientWait(6.0_sec);
	}

	buffInit = BufferInitInfo("HzbBoxIndices");
	buffInit.m_size = sizeof(kBoxIndices);
	buffInit.m_usage = BufferUsageBit::kIndex;
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	m_boxIndexBuffer = GrManager::getSingleton().newBuffer(buffInit);

	void* mappedMem = m_boxIndexBuffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);
	memcpy(mappedMem, kBoxIndices, sizeof(kBoxIndices));
	m_boxIndexBuffer->unmap();

	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_fbDescr.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 0.0f;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kClear;
	m_fbDescr.bake();

	return Error::kNone;
}

void HzbGenerator::populateRenderGraphInternal(ConstWeakArray<DispatchInput> dispatchInputs, U32 firstCounterBufferElement, CString customName,
											   RenderGraphDescription& rgraph) const
{
	const U32 dispatchCount = dispatchInputs.getSize();

#if ANKI_ASSERTIONS_ENABLED
	if(m_crntFrame != getRenderer().getFrameCount())
	{
		m_crntFrame = getRenderer().getFrameCount();
		m_counterBufferElementUseMask = 0;
	}

	for(U32 i = 0; i < dispatchCount; ++i)
	{
		ANKI_ASSERT(!(m_counterBufferElementUseMask & (1 << (firstCounterBufferElement + i))));
		m_counterBufferElementUseMask |= (1 << (firstCounterBufferElement + i));
	}
#endif

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass((customName.isEmpty()) ? "HZB generation" : customName);

	Array<DispatchInput, kMaxShadowCascades> dispatchInputsCopy;
	for(U32 i = 0; i < dispatchCount; ++i)
	{
		TextureSubresourceInfo firstMipSubresource;
		pass.newTextureDependency(dispatchInputs[i].m_srcDepthRt, TextureUsageBit::kSampledCompute, firstMipSubresource);
		pass.newTextureDependency(dispatchInputs[i].m_dstHzbRt, TextureUsageBit::kUavComputeWrite);

		dispatchInputsCopy[i] = dispatchInputs[i];
	}

	pass.setWork([this, dispatchInputsCopy, dispatchCount, firstCounterBufferElement](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_genPyramidGrProg.get());
		cmdb.bindSampler(0, 3, m_maxSampler.isCreated() ? m_maxSampler.get() : getRenderer().getSamplers().m_trilinearClamp.get());

		for(U32 dispatch = 0; dispatch < dispatchCount; ++dispatch)
		{
			const DispatchInput& in = dispatchInputsCopy[dispatch];

			const U32 hzbMipCount =
				min(kMaxMipsSinglePassDownsamplerCanProduce, computeMaxMipmapCount2d(in.m_dstHzbRtSize.x(), in.m_dstHzbRtSize.y()));

			const U32 mipsToCompute = hzbMipCount;

			varAU2(dispatchThreadGroupCountXY);
			varAU2(workGroupOffset); // needed if Left and Top are not 0,0
			varAU2(numWorkGroupsAndMips);
			varAU4(rectInfo) = initAU4(0, 0, in.m_dstHzbRtSize.x() * 2, in.m_dstHzbRtSize.y() * 2);
			SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, mipsToCompute);

			struct Constants
			{
				Vec2 m_invSrcTexSize;
				U32 m_threadGroupCount;
				U32 m_mipmapCount;
			} pc;

			pc.m_invSrcTexSize = 1.0f / Vec2(in.m_dstHzbRtSize * 2);
			pc.m_threadGroupCount = numWorkGroupsAndMips[0];
			pc.m_mipmapCount = numWorkGroupsAndMips[1];

			cmdb.setPushConstants(&pc, sizeof(pc));

			for(U32 mip = 0; mip < kMaxMipsSinglePassDownsamplerCanProduce; ++mip)
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

				rgraphCtx.bindUavTexture(0, 0, in.m_dstHzbRt, subresource, mip);
			}

			cmdb.bindUavBuffer(0, 1, m_counterBuffer.get(), (firstCounterBufferElement + dispatch) * m_counterBufferElementSize, sizeof(U32));
			rgraphCtx.bindTexture(0, 2, in.m_srcDepthRt, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

			cmdb.dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
		}
	});
}

void HzbGenerator::populateRenderGraph(RenderTargetHandle srcDepthRt, UVec2 srcDepthRtSize, RenderTargetHandle dstHzbRt, UVec2 dstHzbRtSize,
									   RenderGraphDescription& rgraph, CString customName) const
{
	DispatchInput in;
	in.m_dstHzbRt = dstHzbRt;
	in.m_dstHzbRtSize = dstHzbRtSize;
	in.m_srcDepthRt = srcDepthRt;
	in.m_srcDepthRtSize = srcDepthRtSize;
	populateRenderGraphInternal({&in, 1}, 0, customName, rgraph);
}

void HzbGenerator::populateRenderGraphDirectionalLight(const HzbDirectionalLightInput& in, RenderGraphDescription& rgraph) const
{
	const U32 cascadeCount = in.m_cascadeCount;
	ANKI_ASSERT(cascadeCount > 0);

	// Generate a temp RT with the max depth of each 64x64 tile of the depth buffer
	RenderTargetHandle maxDepthRt;
	constexpr U32 kTileSize = 64;
	const UVec2 maxDepthRtSize = (in.m_depthBufferRtSize + kTileSize - 1) / kTileSize;
	{
		RenderTargetDescription maxDepthRtDescr("HZB max tile depth");
		maxDepthRtDescr.m_width = maxDepthRtSize.x();
		maxDepthRtDescr.m_height = maxDepthRtSize.y();
		maxDepthRtDescr.m_format = Format::kR32_Sfloat;
		maxDepthRtDescr.bake();
		maxDepthRt = rgraph.newRenderTarget(maxDepthRtDescr);

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HZB max tile depth");

		pass.newTextureDependency(in.m_depthBufferRt, TextureUsageBit::kSampledCompute, DepthStencilAspectBit::kDepth);
		pass.newTextureDependency(maxDepthRt, TextureUsageBit::kUavComputeWrite);

		pass.setWork([this, depthBufferRt = in.m_depthBufferRt, maxDepthRt, maxDepthRtSize](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			rgraphCtx.bindTexture(0, 0, depthBufferRt, TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
			cmdb.bindSampler(0, 1, getRenderer().getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindUavTexture(0, 2, maxDepthRt);

			cmdb.bindShaderProgram(m_maxDepthGrProg.get());

			cmdb.dispatchCompute(maxDepthRtSize.x(), maxDepthRtSize.y(), 1);
		});
	}

	// Project a box for each tile on each cascade's HZB
	Array<RenderTargetHandle, kMaxShadowCascades> depthRts;
	for(U32 i = 0; i < cascadeCount; ++i)
	{
		const HzbDirectionalLightInput::Cascade& cascade = in.m_cascades[i];

		// Compute the cascade's min and max depth as seen by the camera
		F32 cascadeMinDepth, cascadeMaxDepth;
		{
			if(i > 0)
			{
				// Do the reverse of computeShadowCascadeIndex2 to find the actual distance of this cascade. computeShadowCascadeIndex2 makes the min
				// distance of a cascade to become even less. See https://www.desmos.com/calculator/g1ibye6ebg
				// F = ((x-m)/(M-m))^16 and solving for x we have the new minDist
				const F32 m = (i >= 2) ? in.m_cascades[i - 2].m_cascadeMaxDistance : 0.0f; // Prev cascade min dist
				const F32 M = in.m_cascades[i - 1].m_cascadeMaxDistance; // Prev cascade max dist
				constexpr F32 F = 0.01f; // Desired factor
				const F32 minDist = pow(F, 1.0f / 16.0f) * (M - m) + m;
				ANKI_ASSERT(minDist < M);

				Vec4 v4 = in.m_cameraProjectionMatrix * Vec4(0.0f, 0.0f, -minDist, 1.0f);
				cascadeMinDepth = saturate(v4.z() / v4.w());
			}
			else
			{
				cascadeMinDepth = 0.0f;
			}

			const F32 maxDist = cascade.m_cascadeMaxDistance;
			const Vec4 v4 = in.m_cameraProjectionMatrix * Vec4(0.0f, 0.0f, -maxDist, 1.0f);
			cascadeMaxDepth = saturate(v4.z() / v4.w());

			ANKI_ASSERT(cascadeMinDepth <= cascadeMaxDepth);
		}

		RenderTargetDescription depthRtDescr("HZB boxes depth");
		depthRtDescr.m_width = cascade.m_hzbRtSize.x() * 2;
		depthRtDescr.m_height = cascade.m_hzbRtSize.y() * 2;
		depthRtDescr.m_format = Format::kD16_Unorm;
		depthRtDescr.bake();
		depthRts[i] = rgraph.newRenderTarget(depthRtDescr);

		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("HZB boxes");

		pass.setFramebufferInfo(m_fbDescr, {}, depthRts[i]);

		pass.newTextureDependency(maxDepthRt, TextureUsageBit::kSampledFragment);
		pass.newTextureDependency(depthRts[i], TextureUsageBit::kFramebufferWrite, DepthStencilAspectBit::kDepth);

		pass.setWork([this, maxDepthRt, invViewProjMat = in.m_cameraInverseViewProjectionMatrix,
					  lightViewProjMat = cascade.m_projectionMatrix * Mat4(cascade.m_viewMatrix, Vec4(0.0f, 0.0f, 0.0f, 1.0f)),
					  viewport = cascade.m_hzbRtSize * 2, maxDepthRtSize, cascadeMinDepth, cascadeMaxDepth](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.setDepthCompareOperation(CompareOperation::kGreater);

			cmdb.setViewport(0, 0, viewport.x(), viewport.y());

			cmdb.bindShaderProgram(m_maxBoxGrProg.get());

			rgraphCtx.bindColorTexture(0, 0, maxDepthRt);

			struct Constants
			{
				Mat4 m_reprojectionMat;

				F32 m_cascadeMinDepth;
				F32 m_cascadeMaxDepth;
				F32 m_padding0;
				F32 m_padding1;
			} unis;

			unis.m_reprojectionMat = lightViewProjMat * invViewProjMat;
			unis.m_cascadeMinDepth = cascadeMinDepth;
			unis.m_cascadeMaxDepth = cascadeMaxDepth;

			cmdb.setPushConstants(&unis, sizeof(unis));

			cmdb.bindIndexBuffer(m_boxIndexBuffer.get(), 0, IndexType::kU16);

			cmdb.drawIndexed(PrimitiveTopology::kTriangles, sizeof(kBoxIndices) / sizeof(kBoxIndices[0]), maxDepthRtSize.x() * maxDepthRtSize.y());

			// Restore state
			cmdb.setDepthCompareOperation(CompareOperation::kLess);
		});
	}

	// Generate the HZBs
	Array<DispatchInput, kMaxShadowCascades> inputs;
	for(U32 i = 0; i < cascadeCount; ++i)
	{
		const HzbDirectionalLightInput::Cascade& cascade = in.m_cascades[i];

		inputs[i].m_dstHzbRt = cascade.m_hzbRt;
		inputs[i].m_dstHzbRtSize = cascade.m_hzbRtSize;
		inputs[i].m_srcDepthRt = depthRts[i];
		inputs[i].m_srcDepthRtSize = cascade.m_hzbRtSize * 2;
	}
	populateRenderGraphInternal({&inputs[0], cascadeCount}, 1, "HZB generation shadow cascades", rgraph);
}

} // end namespace anki
