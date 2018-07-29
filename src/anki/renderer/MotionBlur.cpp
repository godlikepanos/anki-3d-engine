// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/MotionBlur.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/GBuffer.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

MotionBlur::~MotionBlur()
{
}

Error MotionBlur::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing motion blur");

	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to init motion blur");
	}

	return err;
}

Error MotionBlur::initInternal(const ConfigSet& config)
{
	// RT
	m_rtDescr = m_r->create2DRenderTargetDescription(
		m_r->getWidth(), m_r->getHeight(), LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT, "MBlur");
	m_rtDescr.bake();

	// Prog
	ANKI_CHECK(getResourceManager().loadResource("shaders/MotionBlur.glslp", m_prog));

	ShaderProgramResourceConstantValueInitList<3> consts(m_prog);
	consts.add("WORKGROUP_SIZE", UVec2(m_workgroupSize[0], m_workgroupSize[1]))
		.add("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()))
		.add("MAX_SAMPLES", U32(config.getNumber("r.motionBlur.maxSamples")));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void MotionBlur::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Motion blur");

	pass.newConsumer({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
	pass.newConsumer({m_r->getGBuffer().getColorRt(3), TextureUsageBit::SAMPLED_COMPUTE});
	pass.newConsumer({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});
	pass.newConsumerAndProducer({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});

	pass.setWork(runCallback, this, 0);
}

void MotionBlur::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const RenderingContext& ctx = *m_runCtx.m_ctx;

	cmdb->bindShaderProgram(m_grProg);

	rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getGBuffer().getColorRt(3), m_r->getNearestSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getTemporalAA().getRt(), m_r->getNearestSampler());
	rgraphCtx.bindTextureAndSampler(0,
		2,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());

	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, TextureSubresourceInfo());

	Mat4 prevViewProjMatMulInvViewProjMat = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection
											* ctx.m_matrices.m_viewProjectionJitter.getInverse();
	cmdb->setPushConstants(&prevViewProjMatMulInvViewProjMat, sizeof(prevViewProjMatMulInvViewProjMat));

	dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_r->getWidth(), m_r->getHeight());
}

} // end namespace anki