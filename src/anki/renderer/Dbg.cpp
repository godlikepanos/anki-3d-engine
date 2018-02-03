// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Dbg.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/DebugDrawer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/Scene.h>
#include <anki/util/Logger.h>
#include <anki/util/Enum.h>
#include <anki/misc/ConfigSet.h>
#include <anki/collision/ConvexHullShape.h>

namespace anki
{

Dbg::Dbg(Renderer* r)
	: RendererObject(r)
{
}

Dbg::~Dbg()
{
	if(m_drawer != nullptr)
	{
		getAllocator().deleteInstance(m_drawer);
	}
}

Error Dbg::init(const ConfigSet& initializer)
{
	m_enabled = initializer.getNumber("r.dbg.enabled");
	return Error::NONE;
}

Error Dbg::lazyInit()
{
	ANKI_ASSERT(!m_initialized);

	// RT descr
	m_rtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth(),
		m_r->getHeight(),
		DBG_COLOR_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		"Dbg");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fbDescr.bake();

	m_drawer = getAllocator().newInstance<DebugDrawer>();
	ANKI_CHECK(m_drawer->init(m_r));

	return Error::NONE;
}

Bool Dbg::getDepthTestEnabled() const
{
	return m_drawer->getDepthTestEnabled();
}

void Dbg::setDepthTestEnabled(Bool enable)
{
	m_drawer->setDepthTestEnabled(enable);
}

void Dbg::switchDepthTestEnabled()
{
	Bool enabled = m_drawer->getDepthTestEnabled();
	m_drawer->setDepthTestEnabled(!enabled);
}

void Dbg::run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx)
{
	ANKI_ASSERT(m_enabled);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	if(!m_initialized)
	{
		if(lazyInit())
		{
			return;
		}
		m_initialized = true;
	}

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	m_drawer->prepareFrame(cmdb);
	m_drawer->setViewProjectionMatrix(ctx.m_renderQueue->m_viewProjectionMatrix);
	m_drawer->setModelMatrix(Mat4::getIdentity());
	// m_drawer->drawGrid();

	SceneDebugDrawer sceneDrawer(m_drawer);

	RenderQueueDrawContext dctx;
	dctx.m_viewMatrix = ctx.m_renderQueue->m_viewMatrix;
	dctx.m_viewProjectionMatrix = ctx.m_renderQueue->m_viewProjectionMatrix;
	dctx.m_projectionMatrix = Mat4::getIdentity(); // TODO
	dctx.m_cameraTransform = ctx.m_renderQueue->m_viewMatrix.getInverse();
	dctx.m_stagingGpuAllocator = &m_r->getStagingGpuMemoryManager();
	dctx.m_commandBuffer = cmdb;
	dctx.m_key = RenderingKey(Pass::GB_FS, 0, 1);
	dctx.m_debugDraw = true;

	for(const RenderableQueueElement& el : ctx.m_renderQueue->m_renderables)
	{
		Array<void*, 1> a = {{const_cast<void*>(el.m_userData)}};
		el.m_callback(dctx, {&a[0], 1});
	}

	for(const PointLightQueueElement& plight : ctx.m_renderQueue->m_pointLights)
	{
		sceneDrawer.draw(plight);
	}

	m_drawer->finishFrame();
}

void Dbg::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("DBG");

	pass.setWork(runCallback, this, 0);
	pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rt}}, m_r->getGBuffer().getDepthRt());

	pass.newConsumer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	pass.newConsumer({m_r->getGBuffer().getDepthRt(),
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ});
	pass.newProducer({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
}

} // end namespace anki
