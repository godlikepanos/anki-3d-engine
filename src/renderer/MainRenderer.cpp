// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/MainRenderer.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Ir.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Camera.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/core/Trace.h>
#include <anki/core/App.h>
#include <anki/misc/ConfigSet.h>

namespace anki {

//==============================================================================
MainRenderer::MainRenderer()
{}

//==============================================================================
MainRenderer::~MainRenderer()
{
	ANKI_LOGI("Destroying main renderer");
	m_materialShaderSource.destroy(m_alloc);
}

//==============================================================================
Error MainRenderer::create(
	ThreadPool* threadpool,
	ResourceManager* resources,
	GrManager* gr,
	AllocAlignedCallback allocCb,
	void* allocCbUserData,
	const ConfigSet& config,
	const Timestamp* globalTimestamp)
{
	ANKI_LOGI("Initializing main renderer");

	m_alloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	m_frameAlloc = StackAllocator<U8>(allocCb, allocCbUserData,
		1024 * 1024 * 10, 1.0);

	// Init default FB
	m_width = config.getNumber("width");
	m_height = config.getNumber("height");
	FramebufferInitializer fbInit;
	m_defaultFb = gr->newInstance<Framebuffer>(fbInit);

	// Init renderer
	ConfigSet config2 = config;
	m_renderingQuality = config.getNumber("renderingQuality");
	config2.set("width", m_renderingQuality * F32(m_width));
	config2.set("height", m_renderingQuality * F32(m_height));

	m_r.reset(m_alloc.newInstance<Renderer>());
	ANKI_CHECK(m_r->init(threadpool, resources, gr, m_alloc,
		m_frameAlloc, config2, globalTimestamp));

	// Set the default preprocessor string
	m_materialShaderSource.sprintf(
		m_alloc,
		"#define ANKI_RENDERER_WIDTH %u\n"
		"#define ANKI_RENDERER_HEIGHT %u\n"
		"#define TILE_SIZE %u\n",
		m_r->getWidth(), m_r->getHeight(), Renderer::TILE_SIZE);

	// Init other
	ANKI_CHECK(m_r->getResourceManager().loadResource(
		"shaders/Final.frag.glsl", m_blitFrag));

	ColorStateInfo colorState;
	colorState.m_attachmentCount = 1;
	colorState.m_attachments[0].m_format =
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
	m_r->createDrawQuadPipeline(
		m_blitFrag->getGrShader(), colorState, m_blitPpline);

	ANKI_LOGI("Main renderer initialized. Rendering size %dx%d",
		m_width, m_height);

	// Init RC group
	ResourceGroupInitializer rcinit;
	if(m_r->getPps().getEnabled())
	{
		rcinit.m_textures[0].m_texture = m_r->getPps().getRt();
	}
	else
	{
		rcinit.m_textures[0].m_texture = m_r->getIs().getRt();
	}

	m_rcGroup = m_r->getGrManager().newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

//==============================================================================
Error MainRenderer::render(SceneGraph& scene)
{
	ANKI_TRACE_START_EVENT(RENDER);

	// First thing, reset the temp mem pool
	m_frameAlloc.getMemoryPool().reset();

	GrManager& gl = m_r->getGrManager();
	Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT> cmdbs;
	CommandBufferPtr& cmdb = cmdbs[RENDERER_COMMAND_BUFFERS_COUNT - 1];

	for(U i = 0; i < RENDERER_COMMAND_BUFFERS_COUNT; i++)
	{
		cmdbs[i] = gl.newInstance<CommandBuffer>(m_cbInitHints[i]);
	}

	// Find where the m_r should draw
	Bool rDrawToDefault;
	if(m_renderingQuality == 1.0 && !m_r->getDbg().getEnabled()
		&& m_r->getPps().getEnabled())
	{
		rDrawToDefault = true;
	}
	else
	{
		rDrawToDefault = false;
	}

	if(rDrawToDefault)
	{
		m_r->setOutputFramebuffer(m_defaultFb, m_width, m_height);
	}
	else
	{
		m_r->setOutputFramebuffer(FramebufferPtr(), 0, 0);
	}

	// Run renderer
	m_r->getIs().setAmbientColor(scene.getAmbientColor());
	ANKI_CHECK(m_r->render(scene.getActiveCamera(), 0, cmdbs));

	if(!rDrawToDefault)
	{
		cmdb->bindFramebuffer(m_defaultFb);
		cmdb->setViewport(0, 0, m_width, m_height);

		cmdb->bindPipeline(m_blitPpline);
		cmdb->bindResourceGroup(m_rcGroup, 0, nullptr);

		m_r->drawQuad(cmdb);
	}

	// Flush the command buffers
	for(U i = 0; i < RENDERER_COMMAND_BUFFERS_COUNT; i++)
	{
		cmdbs[i]->flush();
	}

	// Set the hints
	for(U i = 0; i < RENDERER_COMMAND_BUFFERS_COUNT; i++)
	{
		m_cbInitHints[i] = cmdbs[i]->computeInitHints();
	}

	ANKI_TRACE_STOP_EVENT(RENDER);

	return ErrorCode::NONE;
}

//==============================================================================
Dbg& MainRenderer::getDbg()
{
	return m_r->getDbg();
}

//==============================================================================
F32 MainRenderer::getAspectRatio() const
{
	return m_r->getAspectRatio();
}

//==============================================================================
void MainRenderer::prepareForVisibilityTests(Camera& cam)
{
	m_r->prepareForVisibilityTests(cam);
}

} // end namespace anki
