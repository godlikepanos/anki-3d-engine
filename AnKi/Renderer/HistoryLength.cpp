// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/HistoryLength.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/MotionVectors.h>

namespace anki {

Error HistoryLength::init()
{
	for(U32 i = 0; i < 2; ++i)
	{
		TextureUsageBit texUsage = TextureUsageBit::kAllSrv;
		texUsage |= (g_preferComputeCVar) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;

		const TextureInitInfo init =
			getRenderer().create2DRenderTargetInitInfo(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
													   Format::kR8_Unorm, texUsage, generateTempPassName("HistoryLen #%d", i));

		m_historyLenTextures[i] = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/HistoryLength.ankiprogbin", m_prog, m_grProg));

	return Error::kNone;
}

void HistoryLength::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	RenderTargetHandle history;
	RenderTargetHandle current;
	U32 readTex = getRenderer().getFrameCount() & 1;
	U32 writeTex = !readTex;
	if(m_texturesImportedOnce) [[likely]]
	{
		history = rgraph.importRenderTarget(m_historyLenTextures[readTex].get());
		current = rgraph.importRenderTarget(m_historyLenTextures[writeTex].get());
	}
	else
	{
		history = rgraph.importRenderTarget(m_historyLenTextures[readTex].get(), TextureUsageBit::kSrvCompute);
		current = rgraph.importRenderTarget(m_historyLenTextures[writeTex].get(), TextureUsageBit::kSrvCompute);
		m_texturesImportedOnce = true;
	}

	m_runCtx.m_rt = current;

	RenderPassBase* pass;
	TextureUsageBit readTexUsage;
	TextureUsageBit writeTexUsage;
	if(g_preferComputeCVar)
	{
		pass = &rgraph.newNonGraphicsRenderPass("History length");
		readTexUsage = TextureUsageBit::kSrvCompute;
		writeTexUsage = TextureUsageBit::kUavCompute;
	}
	else
	{
		GraphicsRenderPass& rpass = rgraph.newGraphicsRenderPass("History length");
		rpass.setRenderpassInfo({GraphicsRenderPassTargetDesc(current)});
		pass = &rpass;

		readTexUsage = TextureUsageBit::kSrvPixel;
		writeTexUsage = TextureUsageBit::kRtvDsvWrite;
	}

	pass->newTextureDependency(getGBuffer().getDepthRt(), readTexUsage);
	pass->newTextureDependency(getGBuffer().getPreviousFrameDepthRt(), readTexUsage);
	pass->newTextureDependency(getMotionVectors().getMotionVectorsRt(), readTexUsage);
	pass->newTextureDependency(history, readTexUsage);
	pass->newTextureDependency(current, writeTexUsage);

	pass->setWork([this, &ctx, history, current](RenderPassWorkContext& rgraphCtx) {
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		rgraphCtx.bindSrv(0, 0, getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(1, 0, getGBuffer().getPreviousFrameDepthRt());
		rgraphCtx.bindSrv(2, 0, getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindSrv(3, 0, history);

		cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

		if(g_preferComputeCVar)
		{
			rgraphCtx.bindUav(0, 0, current);
			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		}
		else
		{
			cmdb.setViewport(0, 0, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
			drawQuad(cmdb);
		}
	});
}

} // end namespace anki
