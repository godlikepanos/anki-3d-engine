// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
}

Error DepthDownscale::initInternal(const ConfigSet&)
{
	const U width = m_r->getWidth() / 2;
	const U height = m_r->getHeight() / 2;

	// Create RT descrs
	m_depthRtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		"Half depth");
	m_depthRtDescr.bake();

	m_hizRtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		Format::R32_SFLOAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT
			| TextureUsageBit::SAMPLED_COMPUTE,
		"HiZ");
	m_hizRtDescr.m_mipmapCount = HIERARCHICAL_Z_MIPMAP_COUNT;
	m_hizRtDescr.bake();

	// Create FB descr
	m_passes[0].m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_passes[0].m_fbDescr.m_colorAttachmentCount = 1;
	m_passes[0].m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_passes[0].m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_passes[0].m_fbDescr.bake();

	for(U i = 1; i < HIERARCHICAL_Z_MIPMAP_COUNT; ++i)
	{
		m_passes[i].m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_passes[i].m_fbDescr.m_colorAttachments[0].m_surface.m_level = i;
		m_passes[i].m_fbDescr.m_colorAttachmentCount = 1;
		m_passes[i].m_fbDescr.bake();
	}

	// Progs
	ANKI_CHECK(getResourceManager().loadResource("programs/DepthDownscale.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<2> mutations(m_prog);
	mutations.add("TYPE", 0).add("SAMPLE_RESOLVE_TYPE", 1);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), variant);
	m_passes[0].m_grProg = variant->getProgram();

	for(U i = 1; i < HIERARCHICAL_Z_MIPMAP_COUNT; ++i)
	{
		mutations[0].m_value = 1;

		m_prog->getOrCreateVariant(mutations.get(), variant);
		m_passes[i].m_grProg = variant->getProgram();
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

void DepthDownscale::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_pass = 0;

	static const Array<CString, 5> passNames = {{"HiZ #0", "HiZ #1", "HiZ #2", "HiZ #3", "HiZ #4"}};

	// Create render targets
	m_runCtx.m_halfDepthRt = rgraph.newRenderTarget(m_depthRtDescr);
	m_runCtx.m_hizRt = rgraph.newRenderTarget(m_hizRtDescr);

	// First render pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[0]);

		pass.setFramebufferInfo(m_passes[0].m_fbDescr, {{m_runCtx.m_hizRt}}, m_runCtx.m_halfDepthRt);
		pass.setWork(runCallback, this, 0);

		TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::DEPTH); // First mip

		pass.newConsumer({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT, subresource});
		pass.newConsumer({m_runCtx.m_halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});

		pass.newProducer({m_runCtx.m_halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		pass.newProducer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Rest of the passes
	for(U i = 1; i < HIERARCHICAL_Z_MIPMAP_COUNT; ++i)
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[i]);

		pass.setFramebufferInfo(m_passes[i].m_fbDescr, {{m_runCtx.m_hizRt}}, {});
		pass.setWork(runCallback, this, 0);

		TextureSubresourceInfo subresourceRead, subresourceWrite;
		subresourceRead.m_firstMipmap = i - 1;
		subresourceWrite.m_firstMipmap = i;

		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::SAMPLED_FRAGMENT, subresourceRead});
		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresourceWrite});

		pass.newProducer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresourceWrite});
	}
}

void DepthDownscale::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U passIdx = m_runCtx.m_pass++;

	cmdb->setViewport(0, 0, m_r->getWidth() >> (passIdx + 1), m_r->getHeight() >> (passIdx + 1));

	if(passIdx == 0)
	{
		rgraphCtx.bindTextureAndSampler(0,
			0,
			m_r->getGBuffer().getDepthRt(),
			TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
			m_r->getNearestSampler());

		cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	}
	else
	{
		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx - 1;

		rgraphCtx.bindTextureAndSampler(0, 0, m_runCtx.m_hizRt, sampleSubresource, m_r->getNearestSampler());
	}

	cmdb->bindShaderProgram(m_passes[passIdx].m_grProg);

	drawQuad(cmdb);

	// Restore state
	if(passIdx == 0)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}
}

} // end namespace anki
