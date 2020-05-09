// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/core/ConfigSet.h>

namespace anki
{

Error VolumetricFog::init(const ConfigSet& config)
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
	ANKI_R_LOGI("Initializing volumetric fog. Size %ux%ux%u", m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]);

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/VolumetricFogAccumulation.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]));
	variantInitInfo.addConstant("FINAL_CLUSTER_Z", m_finalClusterZ);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();
	m_workgroupSize[0] = variant->getWorkgroupSizes()[0];
	m_workgroupSize[1] = variant->getWorkgroupSizes()[1];

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

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
	rgraphCtx.bindColorTexture(0, 1, m_r->getVolumetricLightingAccumulation().getRt());

	rgraphCtx.bindImage(0, 2, m_runCtx.m_rt, TextureSubresourceInfo());

	struct PushConsts
	{
		F32 m_fogScatteringCoeff;
		F32 m_fogAbsorptionCoeff;
		F32 m_density;
		F32 m_padding0;
		Vec3 m_fogDiffuse;
		U32 m_padding1;
		ClustererMagicValues m_clustererMagic;
	} regs;

	regs.m_fogScatteringCoeff = m_fogScatteringCoeff;
	regs.m_fogAbsorptionCoeff = m_fogAbsorptionCoeff;
	regs.m_density = m_fogDensity;
	regs.m_fogDiffuse = m_fogDiffuseColor;
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
