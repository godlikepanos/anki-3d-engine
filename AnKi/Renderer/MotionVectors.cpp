// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

MotionVectors::~MotionVectors()
{
}

Error MotionVectors::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize motion vectors");
	}
	return err;
}

Error MotionVectors::initInternal()
{
	// Prog
	ANKI_CHECK(getResourceManager().loadResource((getConfig().getRPreferCompute())
													 ? "Shaders/MotionVectorsCompute.ankiprog"
													 : "Shaders/MotionVectorsRaster.ankiprog",
												 m_prog));
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

	m_fbDescr.m_colorAttachmentCount = 2;
	m_fbDescr.bake();

	return Error::NONE;
}

void MotionVectors::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDescr);
	m_runCtx.m_rejectionFactorRtHandle = rgraph.newRenderTarget(m_rejectionFactorRtDescr);

	RenderPassDescriptionBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(getConfig().getRPreferCompute())
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Motion vectors");

		readUsage = TextureUsageBit::SAMPLED_COMPUTE;
		writeUsage = TextureUsageBit::IMAGE_COMPUTE_WRITE;
		ppass = &pass;
	}
	else
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Motion vectors");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_motionVectorsRtHandle, m_runCtx.m_rejectionFactorRtHandle}, {});

		readUsage = TextureUsageBit::SAMPLED_FRAGMENT;
		writeUsage = TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE;
		ppass = &pass;
	}

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) -> void {
		run(ctx, rgraphCtx);
	});

	ppass->newDependency(RenderPassDependency(m_runCtx.m_motionVectorsRtHandle, writeUsage));
	ppass->newDependency(RenderPassDependency(m_runCtx.m_rejectionFactorRtHandle, writeUsage));
	ppass->newDependency(RenderPassDependency(m_r->getGBuffer().getColorRt(3), readUsage));
	ppass->newDependency(RenderPassDependency(m_r->getGBuffer().getDepthRt(), readUsage));
	ppass->newDependency(RenderPassDependency(m_r->getGBuffer().getPreviousFrameDepthRt(), readUsage));
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

	if(getConfig().getRPreferCompute())
	{
		rgraphCtx.bindImage(0, 4, m_runCtx.m_motionVectorsRtHandle, TextureSubresourceInfo());
		rgraphCtx.bindImage(0, 5, m_runCtx.m_rejectionFactorRtHandle, TextureSubresourceInfo());
	}

	class PC
	{
	public:
		Mat4 m_reprojectionMat;
		Mat4 m_prevViewProjectionInvMat;
	} pc;

	pc.m_reprojectionMat = ctx.m_matrices.m_reprojection;
	pc.m_prevViewProjectionInvMat = ctx.m_prevMatrices.m_invertedProjectionJitter;
	cmdb->setPushConstants(&pc, sizeof(pc));

	if(getConfig().getRPreferCompute())
	{
		dispatchPPCompute(cmdb, 8, 8, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
	}
	else
	{
		cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());

		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3);
	}
}

} // end namespace anki
