// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-function"
#	pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
#define A_CPU
#include <ThirdParty/FidelityFX/ffx_a.h>
#include <ThirdParty/FidelityFX/ffx_spd.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki {

DepthDownscale::~DepthDownscale()
{
	if(m_clientBufferAddr)
	{
		m_clientBuffer->unmap();
	}
}

Error DepthDownscale::initInternal()
{
	const U32 width = m_r->getInternalResolution().x() >> 1;
	const U32 height = m_r->getInternalResolution().y() >> 1;

	m_mipCount = computeMaxMipmapCount2d(width, height, HIERARCHICAL_Z_MIN_HEIGHT);

	m_lastMipSize.x() = width >> (m_mipCount - 1);
	m_lastMipSize.y() = height >> (m_mipCount - 1);

	ANKI_R_LOGV("Initializing HiZ. Mip count %u, last mip size %ux%u", m_mipCount, m_lastMipSize.x(),
				m_lastMipSize.y());

	const Bool supportsReductionSampler = getGrManager().getDeviceCapabilities().m_samplingFilterMinMax;

	// Create RT descr
	{
		TextureInitInfo texInit = m_r->create2DRenderTargetInitInfo(
			width, height, Format::R32_SFLOAT, TextureUsageBit::ALL_SAMPLED | TextureUsageBit::IMAGE_COMPUTE_WRITE,
			"HiZ");
		texInit.m_mipmapCount = U8(m_mipCount);
		texInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_hizTex = m_r->createAndClearRenderTarget(texInit);
	}

	// Progs
	{
		ANKI_CHECK(getResourceManager().loadResource("Shaders/DepthDownscale.ankiprog", m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addMutation("SAMPLE_RESOLVE_TYPE", 2);
		variantInitInfo.addMutation("WAVE_OPERATIONS", 0);
		variantInitInfo.addMutation("REDUCTION_SAMPLER", supportsReductionSampler);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProg = variant->getProgram();
	}

	// Reduction sampler
	if(supportsReductionSampler)
	{
		SamplerInitInfo sinit("HiZReduction");
		sinit.m_addressing = SamplingAddressing::CLAMP;
		sinit.m_mipmapFilter = SamplingFilter::MAX;
		sinit.m_minMagFilter = SamplingFilter::MAX;
		m_reductionSampler = getGrManager().newSampler(sinit);
	}

	// Counter buffer
	{
		BufferInitInfo buffInit("HiZCounterBuffer");
		buffInit.m_size = sizeof(U32);
		buffInit.m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::TRANSFER_DESTINATION;
		m_counterBuffer = getGrManager().newBuffer(buffInit);
	}

	// Copy to buffer
	{
		// Create buffer
		BufferInitInfo buffInit("HiZ Client");
		buffInit.m_mapAccess = BufferMapAccessBit::READ;
		buffInit.m_size = PtrSize(m_lastMipSize.y()) * PtrSize(m_lastMipSize.x()) * sizeof(F32);
		buffInit.m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE;
		m_clientBuffer = getGrManager().newBuffer(buffInit);

		m_clientBufferAddr = m_clientBuffer->map(0, buffInit.m_size, BufferMapAccessBit::READ);

		// Fill the buffer with 1.0f
		for(U32 i = 0; i < m_lastMipSize.x() * m_lastMipSize.y(); ++i)
		{
			static_cast<F32*>(m_clientBufferAddr)[i] = 1.0f;
		}
	}

	return Error::NONE;
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

void DepthDownscale::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import RT
	if(m_hizTexImportedOnce)
	{
		m_runCtx.m_hizRt = rgraph.importRenderTarget(m_hizTex);
	}
	else
	{
		m_runCtx.m_hizRt = rgraph.importRenderTarget(m_hizTex, TextureUsageBit::SAMPLED_FRAGMENT);
		m_hizTexImportedOnce = true;
	}
}

void DepthDownscale::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HiZ");

	pass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE,
											TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)));

	for(U32 mip = 0; mip < m_mipCount; ++mip)
	{
		TextureSubresourceInfo subresource;
		subresource.m_firstMipmap = mip;
		pass.newDependency(RenderPassDependency(m_runCtx.m_hizRt, TextureUsageBit::IMAGE_COMPUTE_WRITE, subresource));
	}

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		// Zero the counter buffer before everything else
		if(ANKI_UNLIKELY(!m_counterBufferZeroed))
		{
			m_counterBufferZeroed = true;

			cmdb->fillBuffer(m_counterBuffer, 0, MAX_PTR_SIZE, 0);

			cmdb->setBufferBarrier(m_counterBuffer, BufferUsageBit::TRANSFER_DESTINATION,
								   BufferUsageBit::STORAGE_COMPUTE_WRITE, 0, MAX_PTR_SIZE);
		}

		cmdb->bindShaderProgram(m_grProg);

		varAU2(dispatchThreadGroupCountXY);
		varAU2(workGroupOffset); // needed if Left and Top are not 0,0
		varAU2(numWorkGroupsAndMips);
		varAU4(rectInfo) = initAU4(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
		SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo);
		SpdSetup(dispatchThreadGroupCountXY, workGroupOffset, numWorkGroupsAndMips, rectInfo, m_mipCount);

		class PC
		{
		public:
			U32 m_workgroupCount;
			U32 m_mipmapCount;
			Vec2 m_srcTexSizeOverOne;
			U32 m_lastMipWidth;
			U32 m_padding[3u];
		} pc;
		pc.m_workgroupCount = numWorkGroupsAndMips[0];
		pc.m_mipmapCount = numWorkGroupsAndMips[1];
		pc.m_srcTexSizeOverOne = 1.0f / Vec2(m_r->getInternalResolution());
		pc.m_lastMipWidth = m_lastMipSize.x();

		cmdb->setPushConstants(&pc, sizeof(pc));

		constexpr U32 maxMipsSpdCanProduce = 12;
		for(U32 mip = 0; mip < maxMipsSpdCanProduce; ++mip)
		{
			TextureSubresourceInfo subresource;
			if(mip < m_mipCount)
			{
				subresource.m_firstMipmap = mip;
			}
			else
			{
				subresource.m_firstMipmap = 0;
			}

			rgraphCtx.bindImage(0, 0, m_runCtx.m_hizRt, subresource, mip);
		}

		if(m_mipCount >= 5)
		{
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = 4;
			rgraphCtx.bindImage(0, 1, m_runCtx.m_hizRt, subresource);
		}
		else
		{
			// Bind something random
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = 0;
			rgraphCtx.bindImage(0, 1, m_runCtx.m_hizRt, subresource);
		}

		cmdb->bindStorageBuffer(0, 2, m_counterBuffer, 0, MAX_PTR_SIZE);
		cmdb->bindStorageBuffer(0, 3, m_clientBuffer, 0, MAX_PTR_SIZE);

		cmdb->bindSampler(0, 4, (doesSamplerReduction()) ? m_reductionSampler : m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindTexture(0, 5, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

		cmdb->dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
	});
}

} // end namespace anki
