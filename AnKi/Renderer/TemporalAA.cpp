// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/TemporalAA.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error TemporalAA::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/TemporalAA.ankiprogbin", {{"VARIANCE_CLIPPING", 1}, {"YCBCR", 0}}, m_prog, m_grProg));

	for(U32 i = 0; i < 2; ++i)
	{
		TextureUsageBit usage = TextureUsageBit::kSrvPixel | TextureUsageBit::kSrvCompute;
		usage |= (g_cvarRenderPreferCompute) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;

		TextureInitInfo texinit =
			getRenderer().create2DRenderTargetInitInfo(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
													   getRenderer().getHdrFormat(), usage, String().sprintf("TemporalAA #%u", i).cstr());

		m_rtTextures[i] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);
	}

	return Error::kNone;
}

void TemporalAA::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(TemporalAA);
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const U32 historyRtIdx = (getRenderer().getFrameCount() + 1) & 1;
	const U32 renderRtIdx = !historyRtIdx;
	const Bool preferCompute = g_cvarRenderPreferCompute;

	// Import RTs
	if(m_rtTexturesImportedOnce) [[likely]]
	{
		m_runCtx.m_historyRt = rgraph.importRenderTarget(m_rtTextures[historyRtIdx].get());
	}
	else
	{
		m_runCtx.m_historyRt = rgraph.importRenderTarget(m_rtTextures[historyRtIdx].get(), TextureUsageBit::kSrvPixel);
		m_rtTexturesImportedOnce = true;
	}

	m_runCtx.m_renderRt = rgraph.importRenderTarget(m_rtTextures[renderRtIdx].get(), TextureUsageBit::kNone);

	// Create pass
	TextureUsageBit readUsage;
	RenderPassBase* prpass;
	if(preferCompute)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("TemporalAA");

		pass.newTextureDependency(m_runCtx.m_renderRt, TextureUsageBit::kUavCompute);

		readUsage = TextureUsageBit::kSrvCompute;

		prpass = &pass;
	}
	else
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("TemporalAA");
		pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_renderRt)});

		pass.newTextureDependency(m_runCtx.m_renderRt, TextureUsageBit::kRtvDsvWrite);

		readUsage = TextureUsageBit::kSrvPixel;

		prpass = &pass;
	}

	prpass->newTextureDependency(getGBuffer().getDepthRt(), readUsage);
	prpass->newTextureDependency(getRenderer().getLightShading().getRt(), readUsage);
	prpass->newTextureDependency(m_runCtx.m_historyRt, readUsage);
	prpass->newTextureDependency(getRenderer().getMotionVectors().getMotionVectorsRt(), readUsage);

	prpass->setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(TemporalAA);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindSrv(0, 0, getRenderer().getLightShading().getRt());
		rgraphCtx.bindSrv(1, 0, m_runCtx.m_historyRt);
		rgraphCtx.bindSrv(2, 0, getRenderer().getMotionVectors().getMotionVectorsRt());

		if(g_cvarRenderPreferCompute)
		{
			rgraphCtx.bindUav(0, 0, m_runCtx.m_renderRt);

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
