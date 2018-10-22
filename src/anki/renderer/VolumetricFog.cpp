// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/VolumetricFog.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/ShadowMapping.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/renderer/VolumetricLightingAccumulation.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

Error VolumetricFog::init(const ConfigSet& config)
{
	// Misc
	const U fractionXY = config.getNumber("r.volumetricLightingAccumulation.clusterFractionXY");
	ANKI_ASSERT(fractionXY >= 1);
	const U fractionZ = config.getNumber("r.volumetricLightingAccumulation.clusterFractionZ");
	ANKI_ASSERT(fractionZ >= 1);
	m_finalClusterZ = config.getNumber("r.volumetricLightingAccumulation.finalClusterInZ");
	ANKI_ASSERT(m_finalClusterZ > 0 && m_finalClusterZ < m_r->getClusterCount()[2]);

	m_volumeSize[0] = m_r->getClusterCount()[0] * fractionXY;
	m_volumeSize[1] = m_r->getClusterCount()[1] * fractionXY;
	m_volumeSize[2] = (m_finalClusterZ + 1) * fractionZ;
	ANKI_R_LOGI("Initializing volumetric fog. Size %ux%ux%u", m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]);

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/VolumetricFogAccumulation.glslp", m_prog));

	ShaderProgramResourceConstantValueInitList<5> consts(m_prog);
	consts.add("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]))
		.add("CLUSTER_COUNT", UVec3(m_r->getClusterCount()[0], m_r->getClusterCount()[1], m_r->getClusterCount()[2]))
		.add("FINAL_CLUSTER_Z", U32(m_finalClusterZ))
		.add("FRACTION", UVec3(fractionXY, fractionXY, fractionZ))
		.add("WORKGROUP_SIZE", UVec2(m_workgroupSize[0], m_workgroupSize[1]));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
	m_grProg = variant->getProgram();

	// RT descr
	m_rtDescr =
		m_r->create2DRenderTargetDescription(m_volumeSize[0], m_volumeSize[1], Format::R16G16B16A16_SFLOAT, "Fog");
	m_rtDescr.m_depth = m_volumeSize[2];
	m_rtDescr.m_type = TextureType::_3D;
	m_rtDescr.bake();

	return Error::NONE;
}

void VolumetricFog::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const RenderingContext& ctx = *m_runCtx.m_ctx;

	cmdb->bindShaderProgram(m_grProg);

	rgraphCtx.bindImage(0, 0, m_runCtx.m_rt, TextureSubresourceInfo());

	rgraphCtx.bindColorTextureAndSampler(
		0, 0, m_r->getVolumetricLightingAccumulation().getRt(), m_r->getLinearSampler());

	struct PushConsts
	{
		Vec4 m_fogScatteringCoeffFogAbsorptionCoeffDensityPad1;
		Vec4 m_fogDiffusePad1;
		ClustererMagicValues m_clustererMagic;
	} regs;
	regs.m_fogScatteringCoeffFogAbsorptionCoeffDensityPad1 =
		Vec4(m_fogScatteringCoeff, m_fogAbsorptionCoeff, m_fogDensity, 0.0f);
	regs.m_fogDiffusePad1 = Vec4(m_fogDiffuseColor, 0.0f);
	regs.m_clustererMagic = ctx.m_clusterBinOut.m_shaderMagicValues;

	cmdb->setPushConstants(&regs, sizeof(regs));

	dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_volumeSize[0], m_volumeSize[1]);
}

void VolumetricFog::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol fog");

	auto callback = [](RenderPassWorkContext& rgraphCtx) -> void {
		static_cast<VolumetricFog*>(rgraphCtx.m_userData)->run(rgraphCtx);
	};
	pass.setWork(callback, this, 0);

	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	pass.newDependency({m_r->getVolumetricLightingAccumulation().getRt(), TextureUsageBit::SAMPLED_COMPUTE});
}

} // end namespace anki
