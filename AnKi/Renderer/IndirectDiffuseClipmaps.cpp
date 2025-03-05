// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error IndirectDiffuseClipmaps::init()
{
	m_tmpRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y(),
																Format::kR8G8B8A8_Unorm, "Test");
	m_tmpRtDesc.bake();

	const Bool supports3Comp = GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats;
	TextureInitInfo volumeInit = getRenderer().create2DRenderTargetInitInfo(
		g_indirectDiffuseClipmap0ProbesPerDimCVar, g_indirectDiffuseClipmap0ProbesPerDimCVar,
		(supports3Comp) ? Format::kR16G16B16_Sfloat : Format::kR16G16B16A16_Sfloat, TextureUsageBit::kAllShaderResource, "IndirectDiffuseClipmap #1");
	volumeInit.m_depth = g_indirectDiffuseClipmap0ProbesPerDimCVar;
	volumeInit.m_type = TextureType::k3D;
	m_clipmapLevelTextures[0] = getRenderer().createAndClearRenderTarget(volumeInit, TextureUsageBit::kSrvCompute);

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", {}, m_tmpProg, m_tmpGrProg, "Test"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/IndirectDiffuseClipmaps.ankiprogbin", {}, m_tmpProg, m_tmpGrProg2, "InitTex"));

	return Error::kNone;
}

void IndirectDiffuseClipmaps::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(IndirectDiffuse);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	RenderTargetHandle volumeRt;
	if(!m_clipmapsImportedOnce)
	{
		volumeRt = rgraph.importRenderTarget(m_clipmapLevelTextures[0].get(), TextureUsageBit::kSrvCompute);
	}
	else
	{
		m_clipmapsImportedOnce = true;
		volumeRt = rgraph.importRenderTarget(m_clipmapLevelTextures[0].get());
	}

	m_runCtx.m_tmpRt = rgraph.newRenderTarget(m_tmpRtDesc);

	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps clear");

		pass.newTextureDependency(volumeRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, volumeRt, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tmpGrProg2.get());

			const Vec4 consts(g_indirectDiffuseClipmap0SizeCVar);
			cmdb.setFastConstants(&consts, sizeof(consts));

			rgraphCtx.bindUav(0, 0, volumeRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			const U32 probeCountPerDim = m_clipmapLevelTextures[0]->getWidth();
			dispatchPPCompute(cmdb, 4, 4, 4, probeCountPerDim, probeCountPerDim, probeCountPerDim);
		});
	}

	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("IndirectDiffuseClipmaps test");

		pass.newTextureDependency(volumeRt, TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getRenderer().getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(m_runCtx.m_tmpRt, TextureUsageBit::kUavCompute);

		pass.setWork([this, volumeRt, &ctx](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_tmpGrProg.get());

			const Vec4 consts(g_indirectDiffuseClipmap0SizeCVar);
			cmdb.setFastConstants(&consts, sizeof(consts));

			rgraphCtx.bindSrv(0, 0, volumeRt);
			rgraphCtx.bindSrv(1, 0, getRenderer().getGBuffer().getDepthRt());
			rgraphCtx.bindUav(0, 0, m_runCtx.m_tmpRt);

			cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x(), getRenderer().getInternalResolution().y());
		});
	}
}

} // end namespace anki
