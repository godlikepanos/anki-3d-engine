// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/VolumetricLightingAccumulation.h>
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
	const U fraction = config.getNumber("r.volumetricLightingAccumulationClusterFraction");
	ANKI_ASSERT(fraction >= 1);
	m_volumeSize[0] = m_r->getClusterCount()[0] * fraction;
	m_volumeSize[1] = m_r->getClusterCount()[1] * fraction;
	m_volumeSize[2] = m_r->getClusterCount()[2] * fraction;
	ANKI_R_LOGI("Initializing volumetric lighting accumulation. Size %ux%ux%u",
		m_volumeSize[0],
		m_volumeSize[1],
		m_volumeSize[2]);

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoise_Rgb8_64x64x64_3D.ankitex", m_noiseTex));

	// Shaders
	ANKI_CHECK(getResourceManager().loadResource("shaders/VolumetricLightingAccumulation.glslp", m_prog));

	ShaderProgramResourceMutationInitList<1> mutators(m_prog);
	mutators.add("ENABLE_SHADOWS", 1);

	ShaderProgramResourceConstantValueInitList<4> consts(m_prog);
	consts.add("VOLUME_SIZE", UVec3(m_volumeSize[0], m_volumeSize[1], m_volumeSize[2]))
		.add("CLUSTER_COUNT", UVec3(m_r->getClusterCount()[0], m_r->getClusterCount()[1], m_r->getClusterCount()[2]))
		.add("WORKGROUP_SIZE", UVec3(0, 0, 0)) // TODO
		.add("NOISE_MAP_SIZE", UVec3(m_noiseTex->getWidth(), m_noiseTex->getHeight(), m_noiseTex->getDepth()));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutators.get(), consts.get(), variant);
	m_grProg = variant->getProgram();

	// Create RTs
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(m_volumeSize[0],
		m_volumeSize[1],
		Format::R16G16B16A16_SFLOAT,
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
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	m_runCtx.m_rt = rgraph.importRenderTarget(m_rtTex, TextureUsageBit::SAMPLED_FRAGMENT);

	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Vol light");
	pass.setWork(runCallback, this, 0);

	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::IMAGE_COMPUTE_READ_WRITE});
}

void VolumetricLightingAccumulation::run(RenderPassWorkContext& rgraphCtx)
{
	// TODO
}

} // end namespace anki