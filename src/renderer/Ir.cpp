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
Ir::~Ir()
{}

//==============================================================================
Error Ir::init(
	ThreadPool* threadpool,
	ResourceManager* resources,
	GrManager* gr,
	HeapAllocator<U8> alloc,
	StackAllocator<U8> frameAlloc,
	const ConfigSet& initializer,
	const Timestamp* globalTimestamp)
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
	config.set("is.sm.enabled", true);
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

	ANKI_CHECK(m_r.init(threadpool, resources, gr, alloc, frameAlloc, config,
		globalTimestamp));

	// Init the texture
	TextureInitializer texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_depth = m_cubemapArrSize;
	texinit.m_type = TextureType::CUBE;
	texinit.m_format = Is::RT_PIXEL_FORMAT;
	texinit.m_mipmapsCount = 1;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;

	m_cubemapArr = gr->newInstance<Texture>(texinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::run(SceneNode& frustumNode)
{
	FrustumComponent& frc = frustumNode.getComponent<FrustumComponent>();
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	if(visRez.getReflectionProbeCount() == 0)
	{
		// Early out
		return ErrorCode::NONE;
	}

	const VisibleNode* begin = visRez.getReflectionProbesBegin();
	const VisibleNode* end = visRez.getReflectionProbesBegin();
	while(begin != end)
	{
		ANKI_CHECK(renderReflection(*begin->m_node));
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::renderReflection(SceneNode& node)
{
	const ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	for(U i = 0; i < 6; ++i)
	{
		Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT> cmdb;
		for(U j = 0; j < cmdb.getSize(); ++j)
		{
			cmdb[j] = m_r.getGrManager().newInstance<CommandBuffer>();
		}

		// Render
		ANKI_CHECK(m_r.render(node, i, cmdb));

		// Copy textures
		cmdb[1]->copyTextureToTexture(m_r.getIs().getRt(), 0, 0,
			m_cubemapArr, i, 0);

		// Flush
		for(U j = 0; j < cmdb.getSize(); ++j)
		{
			cmdb[j]->flush();
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki

