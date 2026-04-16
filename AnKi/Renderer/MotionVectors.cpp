// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Window/Input.h>

namespace anki {

Error MotionVectors::init()
{
	ANKI_CHECK(m_grProg.load("ShaderBinaries/MotionVectors.ankiprogbin", {}));

	m_motionVectorsRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR16G16_Sfloat, "MotionVectors");
	m_motionVectorsRtDesc.bake();

	m_adjustedMotionVectorsRtDesc = getRenderer().create2DRenderTargetDescription(
		getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y, Format::kR16G16_Sfloat, "AdjMotionVectors");
	m_adjustedMotionVectorsRtDesc.bake();

	return Error::kNone;
}

void MotionVectors::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(MotionVectors);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_motionVectorsRtHandle = rgraph.newRenderTarget(m_motionVectorsRtDesc);
	m_runCtx.m_adjustedMotionVectorsRtHandle = rgraph.newRenderTarget(m_adjustedMotionVectorsRtDesc);

	RenderPassBase* ppass;
	TextureUsageBit readUsage;
	TextureUsageBit writeUsage;
	if(g_cvarRenderPreferCompute)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("MotionVectors");

		readUsage = TextureUsageBit::kSrvCompute;
		writeUsage = TextureUsageBit::kUavCompute;
		ppass = &pass;
	}
	else
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("MotionVectors");
		pass.setRenderpassInfo(
			{GraphicsRenderPassTargetDesc(m_runCtx.m_motionVectorsRtHandle), GraphicsRenderPassTargetDesc(m_runCtx.m_adjustedMotionVectorsRtHandle)});

		readUsage = TextureUsageBit::kSrvPixel;
		writeUsage = TextureUsageBit::kRtvDsvWrite;
		ppass = &pass;
	}

	ppass->newTextureDependency(m_runCtx.m_motionVectorsRtHandle, writeUsage);
	ppass->newTextureDependency(m_runCtx.m_adjustedMotionVectorsRtHandle, writeUsage);
	ppass->newTextureDependency(getGBuffer().getColorRt(3), readUsage);
	ppass->newTextureDependency(getGBuffer().getDepthRt(), readUsage);
	ppass->newTextureDependency(getGBuffer().getPreviousFrameDepthRt(), readUsage);

	ppass->setWork([this](RenderPassWorkContext& rgraphCtx) -> void {
		ANKI_TRACE_SCOPED_EVENT(MotionVectors);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
		RenderingContext& ctx = getRenderingContext();

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(1, 0, getGBuffer().getColorRt(3));
		rgraphCtx.bindSrv(2, 0, getGBuffer().getPreviousFrameDepthRt());

		cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

		if(g_cvarRenderPreferCompute)
		{
			rgraphCtx.bindUav(0, 0, m_runCtx.m_motionVectorsRtHandle);
			rgraphCtx.bindUav(1, 0, m_runCtx.m_adjustedMotionVectorsRtHandle);
		}

		if(g_cvarRenderPreferCompute)
		{
			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		}
		else
		{
			cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);

			cmdb.draw(PrimitiveTopology::kTriangles, 3);
		}
	});
}

} // end namespace anki
