// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Bloom.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Tonemapping.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

Bloom::Bloom(Renderer* r)
	: RendererObject(r)
{
}

Bloom::~Bloom()
{
}

Error Bloom::initExposure(const ConfigSet& config)
{
	m_exposure.m_width = m_r->getDownscaleBlur().getPassWidth(MAX_U) * 2;
	m_exposure.m_height = m_r->getDownscaleBlur().getPassHeight(MAX_U) * 2;

	m_exposure.m_threshold = config.getNumber("r.bloom.threshold");
	m_exposure.m_scale = config.getNumber("r.bloom.scale");

	// Create RT info
	m_exposure.m_rtDescr = m_r->create2DRenderTargetDescription(m_exposure.m_width,
		m_exposure.m_height,
		RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		"Bloom Exp");
	m_exposure.m_rtDescr.bake();

	// Create FB info
	m_exposure.m_fbDescr.m_colorAttachmentCount = 1;
	m_exposure.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_exposure.m_fbDescr.bake();

	// init shaders
	ANKI_CHECK(getResourceManager().loadResource("programs/Bloom.ankiprog", m_exposure.m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_exposure.m_prog);
	consts.add(
		"TEX_SIZE", Vec2(m_r->getDownscaleBlur().getPassWidth(MAX_U), m_r->getDownscaleBlur().getPassHeight(MAX_U)));

	const ShaderProgramResourceVariant* variant;
	m_exposure.m_prog->getOrCreateVariant(consts.get(), variant);
	m_exposure.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error Bloom::initUpscale(const ConfigSet& config)
{
	m_upscale.m_width = m_r->getWidth() / BLOOM_FRACTION;
	m_upscale.m_height = m_r->getHeight() / BLOOM_FRACTION;

	// Create RT descr
	m_upscale.m_rtDescr = m_r->create2DRenderTargetDescription(m_upscale.m_width,
		m_upscale.m_height,
		RT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		"Bloom Upscale");
	m_upscale.m_rtDescr.bake();

	// Create FB descr
	m_upscale.m_fbDescr.m_colorAttachmentCount = 1;
	m_upscale.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_upscale.m_fbDescr.bake();

	// init shaders
	ANKI_CHECK(getResourceManager().loadResource("programs/BloomUpscale.ankiprog", m_upscale.m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_upscale.m_prog);
	consts.add("TEX_SIZE", Vec2(m_upscale.m_width, m_upscale.m_height));

	const ShaderProgramResourceVariant* variant;
	m_upscale.m_prog->getOrCreateVariant(consts.get(), variant);
	m_upscale.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error Bloom::initSslf(const ConfigSet& cfg)
{
	ANKI_CHECK(getResourceManager().loadResource("engine_data/LensDirt.ankitex", m_sslf.m_lensDirtTex));
	ANKI_CHECK(getResourceManager().loadResource("programs/ScreenSpaceLensFlare.ankiprog", m_sslf.m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_sslf.m_prog);
	consts.add("INPUT_TEX_SIZE", UVec2(m_exposure.m_width, m_exposure.m_height));

	const ShaderProgramResourceVariant* variant;
	m_sslf.m_prog->getOrCreateVariant(consts.get(), variant);
	m_sslf.m_grProg = variant->getProgram();

	return Error::NONE;
}

void Bloom::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Main pass
	{
		// Ask for render target
		m_runCtx.m_exposureRt = rgraph.newRenderTarget(m_exposure.m_rtDescr);

		// Set the render pass
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("Bloom Main");
		rpass.setWork(runExposureCallback, this, 0);
		rpass.setFramebufferInfo(m_exposure.m_fbDescr, {{m_runCtx.m_exposureRt}}, {});

		TextureSubresourceInfo inputTexSubresource;
		inputTexSubresource.m_firstMipmap = m_r->getDownscaleBlur().getMipmapCount() - 1;
		rpass.newConsumer({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_FRAGMENT, inputTexSubresource});
		rpass.newConsumer({m_runCtx.m_exposureRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		rpass.newConsumer({m_r->getTonemapping().getAverageLuminanceBuffer(), BufferUsageBit::STORAGE_FRAGMENT_READ});
		rpass.newProducer({m_runCtx.m_exposureRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Upscale & SSLF pass
	{
		// Ask for render target
		m_runCtx.m_upscaleRt = rgraph.newRenderTarget(m_upscale.m_rtDescr);

		// Set the render pass
		GraphicsRenderPassDescription& rpass = rgraph.newGraphicsRenderPass("Bloom Upscale");
		rpass.setWork(runUpscaleAndSslfCallback, this, 0);
		rpass.setFramebufferInfo(m_upscale.m_fbDescr, {{m_runCtx.m_upscaleRt}}, {});

		rpass.newConsumer({m_runCtx.m_upscaleRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		rpass.newConsumer({m_runCtx.m_exposureRt, TextureUsageBit::SAMPLED_FRAGMENT});
		rpass.newProducer({m_runCtx.m_upscaleRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}
}

void Bloom::runExposure(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_exposure.m_width, m_exposure.m_height);
	cmdb->bindShaderProgram(m_exposure.m_grProg);

	TextureSubresourceInfo inputTexSubresource;
	inputTexSubresource.m_firstMipmap = m_r->getDownscaleBlur().getMipmapCount() - 1;
	rgraphCtx.bindTextureAndSampler(
		0, 0, m_r->getDownscaleBlur().getRt(), inputTexSubresource, m_r->getLinearSampler());

	Vec4* uniforms = allocateAndBindUniforms<Vec4*>(sizeof(Vec4), cmdb, 0, 0);
	*uniforms = Vec4(m_exposure.m_threshold, m_exposure.m_scale, 0.0, 0.0);

	rgraphCtx.bindStorageBuffer(0, 0, m_r->getTonemapping().getAverageLuminanceBuffer());

	drawQuad(cmdb);
}

void Bloom::runUpscaleAndSslf(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	// Upscale
	cmdb->setViewport(0, 0, m_upscale.m_width, m_upscale.m_height);
	cmdb->bindShaderProgram(m_upscale.m_grProg);
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_runCtx.m_exposureRt, m_r->getLinearSampler());
	drawQuad(cmdb);

	// SSLF
	cmdb->bindShaderProgram(m_sslf.m_grProg);
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ONE);
	cmdb->bindTextureAndSampler(0,
		1,
		m_sslf.m_lensDirtTex->getGrTextureView(),
		m_sslf.m_lensDirtTex->getSampler(),
		TextureUsageBit::SAMPLED_FRAGMENT);
	drawQuad(cmdb);

	// Retore state
	cmdb->setBlendFactors(0, BlendFactor::ONE, BlendFactor::ZERO);
}

} // end namespace anki
