// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/VolumetricFog.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error VolumetricFog::init()
{
	// Misc
	const U32 zSplitCount = min<U32>(g_cvarRenderClustererZSplitCount, g_cvarRenderVolumetricLightingAccumulationFinalZSplit + 1);

	m_volumeSize.xy = getClusterBinning().getTileCounts() << g_cvarRenderVolumetricLightingAccumulationSubdivisionXY;
	m_volumeSize.z = zSplitCount << g_cvarRenderVolumetricLightingAccumulationSubdivisionZ;

	// Shaders
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VolumetricFogAccumulation.ankiprogbin", m_prog, m_grProg));

	// RT descr
	m_rtDescr = getRenderer().create2DRenderTargetDescription(m_volumeSize.x, m_volumeSize.y, Format::kR16G16B16A16_Sfloat, "Fog");
	m_rtDescr.m_depth = m_volumeSize.z;
	m_rtDescr.m_type = TextureType::k3D;
	m_rtDescr.bake();

	return Error::kNone;
}

void VolumetricFog::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(VolumetricFog);
	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Vol fog");

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kUavCompute);
	pass.newTextureDependency(getRenderer().getVolumetricLightingAccumulation().getRt(), TextureUsageBit::kSrvCompute);

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(VolumetricFog);
		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

		cmdb.bindShaderProgram(m_grProg.get());

		cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
		rgraphCtx.bindSrv(0, 0, getRenderer().getVolumetricLightingAccumulation().getRt());

		rgraphCtx.bindUav(0, 0, m_runCtx.m_rt);

		const SkyboxComponent* sky = SceneGraph::getSingleton().getSkybox();

		VolumetricFogConstants consts;
		consts.m_fogDiffuse = (sky) ? sky->getFogDiffuseColor() : Vec3(0.0f);
		consts.m_fogScatteringCoeff = (sky) ? sky->getFogScatteringCoefficient() : 0.0f;
		consts.m_fogAbsorptionCoeff = (sky) ? sky->getFogAbsorptionCoefficient() : 0.0f;
		consts.m_zSplitThickness = (getClusterBinning().computeClustererFar() - getRenderingContext().m_matrices.m_near)
								   / F32(g_cvarRenderClustererZSplitCount << g_cvarRenderVolumetricLightingAccumulationSubdivisionZ);
		consts.m_volumeSize = m_volumeSize;

		cmdb.setFastConstants(&consts, sizeof(consts));

		dispatchPPCompute(cmdb, 8, 8, m_volumeSize.x, m_volumeSize.y);
	});
}

} // end namespace anki
