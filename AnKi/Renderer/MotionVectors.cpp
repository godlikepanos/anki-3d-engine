// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Window/Input.h>

namespace anki {

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
	ANKI_R_LOGV("Initializing motion vectors");

	// Prog
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/MotionVectors.ankiprogbin", m_prog, m_grProg));

	// RTs
	m_motionVectorsRtDescr = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(), Format::kR16G16_Sfloat, "MotionVectors");
	m_motionVectorsRtDescr.bake();

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	return Error::kNone;
}

void MotionVectors::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(MotionVectors);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDescr);

	RenderPassDescriptionBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(g_preferComputeCVar.get())
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("MotionVectors");

		readUsage = TextureUsageBit::kSampledCompute;
		writeUsage = TextureUsageBit::kStorageComputeWrite;
		ppass = &pass;
	}
	else
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("MotionVectors");
		pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_motionVectorsRtHandle});

		readUsage = TextureUsageBit::kSampledFragment;
		writeUsage = TextureUsageBit::kFramebufferWrite;
		ppass = &pass;
	}

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) -> void {
		ANKI_TRACE_SCOPED_EVENT(MotionVectors);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		rgraphCtx.bindTexture(0, 1, getRenderer().getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));
		rgraphCtx.bindColorTexture(0, 2, getRenderer().getGBuffer().getColorRt(3));

		class Uniforms
		{
		public:
			Mat4 m_currentViewProjMat;
			Mat4 m_currentInvViewProjMat;
			Mat4 m_prevViewProjMat;
		} * pc;
		pc = allocateAndBindConstants<Uniforms>(cmdb, 0, 3);

		pc->m_currentViewProjMat = ctx.m_matrices.m_viewProjection;
		pc->m_currentInvViewProjMat = ctx.m_matrices.m_invertedViewProjection;
		pc->m_prevViewProjMat = ctx.m_prevMatrices.m_viewProjection;

		if(g_preferComputeCVar.get())
		{
			rgraphCtx.bindStorageTexture(0, 4, m_runCtx.m_motionVectorsRtHandle, TextureSubresourceInfo());
		}

		if(g_preferComputeCVar.get())
		{
			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		}
		else
		{
			cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());

			cmdb.draw(PrimitiveTopology::kTriangles, 3);
		}
	});

	ppass->newTextureDependency(m_runCtx.m_motionVectorsRtHandle, writeUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getColorRt(3), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getDepthRt(), readUsage);
	ppass->newTextureDependency(getRenderer().getGBuffer().getPreviousFrameDepthRt(), readUsage);
}

} // end namespace anki
