// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/core/Config.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/ReflectionProbe.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct ShaderReflectionProbe
{
	Vec3 m_pos;
	F32 m_radiusSq;
	F32 m_cubemapIndex;
	U32 _m_pading[3];
};

//==============================================================================
// Ir                                                                          =
//==============================================================================

//==============================================================================
Ir::~Ir()
{}

//==============================================================================
Error Ir::init(const ConfigSet& initializer)
{
	ANKI_LOGI("Initializing IR (Image Reflections)");
	m_fbSize = initializer.getNumber("ir.rendererSize");

	if(m_fbSize < Renderer::TILE_SIZE)
	{
		ANKI_LOGE("Too low ir.rendererSize");
		return ErrorCode::USER_DATA;
	}

	m_cubemapArrSize = initializer.getNumber("ir.cubemapTextureArraySize");

	if(m_cubemapArrSize < 2)
	{
		ANKI_LOGE("Too low ir.cubemapTextureArraySize");
		return ErrorCode::USER_DATA;
	}

	// Init the renderer
	Config config;
	config.set("dbg.enabled", false);
	config.set("is.sm.bilinearEnabled", true);
	config.set("is.groundLightEnabled", false);
	config.set("is.sm.enabled", false);
	config.set("is.sm.maxLights", 8);
	config.set("is.sm.poissonEnabled", false);
	config.set("is.sm.resolution", 16);
	config.set("lf.maxFlares", 8);
	config.set("pps.enabled", false);
	config.set("renderingQuality", 1.0);
	config.set("width", m_fbSize);
	config.set("height", m_fbSize);
	config.set("lodDistance", 10.0);
	config.set("samples", 1);
	config.set("ir.enabled", false); // Very important to disable that

	ANKI_CHECK(m_nestedR.init(&m_r->getThreadPool(),
		&m_r->getResourceManager(), &m_r->getGrManager(), m_r->getAllocator(),
		m_r->getFrameAllocator(), config, m_r->getGlobalTimestampPtr()));

	// Init the texture
	TextureInitializer texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_depth = m_cubemapArrSize;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_format = Is::RT_PIXEL_FORMAT;
	texinit.m_mipmapsCount = 1;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;

	m_cubemapArr = getGrManager().newInstance<Texture>(texinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::run(CommandBufferPtr cmdb)
{
	FrustumComponent& frc = m_r->getActiveFrustumComponent();
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	const VisibleNode* it = visRez.getReflectionProbesBegin();
	const VisibleNode* end = visRez.getReflectionProbesEnd();
	U probCount = end - it;

	void* data = getGrManager().allocateFrameHostVisibleMemory(
		sizeof(ShaderReflectionProbe) * m_cubemapArrSize + sizeof(UVec4),
		BufferUsage::STORAGE, m_probesToken);

	UVec4* counts = reinterpret_cast<UVec4*>(data);
	counts->x() = probCount;

	ShaderReflectionProbe* probes = reinterpret_cast<ShaderReflectionProbe*>(
		counts + 1);

	while(it != end)
	{
		ANKI_CHECK(renderReflection(*it->m_node, *probes));
		++it;
		++probes;
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::renderReflection(SceneNode& node, ShaderReflectionProbe& shaderProb)
{
	const FrustumComponent& frc = m_r->getActiveFrustumComponent();
	const ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	// Write shader var
	shaderProb.m_pos = (frc.getViewMatrix() * reflc.getPosition()).xyz();
	shaderProb.m_radiusSq = reflc.getRadius() * reflc.getRadius();
	shaderProb.m_cubemapIndex = 0;

	// Render cubemap
	for(U i = 0; i < 6; ++i)
	{
		Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT> cmdb;
		for(U j = 0; j < cmdb.getSize(); ++j)
		{
			cmdb[j] = getGrManager().newInstance<CommandBuffer>();
		}

		// Render
		ANKI_CHECK(m_nestedR.render(node, i, cmdb));

		// Copy textures
		cmdb[cmdb.getSize() - 1]->copyTextureToTexture(
			m_nestedR.getIs().getRt(), 0, 0, m_cubemapArr, i, 0);

		// Flush
		for(U j = 0; j < cmdb.getSize(); ++j)
		{
			cmdb[j]->flush();
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki

