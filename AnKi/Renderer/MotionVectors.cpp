// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

	return Error::kNone;
}

void MotionVectors::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(MotionVectors);
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDescr);

	RenderPassBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(g_preferComputeCVar.get())
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("MotionVectors");

		readUsage = TextureUsageBit::kSrvCompute;
		writeUsage = TextureUsageBit::kUavCompute;
		ppass = &pass;
	}
	else
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("MotionVectors");
		pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_motionVectorsRtHandle)});

		readUsage = TextureUsageBit::kSrvPixel;
		writeUsage = TextureUsageBit::kRtvDsvWrite;
		ppass = &pass;
	}

	ppass->setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) -> void {
		ANKI_TRACE_SCOPED_EVENT(MotionVectors);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
		rgraphCtx.bindSrv(0, 0, getRenderer().getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getColorRt(3));

		class Constants
		{
		public:
			Mat4 m_currentViewProjMat;
			Mat4 m_currentInvViewProjMat;
			Mat4 m_prevViewProjMat;
		} * pc;
		pc = allocateAndBindConstants<Constants>(cmdb, 0, 0);

		pc->m_currentViewProjMat = ctx.m_matrices.m_viewProjection;
		pc->m_currentInvViewProjMat = ctx.m_matrices.m_invertedViewProjection;
		pc->m_prevViewProjMat = ctx.m_prevMatrices.m_viewProjection;

		if(g_preferComputeCVar.get())
		{
			rgraphCtx.bindUav(0, 0, m_runCtx.m_motionVectorsRtHandle);
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
