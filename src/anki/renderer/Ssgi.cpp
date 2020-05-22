// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssgi.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/core/ConfigSet.h>
#include <shaders/glsl_cpp_common/Ssgi.h>

namespace anki
{

Ssgi::~Ssgi()
{
}

Error Ssgi::init(const ConfigSet& cfg)
{
	const Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize SSGI pass");
	}
	return err;
}

Error Ssgi::initInternal(const ConfigSet& cfg)
{
	const U32 width = m_r->getWidth();
	const U32 height = m_r->getHeight();
	ANKI_R_LOGI("Initializing SSGI pass (%ux%u)", width, height);
	m_main.m_maxSteps = cfg.getNumberU32("r_ssgiMaxSteps");
	m_main.m_depthLod = min(cfg.getNumberU32("r_ssgiDepthLod"), m_r->getDepthDownscale().getMipmapCount() - 1);

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseRgb816x16.png", m_main.m_noiseTex));

	// Create RTs
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(width,
		height,
		Format::R16G16B16A16_SFLOAT,
		TextureUsageBit::IMAGE_COMPUTE_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		"SSGI");
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_main.m_rt = m_r->createAndClearRenderTarget(texinit);

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/Ssgi.ankiprog", m_main.m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_main.m_prog);
	variantInitInfo.addMutation("VARIANT", 0);

	const ShaderProgramResourceVariant* variant;
	m_main.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_main.m_grProg[0] = variant->getProgram();

	variantInitInfo.addMutation("VARIANT", 1);
	m_main.m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_main.m_grProg[1] = variant->getProgram();

	return Error::NONE;
}

void Ssgi::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create RTs
	m_runCtx.m_rt = rgraph.importRenderTarget(m_main.m_rt, TextureUsageBit::SAMPLED_FRAGMENT);

	// Create pass
	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSGI");
	rpass.setWork(
		[](RenderPassWorkContext& rgraphCtx) { static_cast<Ssgi*>(rgraphCtx.m_userData)->run(rgraphCtx); }, this, 0);

	rpass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_READ_WRITE});

	TextureSubresourceInfo hizSubresource;
	hizSubresource.m_firstMipmap = m_main.m_depthLod;
	rpass.newDependency({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, hizSubresource});
}

void Ssgi::run(RenderPassWorkContext& rgraphCtx)
{
	// RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_main.m_grProg[m_r->getFrameCount() & 1u]);

	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, TextureSubresourceInfo());

	// Bind uniforms
	SsgiUniforms* unis = allocateAndBindUniforms<SsgiUniforms*>(sizeof(SsgiUniforms), cmdb, 0, 1);
	unis->m_depthBufferSize = UVec2(m_r->getWidth(), m_r->getHeight()) >> (m_main.m_depthLod + 1);
	unis->m_framebufferSize = UVec2(m_r->getWidth(), m_r->getHeight());

	// Dispatch
	dispatchPPCompute(cmdb, 16, 16, m_r->getWidth() / 2, m_r->getHeight());
}

} // end namespace anki
