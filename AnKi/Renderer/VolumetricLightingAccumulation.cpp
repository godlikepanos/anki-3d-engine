// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/IndirectDiffuseClipmaps.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error VolumetricLightingAccumulation::init()
{
	// Misc
	const U32 finalZSplit = min<U32>(g_cvarRenderClustererZSplitCount - 1, g_cvarRenderVolumetricLightingAccumulationFinalZSplit);

	m_volumeSize.xy = getClusterBinning().getTileCounts() << g_cvarRenderVolumetricLightingAccumulationSubdivisionXY;
	m_volumeSize.z = (finalZSplit + 1) << g_cvarRenderVolumetricLightingAccumulationSubdivisionZ;

	if(!isAligned(getClusterBinning().getTileCounts().x, m_volumeSize.x) || !isAligned(getClusterBinning().getTileCounts().y, m_volumeSize.y)
	   || m_volumeSize.x == 0 || m_volumeSize.y == 0 || m_volumeSize.z == 0)
	{
		ANKI_R_LOGE("Wrong input");
		return Error::kUserData;
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	// Shaders
	const Array<SubMutation, 2> mutation = {{{"ENABLE_SHADOWS", 1}, {"CLIPMAP_DIFFUSE_INDIRECT", isIndirectDiffuseClipmapsEnabled()}}};
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VolumetricLightingAccumulation.ankiprogbin", mutation, m_prog, m_grProg, "Accumulate"));
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VolumetricLightingAccumulation.ankiprogbin", mutation, m_prog, m_debugGrProg, "Debug"));

	// Create RTs
	TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
		m_volumeSize.x, m_volumeSize.y, Format::kR16G16B16A16_Sfloat,
		TextureUsageBit::kUavCompute | TextureUsageBit::kSrvPixel | TextureUsageBit::kSrvCompute, "VolLight");
	texinit.m_depth = m_volumeSize.z;
	texinit.m_type = TextureType::k3D;
	m_rtTextures[0] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);
	m_rtTextures[1] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);

	m_debugRtDesc = getRenderer().create2DRenderTargetDescription(getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y,
																  Format::kR16G16B16A16_Sfloat);
	m_debugRtDesc.bake();

	m_debugResult = g_cvarRenderVolumetricLightingAccumulationDebug;

	return Error::kNone;
}

void VolumetricLightingAccumulation::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(VolumetricLightingAccumulation);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const U readRtIdx = getRenderer().getFrameCount() & 1;

	m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rtTextures[readRtIdx].get(), TextureUsageBit::kSrvPixel);
	m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rtTextures[!readRtIdx].get(), TextureUsageBit::kNone);

	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Vol light");

	pass.newTextureDependency(m_runCtx.m_rts[0], TextureUsageBit::kSrvCompute);
	pass.newTextureDependency(m_runCtx.m_rts[1], TextureUsageBit::kUavCompute);
	pass.newTextureDependency(getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvCompute);

	pass.newBufferDependency(getClusterBinning().getDependency(), BufferUsageBit::kSrvCompute);

	if(isIndirectDiffuseProbesEnabled() && getIndirectDiffuseProbes().hasCurrentlyRefreshedVolumeRt())
	{
		pass.newTextureDependency(getIndirectDiffuseProbes().getCurrentlyRefreshedVolumeRt(), TextureUsageBit::kSrvCompute);
	}

	if(isIndirectDiffuseClipmapsEnabled())
	{
		getIndirectDiffuseClipmaps().setDependencies(pass, TextureUsageBit::kSrvCompute);
	}

	pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(VolumetricLightingAccumulation);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		// Bind all
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
		cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		cmdb.bindSampler(2, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());

		rgraphCtx.bindUav(0, 0, m_runCtx.m_rts[1]);

		cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

		U32 srv = 0;
		cmdb.bindSrv(srv++, 0, TextureView(&m_noiseImage->getTexture(), TextureSubresourceDesc::all()));
		rgraphCtx.bindSrv(srv++, 0, m_runCtx.m_rts[0]);

		cmdb.bindSrv(srv++, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		rgraphCtx.bindSrv(srv++, 0, getShadowMapping().getShadowmapRt());
		cmdb.bindSrv(srv++, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kFogDensityVolume));
		cmdb.bindSrv(srv++, 0, getClusterBinning().getClustersBuffer());
		rgraphCtx.bindSrv(srv++, 0, getGBuffer().getDepthRt());

		if(isIndirectDiffuseProbesEnabled())
		{
			cmdb.bindSrv(srv++, 0, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
		}

		const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();

		VolumetricLightingConstants consts;
		if(!sky)
		{
			consts.m_minHeight = 0.0f;
			consts.m_oneOverMaxMinusMinHeight = 0.0f;
			consts.m_densityAtMinHeight = 0.0f;
			consts.m_densityAtMaxHeight = 0.0f;
		}
		else if(sky->getHeightOfMaxFogDensity() > sky->getHeightOfMaxFogDensity())
		{
			consts.m_minHeight = sky->getHeightOfMinFogDensity();
			consts.m_oneOverMaxMinusMinHeight = 1.0f / (sky->getHeightOfMaxFogDensity() - consts.m_minHeight + kEpsilonf);
			consts.m_densityAtMinHeight = sky->getMinFogDensity();
			consts.m_densityAtMaxHeight = sky->getMaxFogDensity();
		}
		else
		{
			consts.m_minHeight = sky->getHeightOfMaxFogDensity();
			consts.m_oneOverMaxMinusMinHeight = 1.0f / (sky->getHeightOfMinFogDensity() - consts.m_minHeight + kEpsilonf);
			consts.m_densityAtMinHeight = sky->getMaxFogDensity();
			consts.m_densityAtMaxHeight = sky->getMinFogDensity();
		}
		consts.m_volumeSize = m_volumeSize;
		consts.m_subZSplitThickness = (getClusterBinning().computeClustererFar() - getRenderingContext().m_matrices.m_near)
									  / F32(g_cvarRenderClustererZSplitCount << g_cvarRenderVolumetricLightingAccumulationSubdivisionZ);
		consts.m_clusterSubdivision =
			UVec3(g_cvarRenderVolumetricLightingAccumulationSubdivisionXY, g_cvarRenderVolumetricLightingAccumulationSubdivisionXY,
				  g_cvarRenderVolumetricLightingAccumulationSubdivisionZ);

		cmdb.setFastConstants(&consts, sizeof(consts));

		dispatchPPCompute(cmdb, 8, 8, 8, m_volumeSize.x, m_volumeSize.y, m_volumeSize.z);
	});

	if(m_debugResult)
	{
		m_runCtx.m_debugRt = rgraph.newRenderTarget(m_debugRtDesc);

		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Vol debug");

		pass.newTextureDependency(getRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(getGBuffer().getDepthRt(), TextureUsageBit::kSrvCompute);
		pass.newTextureDependency(m_runCtx.m_debugRt, TextureUsageBit::kUavCompute);

		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			ANKI_TRACE_SCOPED_EVENT(VolumetricLightingAccumulationDebug);
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_debugGrProg.get());

			rgraphCtx.bindSrv(0, 0, getRt());
			rgraphCtx.bindSrv(1, 0, getGBuffer().getDepthRt());

			rgraphCtx.bindUav(0, 0, m_runCtx.m_debugRt);

			cmdb.bindConstantBuffer(0, 0, getRenderingContext().m_globalRenderingConstantsBuffer);

			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getInternalResolution().x, getRenderer().getInternalResolution().y);
		});
	}
	else
	{
		m_runCtx.m_debugRt = {};
	}
}

void VolumetricLightingAccumulation::fillClustererConstants(ClustererConstants& consts)
{
	const U32 lightZSplitCount = min<U32>(g_cvarRenderClustererZSplitCount, g_cvarRenderVolumetricLightingAccumulationFinalZSplit + 1);
	const F32 clustererFar = getClusterBinning().computeClustererFar() / F32(g_cvarRenderClustererZSplitCount) * F32(lightZSplitCount);
	const F32 n = getRenderingContext().m_matrices.m_near;
	const F32 f = getRenderingContext().m_matrices.m_far;

	consts.m_lightVolumeWMagic.x = (clustererFar - n) / (-n);
	consts.m_lightVolumeWMagic.y = f * (clustererFar - n) / (n * (f - n));
}

void VolumetricLightingAccumulation::getDebugRenderTarget([[maybe_unused]] CString rtName,
														  Array<RenderTargetHandle, U32(DebugRenderTargetRegister::kCount)>& handles,
														  DebugRenderTargetDrawStyle& drawStyle) const
{
	if(m_runCtx.m_debugRt.isValid())
	{
		handles[0] = m_runCtx.m_debugRt;
		drawStyle = DebugRenderTargetDrawStyle::kTonemap;
	}
	else
	{
		ANKI_R_LOGW("Need to enable debug drawing of volumetrics else nothing will happen");
	}
}

} // end namespace anki
