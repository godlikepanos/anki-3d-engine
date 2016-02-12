// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ms.h>
#include <anki/renderer/Renderer.h>
#include <anki/util/Logger.h>
#include <anki/scene/Camera.h>
#include <anki/scene/SceneGraph.h>
#include <anki/misc/ConfigSet.h>
#include <anki/core/Trace.h>

namespace anki
{

//==============================================================================
const Array<PixelFormat, Ms::ATTACHMENT_COUNT> Ms::RT_PIXEL_FORMATS = {
	{PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM),
		PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM),
		PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM)}};

const PixelFormat Ms::DEPTH_RT_PIXEL_FORMAT(
	ComponentFormat::D24, TransformFormat::FLOAT);

//==============================================================================
Ms::~Ms()
{
	m_secondLevelCmdbs.destroy(getAllocator());
}

//==============================================================================
Error Ms::createRt(U32 samples)
{
	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		DEPTH_RT_PIXEL_FORMAT,
		samples,
		SamplingFilter::NEAREST,
		log2(SSAO_FRACTION) + 1,
		m_depthRt);

	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		RT_PIXEL_FORMATS[0],
		samples,
		SamplingFilter::NEAREST,
		1,
		m_rt0);

	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		RT_PIXEL_FORMATS[1],
		samples,
		SamplingFilter::NEAREST,
		1,
		m_rt1);

	m_r->createRenderTarget(m_r->getWidth(),
		m_r->getHeight(),
		RT_PIXEL_FORMATS[2],
		samples,
		SamplingFilter::NEAREST,
		log2(SSAO_FRACTION) + 1,
		m_rt2);

	AttachmentLoadOperation loadop = AttachmentLoadOperation::DONT_CARE;
#if ANKI_DEBUG
	loadop = AttachmentLoadOperation::CLEAR;
#endif

	FramebufferInitInfo fbInit;
	fbInit.m_colorAttachmentsCount = ATTACHMENT_COUNT;
	fbInit.m_colorAttachments[0].m_texture = m_rt0;
	fbInit.m_colorAttachments[0].m_loadOperation = loadop;
	fbInit.m_colorAttachments[0].m_clearValue.m_colorf = {{1.0, 0.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[1].m_texture = m_rt1;
	fbInit.m_colorAttachments[1].m_loadOperation = loadop;
	fbInit.m_colorAttachments[1].m_clearValue.m_colorf = {{0.0, 1.0, 0.0, 0.0}};
	fbInit.m_colorAttachments[2].m_texture = m_rt2;
	fbInit.m_colorAttachments[2].m_loadOperation = loadop;
	fbInit.m_colorAttachments[2].m_clearValue.m_colorf = {{0.0, 0.0, 1.0, 0.0}};
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation =
		AttachmentLoadOperation::CLEAR;
	fbInit.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth = 1.0;

	m_fb = getGrManager().newInstance<Framebuffer>(fbInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ms::init(const ConfigSet& initializer)
{
	Error err = initInternal(initializer);
	if(err)
	{
		ANKI_LOGE("Failed to initialize material stage");
	}

	return err;
}

//==============================================================================
Error Ms::initInternal(const ConfigSet& initializer)
{
	ANKI_CHECK(createRt(initializer.getNumber("samples")));

	m_secondLevelCmdbs.create(
		getAllocator(), m_r->getThreadPool().getThreadsCount());

	getGrManager().finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Ms::buildCommandBuffers(
	RenderingContext& ctx, U threadId, U threadCount) const
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	// Get some stuff
	VisibilityTestResults& vis =
		ctx.m_frustumComponent->getVisibilityTestResults();

	U problemSize = vis.getCount(VisibilityGroupType::RENDERABLES_MS);
	PtrSize start, end;
	ThreadPool::Task::choseStartEnd(
		threadId, threadCount, problemSize, start, end);

	if(start == end)
	{
		// Early exit
		return ErrorCode::NONE;
	}

	// Create the command buffer and set some state
	CommandBufferInitInfo cinf;
	cinf.m_secondLevel = true;
	cinf.m_framebuffer = m_fb;
	CommandBufferPtr cmdb =
		m_r->getGrManager().newInstance<CommandBuffer>(cinf);
	ctx.m_ms.m_commandBuffers[threadId] = cmdb;
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->setPolygonOffset(0.0, 0.0);

	// Start drawing
	Error err = m_r->getSceneDrawer().drawRange(Pass::MS_FS,
		*ctx.m_frustumComponent,
		cmdb,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + start,
		vis.getBegin(VisibilityGroupType::RENDERABLES_MS) + end);

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
	return err;
}

//==============================================================================
void Ms::run(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_MS);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->beginRenderPass(m_fb);

	// Set some state anyway because other stages may depend on it
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->setPolygonOffset(0.0, 0.0);

	for(U i = 0; i < m_r->getThreadPool().getThreadsCount(); ++i)
	{
		if(ctx.m_ms.m_commandBuffers[i].isCreated())
		{
			cmdb->pushSecondLevelCommandBuffer(ctx.m_ms.m_commandBuffers[i]);
		}
	}

	ANKI_TRACE_STOP_EVENT(RENDER_MS);
}

} // end namespace anki
