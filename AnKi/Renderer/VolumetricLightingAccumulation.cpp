// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
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
	const F32 qualityXY = g_volumetricLightingAccumulationQualityXYCVar;
	const F32 qualityZ = g_volumetricLightingAccumulationQualityZCVar;
	const U32 finalZSplit = min<U32>(getRenderer().getZSplitCount() - 1, g_volumetricLightingAccumulationFinalZSplitCVar);

	m_volumeSize[0] = U32(F32(getRenderer().getTileCounts().x()) * qualityXY);
	m_volumeSize[1] = U32(F32(getRenderer().getTileCounts().y()) * qualityXY);
	m_volumeSize[2] = U32(F32(finalZSplit + 1) * qualityZ);

	if(!isAligned(getRenderer().getTileCounts().x(), m_volumeSize[0]) || !isAligned(getRenderer().getTileCounts().y(), m_volumeSize[1])
	   || m_volumeSize[0] == 0 || m_volumeSize[1] == 0 || m_volumeSize[2] == 0)
	{
		ANKI_R_LOGE("Wrong input");
		return Error::kUserData;
	}

	ANKI_CHECK(ResourceManager::getSingleton().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	// Shaders
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VolumetricLightingAccumulation.ankiprogbin", {{"ENABLE_SHADOWS", 1}}, m_prog, m_grProg));

	// Create RTs
	TextureInitInfo texinit = getRenderer().create2DRenderTargetInitInfo(
		m_volumeSize[0], m_volumeSize[1], Format::kR16G16B16A16_Sfloat,
		TextureUsageBit::kUavCompute | TextureUsageBit::kSrvPixel | TextureUsageBit::kSrvCompute, "VolLight");
	texinit.m_depth = m_volumeSize[2];
	texinit.m_type = TextureType::k3D;
	m_rtTextures[0] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);
	m_rtTextures[1] = getRenderer().createAndClearRenderTarget(texinit, TextureUsageBit::kSrvPixel);

	return Error::kNone;
}

void VolumetricLightingAccumulation::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(VolumetricLightingAccumulation);
	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	const U readRtIdx = getRenderer().getFrameCount() & 1;

	m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rtTextures[readRtIdx].get(), TextureUsageBit::kSrvPixel);
	m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rtTextures[!readRtIdx].get(), TextureUsageBit::kNone);

	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Vol light");

	pass.newTextureDependency(m_runCtx.m_rts[0], TextureUsageBit::kSrvCompute);
	pass.newTextureDependency(m_runCtx.m_rts[1], TextureUsageBit::kUavCompute);
	pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvCompute);

	pass.newBufferDependency(getRenderer().getClusterBinning().getClustersBufferHandle(), BufferUsageBit::kSrvCompute);
	pass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kLight),
							 BufferUsageBit::kSrvCompute);
	pass.newBufferDependency(
		getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe),
		BufferUsageBit::kSrvCompute);
	pass.newBufferDependency(getRenderer().getClusterBinning().getPackedObjectsBufferHandle(GpuSceneNonRenderableObjectType::kFogDensityVolume),
							 BufferUsageBit::kSrvCompute);

	if(getRenderer().getIndirectDiffuseProbes().hasCurrentlyRefreshedVolumeRt())
	{
		pass.newTextureDependency(getRenderer().getIndirectDiffuseProbes().getCurrentlyRefreshedVolumeRt(), TextureUsageBit::kSrvCompute);
	}

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(VolumetricLightingAccumulation);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		// Bind all
		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearRepeat.get());
		cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		cmdb.bindSampler(2, 0, getRenderer().getSamplers().m_trilinearClampShadow.get());

		rgraphCtx.bindUav(0, 0, m_runCtx.m_rts[1]);

		cmdb.bindSrv(0, 0, TextureView(&m_noiseImage->getTexture(), TextureSubresourceDesc::all()));

		rgraphCtx.bindSrv(1, 0, m_runCtx.m_rts[0]);

		cmdb.bindConstantBuffer(0, 0, ctx.m_globalRenderingConstantsBuffer);
		cmdb.bindSrv(2, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		cmdb.bindSrv(3, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight));
		rgraphCtx.bindSrv(4, 0, getRenderer().getShadowMapping().getShadowmapRt());
		cmdb.bindSrv(5, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe));
		cmdb.bindSrv(6, 0, getRenderer().getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kFogDensityVolume));
		cmdb.bindSrv(7, 0, getRenderer().getClusterBinning().getClustersBuffer());

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
		consts.m_volumeSize = UVec3(m_volumeSize);

		const U32 finalZSplit = min<U32>(getRenderer().getZSplitCount() - 1, g_volumetricLightingAccumulationFinalZSplitCVar);
		consts.m_maxZSplitsToProcessf = F32(finalZSplit + 1);

		cmdb.setFastConstants(&consts, sizeof(consts));

		dispatchPPCompute(cmdb, 8, 8, 8, m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]);
	});
}

} // end namespace anki
