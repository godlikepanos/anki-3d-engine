// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ssr.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/misc/ConfigSet.h>
#include <shaders/glsl_cpp_common/Ssr.h>

namespace anki
{

Ssr::~Ssr()
{
}

Error Ssr::init(const ConfigSet& cfg)
{
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize reflection pass");
	}
	return err;
}

Error Ssr::initInternal(const ConfigSet& cfg)
{
	U32 width = m_r->getWidth() / SSR_FRACTION;
	U32 height = m_r->getHeight() / SSR_FRACTION;
	ANKI_R_LOGI("Initializing SSR pass (%ux%u)", width, height);

	// Create RTs
	m_rtDescr = m_r->create2DRenderTargetDescription(width, height, Format::R16G16B16A16_SFLOAT, "SSR");
	m_rtDescr.bake();

	// Create shader
	ANKI_CHECK(getResourceManager().loadResource("shaders/Ssr.glslp", m_prog));

	ShaderProgramResourceConstantValueInitList<4> consts(m_prog);
	consts.add("FB_SIZE", UVec2(width, height));
	consts.add("WORKGROUP_SIZE", UVec2(m_workgroupSize[0], m_workgroupSize[1]));
	consts.add("MAX_STEPS", U32(64));
	consts.add("LIGHT_BUFFER_MIP_COUNT", U32(m_r->getDownscaleBlur().getMipmapCount()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void Ssr::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create RTs
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	ComputeRenderPassDescription& rpass = rgraph.newComputeRenderPass("SSR");
	rpass.setWork(runCallback, this, 0);

	rpass.newConsumerAndProducer({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(1), TextureUsageBit::SAMPLED_COMPUTE});
	rpass.newConsumer({m_r->getGBuffer().getColorRt(2), TextureUsageBit::SAMPLED_COMPUTE});

	TextureSubresourceInfo hizSubresource; // Only first mip
	rpass.newConsumer({m_r->getDepthDownscale().getHiZRt(), TextureUsageBit::SAMPLED_COMPUTE, hizSubresource});

	rpass.newConsumer({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

void Ssr::run(RenderPassWorkContext& rgraphCtx)
{
	RenderingContext& ctx = *m_runCtx.m_ctx;
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->bindShaderProgram(m_grProg);

	// Bind textures
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getGBuffer().getColorRt(1), m_r->getLinearSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getGBuffer().getColorRt(2), m_r->getLinearSampler());

	TextureSubresourceInfo hizSubresource; // Only first mip
	rgraphCtx.bindTextureAndSampler(0, 2, m_r->getDepthDownscale().getHiZRt(), hizSubresource, m_r->getLinearSampler());

	rgraphCtx.bindColorTextureAndSampler(0, 3, m_r->getDownscaleBlur().getRt(), m_r->getTrilinearRepeatSampler());

	// Bind image
	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, TextureSubresourceInfo());

	// Bind uniforms
	SsrUniforms* unis = allocateAndBindUniforms<SsrUniforms*>(sizeof(SsrUniforms), cmdb, 0, 0);
	unis->m_nearPad3 = Vec4(ctx.m_renderQueue->m_cameraNear);
	unis->m_prevViewProjMatMulInvViewProjMat = ctx.m_prevViewProjMat * ctx.m_viewProjMatJitter.getInverse();
	unis->m_projMat = ctx.m_projMatJitter;
	unis->m_invProjMat = ctx.m_projMatJitter.getInverse();
	unis->m_normalMat = Mat3x4(ctx.m_renderQueue->m_viewMatrix.getRotationPart());

	// Dispatch
	dispatchPPCompute(
		cmdb, m_workgroupSize[0], m_workgroupSize[1], m_r->getWidth() / SSR_FRACTION, m_r->getHeight() / SSR_FRACTION);
}

} // end namespace anki
