// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

Error VolumetricFog::init()
{
	// Misc
	const F32 qualityXY = getConfig().getRVolumetricLightingAccumulationQualityXY();
	const F32 qualityZ = getConfig().getRVolumetricLightingAccumulationQualityZ();
	m_finalZSplit = min(m_r->getZSplitCount() - 1, getConfig().getRVolumetricLightingAccumulationFinalZSplit());

	m_volumeSize[0] = U32(F32(m_r->getTileCounts().x()) * qualityXY);
	m_volumeSize[1] = U32(F32(m_r->getTileCounts().y()) * qualityXY);
	m_volumeSize[2] = U32(F32(m_finalZSplit + 1) * qualityZ);

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("Shaders/VolumetricFogAccumulation.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]));
	variantInitInfo.addConstant("Z_SPLIT_COUNT", m_r->getZSplitCount());
	variantInitInfo.addConstant("FINAL_Z_SPLIT", m_finalZSplit);

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

void VolumetricFog::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol fog");

	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_WRITE});
	pass.newDependency({m_r->getVolumetricLightingAccumulation().getRt(), TextureUsageBit::SAMPLED_COMPUTE});

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) -> void {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);

		cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
		rgraphCtx.bindColorTexture(0, 1, m_r->getVolumetricLightingAccumulation().getRt());

		rgraphCtx.bindImage(0, 2, m_runCtx.m_rt, TextureSubresourceInfo());

		struct PushConsts
		{
			F32 m_fogScatteringCoeff;
			F32 m_fogAbsorptionCoeff;
			F32 m_density;
			F32 m_near;
			Vec3 m_fogDiffuse;
			F32 m_far;
		} regs;

		regs.m_fogScatteringCoeff = m_fogScatteringCoeff;
		regs.m_fogAbsorptionCoeff = m_fogAbsorptionCoeff;
		regs.m_density = m_fogDensity;
		regs.m_fogDiffuse = m_fogDiffuseColor;
		regs.m_near = ctx.m_renderQueue->m_cameraNear;
		regs.m_far = ctx.m_renderQueue->m_cameraFar;

		cmdb->setPushConstants(&regs, sizeof(regs));

		dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_volumeSize[0], m_volumeSize[1]);
	});
}

} // end namespace anki
