// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

VolumetricLightingAccumulation::VolumetricLightingAccumulation(Renderer* r)
	: RendererObject(r)
{
}

VolumetricLightingAccumulation::~VolumetricLightingAccumulation()
{
}

Error VolumetricLightingAccumulation::init()
{
	// Misc
	const F32 qualityXY = getConfig().getRVolumetricLightingAccumulationQualityXY();
	const F32 qualityZ = getConfig().getRVolumetricLightingAccumulationQualityZ();
	m_finalZSplit = min(m_r->getZSplitCount() - 1, getConfig().getRVolumetricLightingAccumulationFinalZSplit());

	m_volumeSize[0] = U32(F32(m_r->getTileCounts().x()) * qualityXY);
	m_volumeSize[1] = U32(F32(m_r->getTileCounts().y()) * qualityXY);
	m_volumeSize[2] = U32(F32(m_finalZSplit + 1) * qualityZ);
	ANKI_R_LOGV("Initializing volumetric lighting accumulation. Size %ux%ux%u", m_volumeSize[0], m_volumeSize[1],
				m_volumeSize[2]);

	if(!isAligned(m_r->getTileCounts().x(), m_volumeSize[0]) || !isAligned(m_r->getTileCounts().y(), m_volumeSize[1])
	   || m_volumeSize[0] == 0 || m_volumeSize[1] == 0 || m_volumeSize[2] == 0)
	{
		ANKI_R_LOGE("Wrong input");
		return Error::kUserData;
	}

	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/VolumetricLightingAccumulation.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("ENABLE_SHADOWS", 1);
	variantInitInfo.addConstant("kVolumeSize", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]));
	variantInitInfo.addConstant("kTileCount", UVec2(m_r->getTileCounts().x(), m_r->getTileCounts().y()));
	variantInitInfo.addConstant("kZSplitCount", m_r->getZSplitCount());
	variantInitInfo.addConstant("kFinalZSplit", m_finalZSplit);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();
	m_workgroupSize = variant->getWorkgroupSizes();

	// Create RTs
	TextureInitInfo texinit =
		m_r->create2DRenderTargetInitInfo(m_volumeSize[0], m_volumeSize[1], Format::kR16G16B16A16_Sfloat,
										  TextureUsageBit::kImageComputeRead | TextureUsageBit::kImageComputeWrite
											  | TextureUsageBit::kSampledFragment | TextureUsageBit::kSampledCompute,
										  "VolLight");
	texinit.m_depth = m_volumeSize[2];
	texinit.m_type = TextureType::k3D;
	m_rtTextures[0] = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);
	m_rtTextures[1] = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::kSampledFragment);

	return Error::kNone;
}

void VolumetricLightingAccumulation::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const U readRtIdx = m_r->getFrameCount() & 1;

	m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rtTextures[readRtIdx], TextureUsageBit::kSampledFragment);
	m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rtTextures[!readRtIdx], TextureUsageBit::kNone);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol light");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});

	pass.newTextureDependency(m_runCtx.m_rts[0], TextureUsageBit::kSampledCompute);
	pass.newTextureDependency(m_runCtx.m_rts[1], TextureUsageBit::kImageComputeWrite);
	pass.newTextureDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::kSampledCompute);

	pass.newBufferDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::kStorageComputeRead);

	m_r->getIndirectDiffuseProbes().setRenderGraphDependencies(ctx, pass, TextureUsageBit::kSampledCompute);
}

void VolumetricLightingAccumulation::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const ClusteredShadingContext& rsrc = ctx.m_clusteredShading;

	cmdb->bindShaderProgram(m_grProg);

	// Bind all
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearRepeat);
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindImage(0, 2, m_runCtx.m_rts[1], TextureSubresourceInfo());

	cmdb->bindTexture(0, 3, m_noiseImage->getTextureView());

	rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_rts[0]);

	bindUniforms(cmdb, 0, 5, rsrc.m_clusteredShadingUniformsToken);
	bindUniforms(cmdb, 0, 6, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 7, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 8, m_r->getShadowMapping().getShadowmapRt());

	m_r->getIndirectDiffuseProbes().bindVolumeTextures(ctx, rgraphCtx, 0, 9);
	bindUniforms(cmdb, 0, 10, rsrc.m_globalIlluminationProbesToken);

	bindUniforms(cmdb, 0, 11, rsrc.m_fogDensityVolumesToken);
	bindStorage(cmdb, 0, 12, rsrc.m_clustersToken);

	VolumetricLightingUniforms unis;
	const SkyboxQueueElement& queueEl = ctx.m_renderQueue->m_skybox;
	if(queueEl.m_fog.m_heightOfMaxDensity > queueEl.m_fog.m_heightOfMinDensity)
	{
		unis.m_minHeight = queueEl.m_fog.m_heightOfMinDensity;
		unis.m_oneOverMaxMinusMinHeight = 1.0f / (queueEl.m_fog.m_heightOfMaxDensity - unis.m_minHeight + kEpsilonf);
		unis.m_densityAtMinHeight = queueEl.m_fog.m_minDensity;
		unis.m_densityAtMaxHeight = queueEl.m_fog.m_maxDensity;
	}
	else
	{
		unis.m_minHeight = queueEl.m_fog.m_heightOfMaxDensity;
		unis.m_oneOverMaxMinusMinHeight = 1.0f / (queueEl.m_fog.m_heightOfMinDensity - unis.m_minHeight + kEpsilonf);
		unis.m_densityAtMinHeight = queueEl.m_fog.m_maxDensity;
		unis.m_densityAtMaxHeight = queueEl.m_fog.m_minDensity;
	}
	cmdb->setPushConstants(&unis, sizeof(unis));

	dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_workgroupSize[2], m_volumeSize[0],
					  m_volumeSize[1], m_volumeSize[2]);
}

} // end namespace anki
