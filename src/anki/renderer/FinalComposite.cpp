// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/Tonemapping.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Reflections.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/UiStage.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

FinalComposite::FinalComposite(Renderer* r)
	: RendererObject(r)
{
}

FinalComposite::~FinalComposite()
{
}

Error FinalComposite::initInternal(const ConfigSet& config)
{
	ANKI_ASSERT("Initializing PPS");

	ANKI_CHECK(loadColorGradingTexture("engine_data/DefaultLut.ankitex"));
	m_sharpenEnabled = config.getNumber("r.finalComposite.sharpen");

	if(!m_r->getDrawToDefaultFramebuffer())
	{
		m_rtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth(),
			m_r->getHeight(),
			RT_PIXEL_FORMAT,
			TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
			"Final Composite");
		m_rtDescr.bake();

		m_fbDescr.m_colorAttachmentCount = 1;
		m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fbDescr.bake();
	}
	else
	{
		m_fbDescr.setDefaultFramebuffer();
		m_fbDescr.bake();
	}

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_blueNoise));

	// Progs
	ANKI_CHECK(getResourceManager().loadResource("programs/FinalComposite.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<4> mutations(m_prog);
	mutations.add("BLUE_NOISE", 1)
		.add("SHARPEN_ENABLED", m_sharpenEnabled)
		.add("BLOOM_ENABLED", 1)
		.add("DBG_ENABLED", 0);

	ShaderProgramResourceConstantValueInitList<2> consts(m_prog);
	consts.add("LUT_SIZE", U32(LUT_SIZE)).add("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
	m_grProgs[0] = variant->getProgram();

	mutations[3].m_value = 1;
	m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
	m_grProgs[1] = variant->getProgram();

	return Error::NONE;
}

Error FinalComposite::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to init PPS");
	}

	return err;
}

Error FinalComposite::loadColorGradingTexture(CString filename)
{
	m_lut.reset(nullptr);
	ANKI_CHECK(getResourceManager().loadResource(filename, m_lut));
	ANKI_ASSERT(m_lut->getWidth() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getHeight() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getDepth() == LUT_SIZE);

	return Error::NONE;
}

void FinalComposite::run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const Bool dbgEnabled = m_r->getDbg().getEnabled();
	const Bool drawToDefaultFb = m_r->getDrawToDefaultFramebuffer();

	// Bind stuff
	rgraphCtx.bindColorTextureAndSampler(
		0, 0, m_r->getTemporalAA().getRt(), (drawToDefaultFb) ? m_r->getNearestSampler() : m_r->getLinearSampler());

	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getBloom().getRt(), m_r->getLinearSampler());
	cmdb->bindTextureAndSampler(
		0, 2, m_lut->getGrTextureView(), m_r->getLinearSampler(), TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTextureAndSampler(
		0, 3, m_blueNoise->getGrTextureView(), m_blueNoise->getSampler(), TextureUsageBit::SAMPLED_FRAGMENT);
	if(dbgEnabled)
	{
		rgraphCtx.bindColorTextureAndSampler(0, 5, m_r->getDbg().getRt(), m_r->getLinearSampler());
	}

	rgraphCtx.bindUniformBuffer(0, 1, m_r->getTonemapping().getAverageLuminanceBuffer());

	Vec4* uniforms = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	uniforms->x() = F32(m_r->getFrameCount() % m_blueNoise->getLayerCount());

	U width, height;
	if(drawToDefaultFb)
	{
		width = ctx.m_outFbWidth;
		height = ctx.m_outFbHeight;
	}
	else
	{
		width = m_r->getWidth();
		height = m_r->getHeight();
	}
	cmdb->setViewport(0, 0, width, height);

	cmdb->bindShaderProgram(m_grProgs[dbgEnabled]);
	drawQuad(cmdb);

	// Draw UI
	m_r->getUiStage().draw(ctx, cmdb);
}

void FinalComposite::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;
	const Bool drawToDefaultFb = m_r->getDrawToDefaultFramebuffer();
	const Bool dbgEnabled = m_r->getDbg().getEnabled();

	// Maybe create the RT
	if(!drawToDefaultFb)
	{
		m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);
	}

	// Create the pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Final Composite");

	pass.setWork(runCallback, this, 0);

	if(drawToDefaultFb)
	{
		pass.setFramebufferInfo(m_fbDescr, {}, {});
	}
	else
	{
		pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rt}}, {});
	}

	if(!drawToDefaultFb)
	{
		pass.newConsumer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	if(dbgEnabled)
	{
		pass.newConsumer({m_r->getDbg().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	}
	pass.newConsumer({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_r->getBloom().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_r->getTonemapping().getAverageLuminanceBuffer(), BufferUsageBit::UNIFORM_FRAGMENT});
}

} // end namespace anki
