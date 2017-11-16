// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	m_passes.destroy(getAllocator());
	m_runCtx.m_rts.destroy(getAllocator());
}

Error DownscaleBlur::initSubpass(U idx, const UVec2& inputTexSize)
{
	Subpass& pass = m_passes[idx];

	pass.m_width = inputTexSize.x() / 2;
	pass.m_height = inputTexSize.y() / 2;

	// RT
	StringAuto name(getAllocator());
	name.sprintf("DownBlur #%u", idx);
	pass.m_rtDescr = m_r->create2DRenderTargetDescription(pass.m_width,
		pass.m_height,
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
			| TextureUsageBit::SAMPLED_COMPUTE,
		SamplingFilter::LINEAR,
		name.toCString());
	pass.m_rtDescr.bake();

	return Error::NONE;
}

Error DownscaleBlur::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing dowscale blur");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize downscale blur");
	}

	return err;
}

Error DownscaleBlur::initInternal(const ConfigSet&)
{
	const U passCount = computeMaxMipmapCount2d(m_r->getWidth(), m_r->getHeight(), DOWNSCALE_BLUR_DOWN_TO) - 1;
	m_passes.create(getAllocator(), passCount);

	UVec2 size(m_r->getWidth(), m_r->getHeight());
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		ANKI_CHECK(initSubpass(i, size));
		size /= 2;
	}

	m_runCtx.m_rts.create(getAllocator(), passCount);

	// FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	// Shader programs
	ANKI_CHECK(getResourceManager().loadResource("programs/DownscaleBlur.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void DownscaleBlur::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U passIdx = m_runCtx.m_crntPassIdx++;

	if(passIdx > 0)
	{
		// Bind the previous pass' Rt

		cmdb->bindTexture(0, 0, rgraphCtx.getTexture(m_runCtx.m_rts[passIdx - 1]));
	}
	else
	{
		cmdb->bindTexture(0, 0, rgraphCtx.getTexture(m_r->getTemporalAA().getRt()));
	}

	const Subpass& pass = m_passes[passIdx];
	cmdb->setViewport(0, 0, pass.m_width, pass.m_height);
	cmdb->bindShaderProgram(m_grProg);
	drawQuad(cmdb);
}

void DownscaleBlur::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_crntPassIdx = 0;

	// Create RTs
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		m_runCtx.m_rts[i] = rgraph.newRenderTarget(m_passes[i].m_rtDescr);
	}

	// Create passes
	Array<CString, 8> passNames = {{"DownBlur #0",
		"Down/Blur #1",
		"Down/Blur #2",
		"Down/Blur #3",
		"Down/Blur #4",
		"Down/Blur #5",
		"Down/Blur #6",
		"Down/Blur #7"}};
	for(U i = 0; i < m_passes.getSize(); ++i)
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[i]);

		pass.setWork(runCallback, this, 0);

		RenderTargetHandle renderRt, sampleRt;
		if(i > 0)
		{
			renderRt = m_runCtx.m_rts[i];
			sampleRt = m_runCtx.m_rts[i - 1];
		}
		else
		{
			renderRt = m_runCtx.m_rts[0];
			sampleRt = m_r->getTemporalAA().getRt();
		}

		pass.setFramebufferInfo(m_fbDescr, {{renderRt}}, {});

		pass.newConsumer({renderRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer({sampleRt, TextureUsageBit::SAMPLED_FRAGMENT});

		pass.newProducer({renderRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}
}

} // end namespace anki
