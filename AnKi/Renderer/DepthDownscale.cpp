// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Core/ConfigSet.h>

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

	m_fbDescrs.destroy(getAllocator());
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

	const Bool preferCompute = getConfig().getRPreferCompute();
	const Bool supportsReductionSampler = getGrManager().getDeviceCapabilities().m_samplingFilterMinMax;

	// Create RT descr
	{
		TextureUsageBit usage = TextureUsageBit::ALL_SAMPLED;
		if(preferCompute)
		{
			usage |= TextureUsageBit::IMAGE_COMPUTE_WRITE;
		}
		else
		{
			usage |= TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
		}

		TextureInitInfo texInit = m_r->create2DRenderTargetInitInfo(width, height, Format::R32_SFLOAT, usage, "HiZ");
		texInit.m_mipmapCount = U8(m_mipCount);
		texInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		m_hizTex = m_r->createAndClearRenderTarget(texInit);
	}

	// Progs
	if(preferCompute)
	{
		ANKI_CHECK(getResourceManager().loadResource("Shaders/DepthDownscaleCompute.ankiprog", m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addMutation("WAVE_OPERATIONS", 0);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProg = variant->getProgram();
	}
	else
	{
		ANKI_CHECK(getResourceManager().loadResource("Shaders/DepthDownscaleRaster.ankiprog", m_prog));

		ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
		variantInitInfo.addMutation("REDUCTION_SAMPLER", supportsReductionSampler);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProg = variant->getProgram();

		// 1st mip prog
		variantInitInfo.addMutation("REDUCTION_SAMPLER", 0);
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_firstMipGrProg = variant->getProgram();
	}

	// Counter buffer
	if(preferCompute)
	{
		BufferInitInfo buffInit("HiZCounterBuffer");
		buffInit.m_size = sizeof(U32);
		buffInit.m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::TRANSFER_DESTINATION;
		m_counterBuffer = getGrManager().newBuffer(buffInit);
	}

	// Client buffer
	{
		// Create buffer
		BufferInitInfo buffInit("HiZ Client");
		buffInit.m_mapAccess = BufferMapAccessBit::READ;
		buffInit.m_size = PtrSize(m_lastMipSize.y()) * PtrSize(m_lastMipSize.x()) * sizeof(F32);
		buffInit.m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE | BufferUsageBit::STORAGE_FRAGMENT_WRITE;
		m_clientBuffer = getGrManager().newBuffer(buffInit);

		m_clientBufferAddr = m_clientBuffer->map(0, buffInit.m_size, BufferMapAccessBit::READ);

		// Fill the buffer with 1.0f
		for(U32 i = 0; i < m_lastMipSize.x() * m_lastMipSize.y(); ++i)
		{
			static_cast<F32*>(m_clientBufferAddr)[i] = 1.0f;
		}
	}

	// Reduction sampler
	if(!preferCompute && supportsReductionSampler)
	{
		SamplerInitInfo sinit("HiZReduction");
		sinit.m_addressing = SamplingAddressing::CLAMP;
		sinit.m_mipmapFilter = SamplingFilter::MAX;
		sinit.m_minMagFilter = SamplingFilter::MAX;
		m_reductionSampler = getGrManager().newSampler(sinit);
	}

	if(!preferCompute)
	{
		m_fbDescrs.create(getAllocator(), m_mipCount);
		for(U32 mip = 0; mip < m_mipCount; ++mip)
		{
			FramebufferDescription& fbDescr = m_fbDescrs[mip];
			fbDescr.m_colorAttachmentCount = 1;
			fbDescr.m_colorAttachments[0].m_surface.m_level = mip;
			fbDescr.bake();
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

	if(getConfig().getRPreferCompute())
	{
		// Do it with compute

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HiZ");

		pass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE,
												TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)));

		for(U32 mip = 0; mip < m_mipCount; ++mip)
		{
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = mip;
			pass.newDependency(
				RenderPassDependency(m_runCtx.m_hizRt, TextureUsageBit::IMAGE_COMPUTE_WRITE, subresource));
		}

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			runCompute(rgraphCtx);
		});
	}
	else
	{
		// Do it with raster

		for(U32 mip = 0; mip < m_mipCount; ++mip)
		{
			static const Array<CString, 8> passNames = {"HiZ #1", "HiZ #2", "HiZ #3", "HiZ #4",
														"HiZ #5", "HiZ #6", "HiZ #7", "HiZ #8"};
			GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[mip]);
			pass.setFramebufferInfo(m_fbDescrs[mip], {m_runCtx.m_hizRt});

			if(mip == 0)
			{
				pass.newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(),
														TextureUsageBit::SAMPLED_FRAGMENT,
														TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)));
			}
			else
			{
				TextureSurfaceInfo subresource;
				subresource.m_level = mip - 1;
				pass.newDependency(
					RenderPassDependency(m_runCtx.m_hizRt, TextureUsageBit::SAMPLED_FRAGMENT, subresource));
			}

			TextureSurfaceInfo subresource;
			subresource.m_level = mip;
			pass.newDependency(
				RenderPassDependency(m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource));

			pass.setWork([this, mip](RenderPassWorkContext& rgraphCtx) {
				runGraphics(mip, rgraphCtx);
			});
		}
	}
}

void DepthDownscale::runCompute(RenderPassWorkContext& rgraphCtx)
{
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

	cmdb->bindSampler(0, 4, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindTexture(0, 5, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	cmdb->dispatchCompute(dispatchThreadGroupCountXY[0], dispatchThreadGroupCountXY[1], 1);
}

void DepthDownscale::runGraphics(U32 mip, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	if(mip == 0)
	{
		rgraphCtx.bindTexture(0, 0, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

		cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

		cmdb->bindShaderProgram(m_firstMipGrProg);
	}
	else
	{
		TextureSubresourceInfo subresource;
		subresource.m_firstMipmap = mip - 1;
		rgraphCtx.bindTexture(0, 0, m_runCtx.m_hizRt, subresource);

		if(m_reductionSampler.isCreated())
		{
			cmdb->bindSampler(0, 1, m_reductionSampler);
		}
		else
		{
			cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);
		}

		cmdb->bindShaderProgram(m_grProg);
	}

	cmdb->bindStorageBuffer(0, 2, m_clientBuffer, 0, MAX_PTR_SIZE);

	UVec4 pc((mip != m_mipCount - 1) ? 0 : m_lastMipSize.x());
	cmdb->setPushConstants(&pc, sizeof(pc));

	const UVec2 size = (m_r->getInternalResolution() / 2) >> mip;
	cmdb->setViewport(0, 0, size.x(), size.y());
	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
}

} // end namespace anki
