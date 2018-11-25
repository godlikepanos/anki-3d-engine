// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Dbg.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/Scene.h>
#include <anki/util/Logger.h>
#include <anki/util/Enum.h>
#include <anki/misc/ConfigSet.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/Collision.h>

namespace anki
{

Dbg::Dbg(Renderer* r)
	: RendererObject(r)
{
}

Dbg::~Dbg()
{
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
	m_rtDescr = m_r->create2DRenderTargetDescription(
		m_r->getWidth(), m_r->getHeight(), DBG_COLOR_ATTACHMENT_PIXEL_FORMAT, "Dbg");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fbDescr.bake();

	return Error::NONE;
}

void Dbg::run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx)
{
	ANKI_ASSERT(m_enabled);

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	// Set common state
	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());
	cmdb->setDepthWrite(false);

	rgraphCtx.bindTextureAndSampler(0,
		0,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());

	// Set the context
	RenderQueueDrawContext dctx;
	dctx.m_viewMatrix = ctx.m_renderQueue->m_viewMatrix;
	dctx.m_viewProjectionMatrix = ctx.m_renderQueue->m_viewProjectionMatrix;
	dctx.m_projectionMatrix = Mat4::getIdentity(); // TODO
	dctx.m_cameraTransform = ctx.m_renderQueue->m_viewMatrix.getInverse();
	dctx.m_stagingGpuAllocator = &m_r->getStagingGpuMemoryManager();
	dctx.m_commandBuffer = cmdb;
	dctx.m_key = RenderingKey(Pass::FS, 0, 1, false, false);
	dctx.m_debugDraw = true;
	dctx.m_debugDrawFlags = m_debugDrawFlags;

	// Draw
	for(const RenderableQueueElement& el : ctx.m_renderQueue->m_renderables)
	{
		Array<void*, 1> a = {{const_cast<void*>(el.m_userData)}};
		el.m_callback(dctx, {&a[0], 1});
	}

	{
		DebugDrawer ddrawer;

		ddrawer.init(&m_r->getResourceManager());

		static Mat3 rot(Mat3::getIdentity());
		rot.rotateXAxis(toRad(0.2f));
		rot.rotateYAxis(toRad(0.2f));
		rot.rotateZAxis(toRad(-0.3f));
		static Vec3 pos(0.0f);
		pos += Vec3(0.01, 0, 0);

		ddrawer.prepareFrame(&dctx);

		Array<Vec3, 3> frustumPoints;
		frustumPoints[0] = rot * Vec3(-0.5, 0.2, -0.3) + pos;
		frustumPoints[1] = rot * Vec3(1.5, -.2, 1.3) + pos;
		frustumPoints[2] = rot * Vec3(-1.2, 1.9, -1.3) + pos;

		Vec4 color = Vec4(0, 1, 0, 1);
		ddrawer.setColor(color);
		ddrawer.setModelMatrix(Mat4::getIdentity());
		ddrawer.setViewProjectionMatrix(ctx.m_renderQueue->m_viewProjectionMatrix);

		ddrawer.drawLine(frustumPoints[0], frustumPoints[1], color);
		ddrawer.drawLine(frustumPoints[1], frustumPoints[2], color);
		ddrawer.drawLine(frustumPoints[2], frustumPoints[0], color);

		Vec3 lightDir(-0.5);
		ddrawer.drawLine(lightDir * -10.0, lightDir * 10.0, Vec4(1));

		Vec3 zAxis = lightDir;
		Vec3 yAxis = Vec3(0, 1, 0);
		Vec3 xAxis = zAxis.cross(yAxis).getNormalized();
		yAxis = xAxis.cross(zAxis);

		Mat3 viewRotation;
		viewRotation.setColumns(xAxis, yAxis, zAxis);
		ddrawer.drawLine(Vec3(0.), xAxis, Vec4(1, 0, 0, 1));
		ddrawer.drawLine(Vec3(0.), yAxis, Vec4(0, 1, 0, 1));
		ddrawer.drawLine(Vec3(0.), zAxis, Vec4(0, 0, 1, 1));

		Array<Vec3, 3> frustumPointsLSpace;
		for(int i = 0; i < 3; ++i)
		{
			frustumPointsLSpace[i] = viewRotation.getInverse() * frustumPoints[i];
		}

		/*ddrawer.drawLine(frustumPointsLSpace[0], frustumPointsLSpace[1], Vec4(1, 1, 0, 1));
		ddrawer.drawLine(frustumPointsLSpace[1], frustumPointsLSpace[2], Vec4(1, 1, 0, 1));
		ddrawer.drawLine(frustumPointsLSpace[2], frustumPointsLSpace[0], Vec4(1, 1, 0, 1));*/

		Obb box;
		box.setFromPointCloud(&frustumPointsLSpace[0], 3, sizeof(Vec3), sizeof(frustumPointsLSpace));

		CollisionDebugDrawer cdrawer(&ddrawer);
		box.transform(Transform(Vec4(0.), Mat3x4(viewRotation), 1.0));
		ddrawer.setModelMatrix(Mat4::getIdentity());
		ddrawer.setColor(Vec4(1, 0, 0, 1));
		box.accept(cdrawer);

		ddrawer.finishFrame();
	}
}

void Dbg::populateRenderGraph(RenderingContext& ctx)
{
	if(!m_initialized)
	{
		if(lazyInit())
		{
			return;
		}
		m_initialized = true;
	}

	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("DBG");

	pass.setWork(runCallback, this, 0);
	pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_rt}}, m_r->getGBuffer().getDepthRt());

	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	pass.newDependency({m_r->getGBuffer().getDepthRt(),
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ});
}

} // end namespace anki
