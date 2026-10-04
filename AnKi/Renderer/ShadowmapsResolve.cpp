// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ShadowmapsResolve.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error ShadowmapsResolve::init()
{
	const UVec2 rez = getRenderer().getInternalResolution();

	m_punctualLightsRtDescr =
		getRenderer().create2DRenderTargetDescription(rez.x, rez.y, Format::kR8G8B8A8_Unorm, "Resolve Shadows: Punctual lights");
	m_punctualLightsRtDescr.bake();

	for(U32 i = 0; i < 2; ++i)
	{
		TextureUsageBit texUsage = TextureUsageBit::kAllSrv;
		texUsage |= (g_cvarRenderPreferCompute) ? TextureUsageBit::kUavCompute : TextureUsageBit::kRtvDsvWrite;

		const TextureInitInfo init = getRenderer().create2DRenderTargetInitInfo(rez.x, rez.y, Format::kR8_Unorm, texUsage,
																				generateTempPassName("Resolve Shadows: Dir light #%d", i));

		m_dirLightShadowTextures[i] = getRenderer().createAndClearRenderTarget(init, TextureUsageBit::kSrvCompute);
	}

	// Prog
	for(MutatorValue quality = 0; quality < 3; ++quality)
	{
		ANKI_CHECK(m_progs[U32(quality)].load("ShaderBinaries/ShadowmapsResolve.ankiprogbin", {{"QUALITY", quality}}));
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/STBN_Vec1_2Dx1D_128x128x64.png", m_noiseImage));

	return Error::kNone;
}

void ShadowmapsResolve::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(ShadowmapsResolve);

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_punctualLightsRt = rgraph.newRenderTarget(m_punctualLightsRtDescr);
	const RenderTargetHandle readDirLightRt =
		rgraph.importRenderTarget(m_dirLightShadowTextures[getRenderer().getFrameCount() & 1].get(), m_firstTexImport, TextureUsageBit::kSrvCompute);
	const RenderTargetHandle writeDirLightRt = rgraph.importRenderTarget(m_dirLightShadowTextures[(getRenderer().getFrameCount() + 1) & 1].get(),
																		 m_firstTexImport, TextureUsageBit::kSrvCompute);
	m_runCtx.m_dirLightRt = writeDirLightRt;
	m_firstTexImport = false;

	RenderPassBase* ppass;
	TextureUsageBit readTexUsage, writeTexUsage;
	BufferUsageBit readBuffUsage;
	if(g_cvarRenderPreferCompute)
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Resolve Shadows");
		ppass = &pass;

		readTexUsage = TextureUsageBit::kSrvCompute;
		writeTexUsage = TextureUsageBit::kUavCompute;
		readBuffUsage = BufferUsageBit::kSrvCompute;
	}
	else
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Resolve Shadows");
		pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(m_runCtx.m_punctualLightsRt), GraphicsRenderPassTargetDesc(m_runCtx.m_dirLightRt)});
		ppass = &pass;

		readTexUsage = TextureUsageBit::kSrvPixel;
		writeTexUsage = TextureUsageBit::kRtvDsvWrite;
		readBuffUsage = BufferUsageBit::kSrvPixel;
	}

	ppass->newTextureDependency(m_runCtx.m_punctualLightsRt, writeTexUsage);
	ppass->newTextureDependency(writeDirLightRt, writeTexUsage);
	ppass->newTextureDependency(readDirLightRt, readTexUsage);
	ppass->newTextureDependency(getGBuffer().getDepthRt(), readTexUsage);
	ppass->newTextureDependency(getGBuffer().getColorRt(2), readTexUsage);
	ppass->newTextureDependency(getShadowMapping().getShadowmapRt(), readTexUsage);
	ppass->newTextureDependency(getMotionVectors().getAdjustedMotionVectorsRt(), readTexUsage);
	ppass->newBufferDependency(getClusterBinning().getDependency(), readBuffUsage);

	ppass->setWork([this, readDirLightRt](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(ShadowmapsResolve);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		U32 quality;
		if(g_cvarRenderSmPcss)
		{
			quality = 2;
		}
		else if(g_cvarRenderSmPcf)
		{
			quality = 1;
		}
		else
		{
			quality = 0;
		}

		cmdb.bindShaderProgram(m_progs[quality].get());

		cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());

		cmdb.bindSrv(0, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		rgraphCtx.bindSrv(1, 0, getShadowMapping().getShadowmapRt());
		cmdb.bindSrv(2, 0, getClusterBinning().getClustersBuffer());
		rgraphCtx.bindSrv(3, 0, getGBuffer().getDepthRt());
		rgraphCtx.bindSrv(4, 0, getGBuffer().getColorRt(2));
		cmdb.bindSrv(5, 0, TextureView(&m_noiseImage->getTexture(), TextureSubresourceDesc::all()));
		rgraphCtx.bindSrv(6, 0, readDirLightRt);
		rgraphCtx.bindSrv(7, 0, getMotionVectors().getAdjustedMotionVectorsRt());

		const UVec2 rez = getRenderer().getInternalResolution();
		if(g_cvarRenderPreferCompute)
		{
			rgraphCtx.bindUav(0, 0, m_runCtx.m_punctualLightsRt);
			rgraphCtx.bindUav(1, 0, m_runCtx.m_dirLightRt);
			dispatchPPCompute(cmdb, 8, 8, rez.x, rez.y);
		}
		else
		{
			cmdb.setViewport(0, 0, rez.x, rez.y);
			cmdb.draw(PrimitiveTopology::kTriangles, 3);
		}
	});
}

} // end namespace anki
