// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Scene.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Collision/ConvexHullShape.h>

namespace anki {

Dbg::Dbg(Renderer* r)
	: RendererObject(r)
{
}

Dbg::~Dbg()
{
}

Error Dbg::init()
{
	ANKI_R_LOGV("Initializing DBG");

	// RT descr
	m_rtDescr = m_r->create2DRenderTargetDescription(m_r->getInternalResolution().x(), m_r->getInternalResolution().y(),
													 Format::kR8G8B8A8_Unorm, "Dbg");
	m_rtDescr.bake();

	// Create FB descr
	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::kClear;
	m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::kLoad;
	m_fbDescr.m_depthStencilAttachment.m_stencilLoadOperation = AttachmentLoadOperation::kDontCare;
	m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::kDepth;
	m_fbDescr.bake();

	return Error::kNone;
}

void Dbg::run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx)
{
	ANKI_ASSERT(getConfig().getRDbgEnabled());

	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	// Set common state
	cmdb->setViewport(0, 0, m_r->getInternalResolution().x(), m_r->getInternalResolution().y());
	cmdb->setDepthWrite(false);

	cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

	rgraphCtx.bindTexture(0, 1, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

	cmdb->setBlendFactors(0, BlendFactor::kSrcAlpha, BlendFactor::kOneMinusSrcAlpha);

	// Set the context
	RenderQueueDrawContext dctx;
	dctx.m_viewMatrix = ctx.m_renderQueue->m_viewMatrix;
	dctx.m_viewProjectionMatrix = ctx.m_renderQueue->m_viewProjectionMatrix;
	dctx.m_projectionMatrix = ctx.m_renderQueue->m_projectionMatrix;
	dctx.m_cameraTransform = ctx.m_renderQueue->m_cameraTransform;
	dctx.m_stagingGpuAllocator = &m_r->getStagingGpuMemory();
	dctx.m_framePool = ctx.m_tempPool;
	dctx.m_commandBuffer = cmdb;
	dctx.m_sampler = m_r->getSamplers().m_trilinearRepeatAniso;
	dctx.m_key = RenderingKey(RenderingTechnique::kForward, 0, 1, false, false);
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

	// Draw forward shaded
	if(threadId == 0)
	{
		for(const RenderableQueueElement& el : ctx.m_renderQueue->m_forwardShadingRenderables)
		{
			Array<void*, 1> a = {const_cast<void*>(el.m_userData)};
			el.m_callback(dctx, a);
		}
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
	if(!getConfig().getRDbgEnabled())
	{
		return;
	}

	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create RT
	m_runCtx.m_rt = rgraph.newRenderTarget(m_rtDescr);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("DBG");

	pass.setWork(computeNumberOfSecondLevelCommandBuffers(ctx.m_renderQueue->m_renderables.getSize()),
				 [this, &ctx](RenderPassWorkContext& rgraphCtx) {
					 run(rgraphCtx, ctx);
				 });

	pass.setFramebufferInfo(m_fbDescr, {m_runCtx.m_rt}, m_r->getGBuffer().getDepthRt());

	pass.newTextureDependency(m_runCtx.m_rt, TextureUsageBit::kFramebufferWrite);
	pass.newTextureDependency(m_r->getGBuffer().getDepthRt(),
							  TextureUsageBit::kSampledFragment | TextureUsageBit::kFramebufferRead);
}

} // end namespace anki
