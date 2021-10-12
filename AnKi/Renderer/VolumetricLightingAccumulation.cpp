// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/IndirectDiffuseProbes.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki
{

VolumetricLightingAccumulation::VolumetricLightingAccumulation(Renderer* r)
	: RendererObject(r)
{
}

VolumetricLightingAccumulation::~VolumetricLightingAccumulation()
{
}

Error VolumetricLightingAccumulation::init(const ConfigSet& config)
{
	// Misc
	const F32 qualityXY = config.getNumberF32("r_volumetricLightingAccumulationQualityXY");
	const F32 qualityZ = config.getNumberF32("r_volumetricLightingAccumulationQualityZ");
	m_finalZSplit = min(m_r->getZSplitCount(), config.getNumberU32("r_volumetricLightingAccumulationFinalZSplit"));

	m_volumeSize[0] = U32(F32(m_r->getTileCounts().x()) * qualityXY);
	m_volumeSize[1] = U32(F32(m_r->getTileCounts().y()) * qualityXY);
	m_volumeSize[2] = U32(F32(m_finalZSplit + 1) * qualityZ);
	ANKI_R_LOGI("Initializing volumetric lighting accumulation. Size %ux%ux%u", m_volumeSize[0], m_volumeSize[1],
				m_volumeSize[2]);

	if(!isAligned(m_r->getTileCounts().x(), m_volumeSize[0]) || !isAligned(m_r->getTileCounts().y(), m_volumeSize[1])
	   || m_volumeSize[0] == 0 || m_volumeSize[1] == 0 || m_volumeSize[2] == 0)
	{
		ANKI_R_LOGE("Wrong input");
		return Error::USER_DATA;
	}

	ANKI_CHECK(getResourceManager().loadResource("EngineAssets/BlueNoise_Rgba8_64x64.png", m_noiseImage));

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("Shaders/VolumetricLightingAccumulation.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("ENABLE_SHADOWS", 1);
	variantInitInfo.addConstant("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]));
	variantInitInfo.addConstant("TILE_COUNT", UVec2(m_r->getTileCounts().x(), m_r->getTileCounts().y()));
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("FINAL_Z_SPLIT", m_finalZSplit);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();
	m_workgroupSize = variant->getWorkgroupSizes();

	// Create RTs
	TextureInitInfo texinit =
		m_r->create2DRenderTargetInitInfo(m_volumeSize[0], m_volumeSize[1], Format::R16G16B16A16_SFLOAT,
										  TextureUsageBit::IMAGE_COMPUTE_READ | TextureUsageBit::IMAGE_COMPUTE_WRITE
											  | TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE,
										  "VolLight");
	texinit.m_depth = m_volumeSize[2];
	texinit.m_type = TextureType::_3D;
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_rtTextures[0] = m_r->createAndClearRenderTarget(texinit);
	m_rtTextures[1] = m_r->createAndClearRenderTarget(texinit);

	return Error::NONE;
}

void VolumetricLightingAccumulation::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const U readRtIdx = m_r->getFrameCount() & 1;

	m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rtTextures[readRtIdx], TextureUsageBit::SAMPLED_FRAGMENT);
	m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rtTextures[!readRtIdx], TextureUsageBit::NONE);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol light");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) { run(ctx, rgraphCtx); });

	pass.newDependency(RenderPassDependency(m_runCtx.m_rts[0], TextureUsageBit::SAMPLED_COMPUTE));
	pass.newDependency(RenderPassDependency(m_runCtx.m_rts[1], TextureUsageBit::IMAGE_COMPUTE_WRITE));
	pass.newDependency(
		RenderPassDependency(m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_COMPUTE));

	pass.newDependency(
		RenderPassDependency(ctx.m_clusteredShading.m_clustersBufferHandle, BufferUsageBit::STORAGE_COMPUTE_READ));

	m_r->getIndirectDiffuseProbes().setRenderGraphDependencies(ctx, pass, TextureUsageBit::SAMPLED_COMPUTE);
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

	dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_workgroupSize[2], m_volumeSize[0],
					  m_volumeSize[1], m_volumeSize[2]);
}

} // end namespace anki
