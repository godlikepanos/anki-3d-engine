// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/core/ConfigSet.h>
#include <anki/collision/ConvexHullShape.h>

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
	m_enabled = initializer.getBool("r_dbgEnabled");
	return Error::NONE;
}

Error Dbg::lazyInit()
{
	ANKI_ASSERT(!m_initialized);

	// RT descr
	m_rtDescr = m_r->create2DRenderTargetDescription(m_r->getWidth(), m_r->getHeight(),
													 DBG_COLOR_ATTACHMENT_PIXEL_FORMAT, "Dbg");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::CLEAR;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::LOAD;
	m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::DONT_CARE;
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

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);

	// Set the context
	RenderQueueDrawContext dctx;
	dctx.m_viewMatrix = ctx.m_renderQueue->m_viewMatrix;
	dctx.m_viewProjectionMatrix = ctx.m_renderQueue->m_viewProjectionMatrix;
	dctx.m_projectionMatrix = ctx.m_renderQueue->m_projectionMatrix;
	dctx.m_cameraTransform = ctx.m_renderQueue->m_viewMatrix.getInverse();
	dctx.m_stagingGpuAllocator = &m_r->getStagingGpuMemoryManager();
	dctx.m_frameAllocator = ctx.m_tempAllocator;
	dctx.m_commandBuffer = cmdb;
	dctx.m_sampler = m_r->getSamplers().m_trilinearRepeatAniso;
	dctx.m_key = RenderingKey(Pass::FS, 0, 1, false, false);
	dctx.m_debugDraw = true;
	dctx.m_debugDrawFlags = m_debugDrawFlags;

	// Draw renderables
	const U32 threadId = rgraphCtx.m_currentSecondLevelCommandBufferIndex;
	const U32 threadCount = rgraphCtx.m_secondLevelCommandBufferCount;
	const U32 problemSize = ctx.m_renderQueue->m_renderables.getSize();
	U32 start, end;
	splitThreadedProblem(threadId, threadCount, problemSize, start, end);

	for(U32 i = start; i < end; ++i)
	{
		const RenderableQueueElement& el = ctx.m_renderQueue->m_renderables[i];
		Array<void*, 1> a = {const_cast<void*>(el.m_userData)};
		el.m_callback(dctx, a);
	}

	// Draw probes
	if(threadId == 0)
	{
		for(const GlobalIlluminationProbeQueueElement& el : ctx.m_renderQueue->m_giProbes)
		{
			Array<void*, 1> a = {const_cast<void*>(el.m_debugDrawCallbackUserData)};
			el.m_debugDrawCallback(dctx, a);
		}
	}

	// Draw lights
	if(threadId == 0)
	{
		U32 count = ctx.m_renderQueue->m_pointLights.getSize();
		while(count--)
		{
			const PointLightQueueElement& el = ctx.m_renderQueue->m_pointLights[count];
			Array<void*, 1> a = {const_cast<void*>(el.m_debugDrawCallbackUserData)};
			el.m_debugDrawCallback(dctx, a);
		}

		for(const SpotLightQueueElement& el : ctx.m_renderQueue->m_spotLights)
		{
			Array<void*, 1> a = {const_cast<void*>(el.m_debugDrawCallbackUserData)};
			el.m_debugDrawCallback(dctx, a);
		}
	}

	// Decals
	if(threadId == 0)
	{
		for(const DecalQueueElement& el : ctx.m_renderQueue->m_decals)
		{
			Array<void*, 1> a = {const_cast<void*>(el.m_debugDrawCallbackUserData)};
			el.m_debugDrawCallback(dctx, a);
		}
	}

	// Reflection probes
	if(threadId == 0)
	{
		for(const ReflectionProbeQueueElement& el : ctx.m_renderQueue->m_reflectionProbes)
		{
			Array<void*, 1> a = {const_cast<void*>(el.m_debugDrawCallbackUserData)};
			el.m_debugDrawCallback(dctx, a);
		}
	}

	// GI probes
	if(threadId == 0)
	{
		for(const GlobalIlluminationProbeQueueElement& el : ctx.m_renderQueue->m_giProbes)
		{
			Array<void*, 1> a = {const_cast<void*>(el.m_debugDrawCallbackUserData)};
			el.m_debugDrawCallback(dctx, a);
		}
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

	pass.setWork(
		[](RenderPassWorkContext& rgraphCtx) {
			Dbg* self = static_cast<Dbg*>(rgraphCtx.m_userData);
			self->run(rgraphCtx, *self->m_runCtx.m_ctx);
		},
		this, computeNumberOfSecondLevelCommandBuffers(ctx.m_renderQueue->m_renderables.getSize()));

	pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt}, m_r->getGBuffer().getDepthRt());

	pass.newDependency({m_runCtx.m_rt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	pass.newDependency({m_r->getGBuffer().getDepthRt(),
						TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ});
}

} // end namespace anki
