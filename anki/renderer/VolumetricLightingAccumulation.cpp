// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/VolumetricLightingAccumulation.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/GlobalIllumination.h>
#include <anki/renderer/Renderer.h>
#include <anki/resource/TextureResource.h>
#include <anki/core/ConfigSet.h>

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
	const U32 fractionXY = config.getNumberU32("r_volumetricLightingAccumulationClusterFractionXY");
	ANKI_ASSERT(fractionXY >= 1);
	const U32 fractionZ = config.getNumberU32("r_volumetricLightingAccumulationClusterFractionZ");
	ANKI_ASSERT(fractionZ >= 1);
	m_finalClusterZ = config.getNumberU32("r_volumetricLightingAccumulationFinalClusterInZ");
	ANKI_ASSERT(m_finalClusterZ > 0 && m_finalClusterZ < m_r->getClusterCount()[2]);

	m_volumeSize[0] = m_r->getClusterCount()[0] * fractionXY;
	m_volumeSize[1] = m_r->getClusterCount()[1] * fractionXY;
	m_volumeSize[2] = (m_finalClusterZ + 1) * fractionZ;
	ANKI_R_LOGI("Initializing volumetric lighting accumulation. Size %ux%ux%u", m_volumeSize[0], m_volumeSize[1],
				m_volumeSize[2]);

	ANKI_CHECK(getResourceManager().loadResource("engine_data/blue_noise_rgb8_16x16x16_3d.ankitex", m_noiseTex));

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/VolumetricLightingAccumulation.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("ENABLE_SHADOWS", 1);
	variantInitInfo.addConstant("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]));
	variantInitInfo.addConstant("CLUSTER_COUNT",
								UVec3(m_r->getClusterCount()[0], m_r->getClusterCount()[1], m_r->getClusterCount()[2]));
	variantInitInfo.addConstant("FINAL_CLUSTER_Z", U32(m_finalClusterZ));
	variantInitInfo.addConstant("FRACTION", UVec3(fractionXY, fractionXY, fractionZ));
	variantInitInfo.addConstant("NOISE_TEX_SIZE",
								UVec3(m_noiseTex->getWidth(), m_noiseTex->getHeight(), m_noiseTex->getDepth()));

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
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	const U readRtIdx = m_r->getFrameCount() & 1;

	m_runCtx.m_rts[0] = rgraph.importRenderTarget(m_rtTextures[readRtIdx], TextureUsageBit::SAMPLED_FRAGMENT);
	m_runCtx.m_rts[1] = rgraph.importRenderTarget(m_rtTextures[!readRtIdx], TextureUsageBit::NONE);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol light");

	auto callback = [](RenderPassWorkContext& rgraphCtx) -> void {
		static_cast<VolumetricLightingAccumulation*>(rgraphCtx.m_userData)->run(rgraphCtx);
	};
	pass.setWork(callback, this, 0);

	pass.newDependency({m_runCtx.m_rts[0], TextureUsageBit::SAMPLED_COMPUTE});
	pass.newDependency({m_runCtx.m_rts[1], TextureUsageBit::IMAGE_COMPUTE_WRITE});
	pass.newDependency({m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_COMPUTE});

	m_r->getGlobalIllumination().setRenderGraphDependencies(ctx, pass, TextureUsageBit::SAMPLED_COMPUTE);
}

void VolumetricLightingAccumulation::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	RenderingContext& ctx = *m_runCtx.m_ctx;
	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;

	cmdb->bindShaderProgram(m_grProg);

	// Bind all
	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearRepeat);
	cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);

	rgraphCtx.bindImage(0, 2, m_runCtx.m_rts[1], TextureSubresourceInfo());

	cmdb->bindTexture(0, 3, m_noiseTex->getGrTextureView(), TextureUsageBit::SAMPLED_COMPUTE);

	rgraphCtx.bindColorTexture(0, 4, m_runCtx.m_rts[0]);

	bindUniforms(cmdb, 0, 5, ctx.m_lightShadingUniformsToken);
	bindUniforms(cmdb, 0, 6, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 7, rsrc.m_spotLightsToken);
	rgraphCtx.bindColorTexture(0, 8, m_r->getShadowMapping().getShadowmapRt());

	m_r->getGlobalIllumination().bindVolumeTextures(ctx, rgraphCtx, 0, 9);
	bindUniforms(cmdb, 0, 10, rsrc.m_globalIlluminationProbesToken);

	bindUniforms(cmdb, 0, 11, rsrc.m_fogDensityVolumesToken);
	bindStorage(cmdb, 0, 12, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 13, rsrc.m_indicesToken);

	struct PushConsts
	{
		Vec3 m_padding;
		F32 m_noiseOffset;
	} regs;
	const F32 texelSize = 1.0f / F32(m_noiseTex->getDepth());
	regs.m_noiseOffset = texelSize * F32(m_r->getFrameCount() % m_noiseTex->getDepth()) + texelSize / 2.0f;

	cmdb->setPushConstants(&regs, sizeof(regs));

	dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_workgroupSize[2], m_volumeSize[0],
					  m_volumeSize[1], m_volumeSize[2]);
}

} // end namespace anki
