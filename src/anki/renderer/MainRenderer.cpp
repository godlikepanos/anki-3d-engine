// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

namespace anki
{

MainRenderer::MainRenderer()
{
}

MainRenderer::~MainRenderer()
{
	ANKI_R_LOGI("Destroying main renderer");
	m_materialShaderSource.destroy(m_alloc);
}

Error MainRenderer::create(ThreadPool* threadpool,
	ResourceManager* resources,
	GrManager* gr,
	StagingGpuMemoryManager* stagingMem,
	AllocAlignedCallback allocCb,
	void* allocCbUserData,
	const ConfigSet& config,
	Timestamp* globTimestamp)
{
	ANKI_R_LOGI("Initializing main renderer");

	m_alloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	m_frameAlloc = StackAllocator<U8>(allocCb, allocCbUserData, 1024 * 1024 * 10, 1.0);

	// Init default FB
	m_width = config.getNumber("width");
	m_height = config.getNumber("height");
	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_defaultFb = gr->newInstance<Framebuffer>(fbInit);

	// Init renderer and manipulate the width/height
	ConfigSet config2 = config;
	m_renderingQuality = config.getNumber("renderingQuality");
	UVec2 size(m_renderingQuality * F32(m_width), m_renderingQuality * F32(m_height));

	config2.set("width", size.x());
	config2.set("height", size.y());

	m_rDrawToDefaultFb = m_renderingQuality == 1.0;

	m_r.reset(m_alloc.newInstance<Renderer>());
	ANKI_CHECK(m_r->init(
		threadpool, resources, gr, stagingMem, m_alloc, m_frameAlloc, config2, globTimestamp, m_rDrawToDefaultFb));

	// Set the default preprocessor string
	m_materialShaderSource.sprintf(m_alloc,
		"#define ANKI_RENDERER_WIDTH %u\n"
		"#define ANKI_RENDERER_HEIGHT %u\n",
		m_r->getWidth(),
		m_r->getHeight());

	// Init other
	if(!m_rDrawToDefaultFb)
	{
		ANKI_CHECK(m_r->getResourceManager().loadResource("shaders/Final.frag.glsl", m_blitFrag));
		m_r->createDrawQuadShaderProgram(m_blitFrag->getGrShader(), m_blitProg);

		ANKI_R_LOGI("The main renderer will have to blit the offscreen renderer's result");
	}

	ANKI_R_LOGI("Main renderer initialized. Rendering size %ux%u", m_width, m_height);

	return ErrorCode::NONE;
}

Error MainRenderer::render(SceneGraph& scene)
{
	ANKI_TRACE_START_EVENT(RENDER);

	// First thing, reset the temp mem pool
	m_frameAlloc.getMemoryPool().reset();

	GrManager& gl = m_r->getGrManager();
	CommandBufferInitInfo cinf;
	cinf.m_flags =
		CommandBufferFlag::COMPUTE_WORK | CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::TRANSFER_WORK;
	cinf.m_hints = m_cbInitHints;
	CommandBufferPtr cmdb = gl.newInstance<CommandBuffer>(cinf);

	// Run renderer
	RenderingContext ctx(m_frameAlloc);

	if(m_rDrawToDefaultFb)
	{
		ctx.m_outFb = m_defaultFb;
		ctx.m_outFbWidth = m_width;
		ctx.m_outFbHeight = m_height;
	}

	ctx.m_commandBuffer = cmdb;
	const FrustumComponent& frc = scene.getActiveCamera().getComponent<FrustumComponent>();
	ctx.m_visResults = &frc.getVisibilityTestResults();
	ctx.m_viewMat = frc.getViewMatrix();
	ctx.m_projMat = frc.getProjectionMatrix();
	ctx.m_viewProjMat = frc.getViewProjectionMatrix();
	ctx.m_camTrfMat = Mat4(frc.getFrustum().getTransform());
	ctx.m_near = frc.getFrustum().getNear();
	ctx.m_far = frc.getFrustum().getFar();
	ctx.m_unprojParams = ctx.m_projMat.extractPerspectiveUnprojectionParams();
	ANKI_CHECK(m_r->render(ctx));

	// Blit renderer's result to default FB if needed
	if(!m_rDrawToDefaultFb)
	{
		cmdb->beginRenderPass(m_defaultFb);
		cmdb->setViewport(0, 0, m_width, m_height);

		cmdb->bindShaderProgram(m_blitProg);
		cmdb->bindTexture(0, 0, m_r->getPps().getRt());

		m_r->drawQuad(cmdb);
		cmdb->endRenderPass();
	}

	// Flush the command buffer
	cmdb->flush();

	// Set the hints
	m_cbInitHints = cmdb->computeInitHints();

	ANKI_TRACE_STOP_EVENT(RENDER);

	return ErrorCode::NONE;
}

Dbg& MainRenderer::getDbg()
{
	return m_r->getDbg();
}

F32 MainRenderer::getAspectRatio() const
{
	return m_r->getAspectRatio();
}

} // end namespace anki
