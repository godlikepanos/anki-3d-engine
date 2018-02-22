// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/TemporalAA.h>

namespace anki
{

DownscaleBlur::~DownscaleBlur()
{
	m_fbDescrs.destroy(getAllocator());
}

Error DownscaleBlur::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize downscale blur");
	}

	return err;
}

Error DownscaleBlur::initInternal(const ConfigSet&)
{
	m_passCount = computeMaxMipmapCount2d(m_r->getWidth(), m_r->getHeight(), DOWNSCALE_BLUR_DOWN_TO) - 1;
	ANKI_R_LOGI("Initializing dowscale blur (passCount: %u)", U(m_passCount));

	// Create the miped texture
	TextureInitInfo texinit = m_r->create2DRenderTargetDescription(m_r->getWidth() / 2,
		m_r->getHeight() / 2,
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::SAMPLED_COMPUTE,
		"DownscaleBlur");
	texinit.m_mipmapCount = m_passCount;
	m_rtTex = m_r->createAndClearRenderTarget(texinit);

	// FB descr
	m_fbDescrs.create(getAllocator(), m_passCount);
	for(U pass = 0; pass < m_passCount; ++pass)
	{
		m_fbDescrs[pass].m_colorAttachmentCount = 1;
		m_fbDescrs[pass].m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fbDescrs[pass].m_colorAttachments[0].m_surface.m_level = pass;
		m_fbDescrs[pass].bake();
	}

	// Shader programs
	ANKI_CHECK(getResourceManager().loadResource("programs/DownscaleBlur.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void DownscaleBlur::importRenderTargets(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_rt = rgraph.importRenderTarget("Down/Blur", m_rtTex, TextureUsageBit::SAMPLED_COMPUTE);
}

void DownscaleBlur::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_crntPassIdx = 0;

	// Create passes
	static const Array<CString, 8> passNames = {{"DownBlur #0",
		"Down/Blur #1",
		"Down/Blur #2",
		"Down/Blur #3",
		"Down/Blur #4",
		"Down/Blur #5",
		"Down/Blur #6",
		"Down/Blur #7"}};
	for(U i = 0; i < m_passCount; ++i)
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[i]);
		pass.setWork(runCallback, this, 0);

		if(i > 0)
		{
			TextureSubresourceInfo sampleSubresource;
			TextureSubresourceInfo renderSubresource;

			sampleSubresource.m_firstMipmap = i - 1;
			renderSubresource.m_firstMipmap = i;

			pass.newConsumer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, renderSubresource});
			pass.newConsumer({m_runCtx.m_rt, TextureUsageBit::SAMPLED_FRAGMENT, sampleSubresource});

			pass.newProducer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, renderSubresource});
		}
		else
		{
			TextureSubresourceInfo renderSubresource;

			pass.newConsumer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, renderSubresource});
			pass.newConsumer({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

			pass.newProducer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, renderSubresource});
		}

		pass.setFramebufferInfo(m_fbDescrs[i], {{m_runCtx.m_rt}}, {});
	}
}

void DownscaleBlur::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U passIdx = m_runCtx.m_crntPassIdx++;

	if(passIdx > 0)
	{
		// Bind the Rt

		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx - 1;

		rgraphCtx.bindTextureAndSampler(0, 0, m_runCtx.m_rt, sampleSubresource, m_r->getLinearSampler());
	}
	else
	{
		rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getTemporalAA().getRt(), m_r->getLinearSampler());
	}

	cmdb->setViewport(0, 0, m_rtTex->getWidth() >> passIdx, m_rtTex->getHeight() >> passIdx);
	cmdb->bindShaderProgram(m_grProg);
	drawQuad(cmdb);
}

} // end namespace anki
