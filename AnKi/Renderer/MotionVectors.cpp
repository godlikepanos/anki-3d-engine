// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

MotionVectors::~MotionVectors()
{
}

Error MotionVectors::init()
{
	// Prog
	ANKI_CHECK(getResourceManager().loadResource("Shaders/MotionVectors.ankiprog", m_prog));
	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getInternalResolution().x(), m_r->getInternalResolution().y()));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// RTs
	m_motionVectorsRtDescr = m_r->create2DRenderTargetDescription(
		m_r->getInternalResolution().x(), m_r->getInternalResolution().y(), Format::R16G16_SFLOAT, "Motion vectors");
	m_motionVectorsRtDescr.bake();

	m_rejectionFactorRtDescr = m_r->create2DRenderTargetDescription(
		m_r->getInternalResolution().x(), m_r->getInternalResolution().y(), Format::R8_UNORM, "Motion vectors rej");
	m_rejectionFactorRtDescr.bake();

	return Error::NONE;
}

void MotionVectors::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDescr);
	m_runCtx.m_rejectionFactorRtHandle = rgraph.newRenderTarget(m_rejectionFactorRtDescr);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Motion vectors");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) -> void {
		run(ctx, rgraphCtx);
	});

	pass.newDependency({m_runCtx.m_motionVectorsRtHandle, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	pass.newDependency({m_runCtx.m_rejectionFactorRtHandle, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	pass.newDependency({m_r->getGBuffer().getColorRt(3), TextureUsageBit::SAMPLED_COMPUTE});
	pass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});
	pass.newDependency({m_r->getGBuffer().getPreviousFrameDepthRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

void MotionVectors::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindTexture(0, 2, m_r->getGBuffer().getPreviousFrameDepthRt(),
						  TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));
	rgraphCtx.bindColorTexture(0, 3, m_r->getGBuffer().getColorRt(3));
	rgraphCtx.bindImage(0, 4, m_runCtx.m_motionVectorsRtHandle, TextureSubresourceInfo());
	rgraphCtx.bindImage(0, 5, m_runCtx.m_rejectionFactorRtHandle, TextureSubresourceInfo());

	class PC
	{
	public:
		Mat4 m_reprojectionMat;
		Mat4 m_prevViewProjectionInvMat;
	} pc;

	pc.m_reprojectionMat = ctx.m_matrices.m_reprojection;
	pc.m_prevViewProjectionInvMat = ctx.m_prevMatrices.m_invertedProjectionJitter;
	cmdb->setPushConstants(&pc, sizeof(pc));

	dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
}

} // end namespace anki
