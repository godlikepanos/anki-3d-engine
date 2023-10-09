// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error VolumetricFog::init()
{
	// Misc
	const F32 qualityXY = g_volumetricLightingAccumulationQualityXYCVar.get();
	const F32 qualityZ = g_volumetricLightingAccumulationQualityZCVar.get();
	m_finalZSplit = min(getRenderer().getZSplitCount() - 1, g_volumetricLightingAccumulationFinalZSplitCVar.get());

	m_volumeSize[0] = U32(F32(getRenderer().getTileCounts().x()) * qualityXY);
	m_volumeSize[1] = U32(F32(getRenderer().getTileCounts().y()) * qualityXY);
	m_volumeSize[2] = U32(F32(m_finalZSplit + 1) * qualityZ);

	ANKI_R_LOGV("Initializing volumetric fog. Resolution %ux%ux%u", m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]);

	// Shaders
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/VolumetricFogAccumulation.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg.reset(&variant->getProgram());
	m_workgroupSize[0] = variant->getWorkgroupSizes()[0];
	m_workgroupSize[1] = variant->getWorkgroupSizes()[1];

	// RT descr
	m_rtDescr = getRenderer().create2DRenderTargetDescription(m_volumeSize[0], m_volumeSize[1], Format::kR16G16B16A16_Sfloat, "Fog");
	m_rtDescr.m_depth = m_volumeSize[2];
	m_rtDescr.m_type = TextureType::k3D;
	m_rtDescr.bake();

	return Error::kNone;
}

void VolumetricFog::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(VolumetricFog);
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol fog");

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavComputeWrite);
	pass.newTextureDependency(getRenderer().getVolumetricLightingAccumulation().getRt(), TextureUsageBit::kSampledCompute);

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(VolumetricFog);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindColorTexture(0, 1, getRenderer().getVolumetricLightingAccumulation().getRt());

		rgraphCtx.bindUavTexture(0, 2, m_runCtx.m_rt, TextureSubresourceInfo());

		const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();

		VolumetricFogConstants regs;
		regs.m_fogDiffuse = (sky) ? sky->getFogDiffuseColor() : Vec3(0.0f);
		regs.m_fogScatteringCoeff = (sky) ? sky->getFogScatteringCoefficient() : 0.0f;
		regs.m_fogAbsorptionCoeff = (sky) ? sky->getFogAbsorptionCoefficient() : 0.0f;
		regs.m_near = ctx.m_cameraNear;
		regs.m_far = ctx.m_cameraFar;
		regs.m_zSplitCountf = F32(getRenderer().getZSplitCount());
		regs.m_volumeSize = UVec3(m_volumeSize);
		regs.m_maxZSplitsToProcessf = F32(m_finalZSplit + 1);

		cmdb.setPushConstants(&regs, sizeof(regs));

		dispatchPPCompute(cmdb, m_workgroupSize[0], m_workgroupSize[1], m_volumeSize[0], m_volumeSize[1]);
	});
}

} // end namespace anki
