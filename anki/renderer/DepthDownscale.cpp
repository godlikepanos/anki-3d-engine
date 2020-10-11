// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>

namespace anki
{

DepthDownscale::~DepthDownscale()
{
	if(m_copyToBuff.m_buffAddr)
	{
		m_copyToBuff.m_buff->unmap();
	}
}

Error DepthDownscale::initInternal(const ConfigSet&)
{
	const U32 width = m_r->getWidth() >> 1;
	const U32 height = m_r->getHeight() >> 1;

	m_mipCount = computeMaxMipmapCount2d(width, height, HIERARCHICAL_Z_MIN_HEIGHT);

	const U32 lastMipWidth = width >> (m_mipCount - 1);
	const U32 lastMipHeight = height >> (m_mipCount - 1);

	ANKI_R_LOGI("Initializing HiZ. Mip count %u, last mip size %ux%u", m_mipCount, lastMipWidth, lastMipHeight);

	// Create RT descr
	TextureInitInfo texInit = m_r->create2DRenderTargetInitInfo(
		width, height, Format::R32_SFLOAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE | TextureUsageBit::IMAGE_COMPUTE_WRITE,
		"HiZ");
	texInit.m_mipmapCount = U8(m_mipCount);
	texInit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_hizTex = m_r->createAndClearRenderTarget(texInit);

	// Progs
	ANKI_CHECK(getResourceManager().loadResource("shaders/DepthDownscale.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("SAMPLE_RESOLVE_TYPE", 2);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// Copy to buffer
	{
		m_copyToBuff.m_lastMipWidth = lastMipWidth;
		m_copyToBuff.m_lastMipHeight = lastMipHeight;

		// Create buffer
		BufferInitInfo buffInit("HiZ Client");
		buffInit.m_mapAccess = BufferMapAccessBit::READ;
		buffInit.m_size = lastMipHeight * lastMipWidth * sizeof(F32);
		buffInit.m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE;
		m_copyToBuff.m_buff = getGrManager().newBuffer(buffInit);

		m_copyToBuff.m_buffAddr = m_copyToBuff.m_buff->map(0, buffInit.m_size, BufferMapAccessBit::READ);

		// Fill the buffer with 1.0f
		for(U32 i = 0; i < lastMipHeight * lastMipWidth; ++i)
		{
			static_cast<F32*>(m_copyToBuff.m_buffAddr)[i] = 1.0f;
		}
	}

	return Error::NONE;
}

Error DepthDownscale::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing depth downscale passes");

	Error err = initInternal(cfg);
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
	m_runCtx.m_mip = 0;

	static const Array<CString, 5> passNames = {"HiZ #0", "HiZ #1", "HiZ #2", "HiZ #3", "HiZ #4"};

	// Every pass can do MIPS_WRITTEN_PER_PASS mips
	for(U32 i = 0; i < m_mipCount; i += MIPS_WRITTEN_PER_PASS)
	{
		const U mipsToFill = (i + 1 < m_mipCount) ? MIPS_WRITTEN_PER_PASS : 1;

		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass(passNames[i / MIPS_WRITTEN_PER_PASS]);

		if(i == 0)
		{
			pass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE,
								TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
		}
		else
		{
			TextureSubresourceInfo subresource;
			subresource.m_firstMipmap = i - 1;

			pass.newDependency({m_runCtx.m_hizRt, TextureUsageBit::SAMPLED_COMPUTE, subresource});
		}

		TextureSubresourceInfo subresource;
		subresource.m_firstMipmap = i;
		pass.newDependency({m_runCtx.m_hizRt, TextureUsageBit::IMAGE_COMPUTE_WRITE, subresource});

		if(mipsToFill == MIPS_WRITTEN_PER_PASS)
		{
			subresource.m_firstMipmap = i + 1;
			pass.newDependency({m_runCtx.m_hizRt, TextureUsageBit::IMAGE_COMPUTE_WRITE, subresource});
		}

		auto callback = [](RenderPassWorkContext& rgraphCtx) {
			DepthDownscale* const self = static_cast<DepthDownscale*>(rgraphCtx.m_userData);
			self->run(rgraphCtx);
		};
		pass.setWork(callback, this, 0);
	}
}

void DepthDownscale::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U32 level = m_runCtx.m_mip;
	m_runCtx.m_mip += MIPS_WRITTEN_PER_PASS;
	const U32 mipsToFill = (level + 1 < m_mipCount) ? MIPS_WRITTEN_PER_PASS : 1;
	const U32 copyToClientLevel = (level + mipsToFill == m_mipCount) ? mipsToFill - 1 : MAX_U32;

	const U32 level0Width = m_r->getWidth() >> (level + 1);
	const U32 level0Height = m_r->getHeight() >> (level + 1);
	const U32 level1Width = level0Width >> 1;
	const U32 level1Height = level0Height >> 1;

	cmdb->bindShaderProgram(m_grProg);

	// Uniforms
	struct PushConsts
	{
		UVec2 m_level0WriteImgSize;
		UVec2 m_level1WriteImgSize;
		U32 m_copyToClientLevel;
		U32 m_writeLevel1;
		U32 m_padding0;
		U32 m_padding1;
	} regs;

	regs.m_level0WriteImgSize = UVec2(level0Width, level0Height);
	regs.m_level1WriteImgSize = UVec2(level1Width, level1Height);
	regs.m_copyToClientLevel = copyToClientLevel;
	regs.m_writeLevel1 = mipsToFill == MIPS_WRITTEN_PER_PASS;
	cmdb->setPushConstants(&regs, sizeof(regs));

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	// Bind input texure
	if(level == 0)
	{
		rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	}
	else
	{
		TextureSubresourceInfo subresource;
		subresource.m_firstMipmap = level - 1;
		rgraphCtx.bindTexture(0, 1, m_runCtx.m_hizRt, subresource);
	}

	// 1st level
	TextureSubresourceInfo subresource;
	subresource.m_firstMipmap = level;
	rgraphCtx.bindImage(0, 2, m_runCtx.m_hizRt, subresource);

	// 2nd level
	subresource.m_firstMipmap = (mipsToFill == MIPS_WRITTEN_PER_PASS) ? level + 1 : level; // Bind the next or the same
	rgraphCtx.bindImage(0, 3, m_runCtx.m_hizRt, subresource);

	// Client buffer
	cmdb->bindStorageBuffer(0, 4, m_copyToBuff.m_buff, 0, m_copyToBuff.m_buff->getSize());

	// Done
	dispatchPPCompute(cmdb, 8, 8, level0Width, level0Height);
}

} // end namespace anki
