// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/VolumetricLightingAccumulation.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/Renderer.h>
#include <anki/resource/TextureResource.h>
#include <anki/misc/ConfigSet.h>

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
	const U fractionXY = config.getNumber("r.volumetricLightingAccumulation.clusterFractionXY");
	ANKI_ASSERT(fractionXY >= 1);
	const U fractionZ = config.getNumber("r.volumetricLightingAccumulation.clusterFractionZ");
	ANKI_ASSERT(fractionZ >= 1);
	const U finalClusterZ = config.getNumber("r.volumetricLightingAccumulation.finalClusterInZ");
	ANKI_ASSERT(finalClusterZ > 0 && finalClusterZ < m_r->getClusterCount()[2]);

	m_volumeSize[0] = m_r->getClusterCount()[0] * fractionXY;
	m_volumeSize[1] = m_r->getClusterCount()[1] * fractionXY;
	m_volumeSize[2] = (finalClusterZ + 1) * fractionZ;
	ANKI_R_LOGI("Initializing volumetric lighting accumulation. Size %ux%ux%u",
		m_volumeSize[0],
		m_volumeSize[1],
		m_volumeSize[2]);

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoise_Rgb8_64x64x64_3D.ankitex", m_noiseTex));

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/VolumetricLightingAccumulation.glslp", m_prog));

	ShaderProgramResourceMutationInitList<1> mutators(m_prog);
	mutators.add("ENABLE_SHADOWS", 1);

	ShaderProgramResourceConstantValueInitList<6> consts(m_prog);
	consts.add("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]))
		.add("CLUSTER_COUNT", UVec3(m_r->getClusterCount()[0], m_r->getClusterCount()[1], m_r->getClusterCount()[2]))
		.add("FINAL_CLUSTER_Z", U32(finalClusterZ))
		.add("FRACTION", UVec3(fractionXY, fractionXY, fractionZ))
		.add("WORKGROUP_SIZE", UVec3(m_workgroupSize[0], m_workgroupSize[1], m_workgroupSize[2]))
		.add("NOISE_TEX_SIZE", UVec3(m_noiseTex->getWidth(), m_noiseTex->getHeight(), m_noiseTex->getDepth()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg = variant->getProgram();

	// Create RTs
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_volumeSize[0],
		m_volumeSize[1],
		LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::IMAGE_COMPUTE_READ_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		"VolLight");
	texinit.m_depth = m_volumeSize[2];
	texinit.m_type = TextureType::_3D;
	texinit.m_initialUsage = TextureUsageBit::SAMPLED_FRAGMENT;
	m_rtTex = m_r->createAndClearRenderTarget(texinit);

	return Error::NONE;
}

void VolumetricLightingAccumulation::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.importRenderTarget(m_rtTex, TextureUsageBit::IMAGE_COMPUTE_READ_WRITE); // TODO

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol light");

	auto callback = [](RenderPassWorkContext& rgraphCtx) -> void {
		static_cast<VolumetricLightingAccumulation*>(rgraphCtx.m_userData)->run(rgraphCtx);
	};
	pass.setWork(callback, this, 0);

	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_READ_WRITE});
	pass.newDependency({m_r->getShadowMapping().getShadowmapRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

void VolumetricLightingAccumulation::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	RenderingContext& ctx = *m_runCtx.m_ctx;

	cmdb->bindShaderProgram(m_grProg);

	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, TextureSubresourceInfo());

	cmdb->bindTextureAndSampler(
		0, 0, m_noiseTex->getGrTextureView(), m_r->getTrilinearRepeatSampler(), TextureUsageBit::SAMPLED_COMPUTE);

	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getShadowMapping().getShadowmapRt(), m_r->getLinearSampler());

	const ClusterBinOut& rsrc = ctx.m_clusterBinOut;
	bindUniforms(cmdb, 0, 0, ctx.m_lightShadingUniformsToken);
	bindUniforms(cmdb, 0, 1, rsrc.m_pointLightsToken);
	bindUniforms(cmdb, 0, 2, rsrc.m_spotLightsToken);
	bindStorage(cmdb, 0, 0, rsrc.m_clustersToken);
	bindStorage(cmdb, 0, 1, rsrc.m_indicesToken);

	struct PushConsts
	{
		Vec4 m_noiseOffsetPad3;
	} regs;
	regs.m_noiseOffsetPad3 = Vec4(0.0f);
	const F32 texelSize = 1.0f / m_noiseTex->getDepth();
	regs.m_noiseOffsetPad3.x() = texelSize * F32(m_r->getFrameCount() % m_noiseTex->getDepth()) + texelSize / 2.0f;

	cmdb->setPushConstants(&regs, sizeof(regs));

	dispatchPPCompute(cmdb,
		m_workgroupSize[0],
		m_workgroupSize[1],
		m_workgroupSize[2],
		m_volumeSize[0],
		m_volumeSize[1],
		m_volumeSize[2]);
}

} // end namespace anki