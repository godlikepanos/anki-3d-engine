// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Drawer.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/resource/MaterialResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/Trace.h>
#include <anki/util/Logger.h>

namespace anki
{

/// Drawer's context
class DrawContext
{
public:
	RenderQueueDrawContext m_queueCtx;

	const RenderableQueueElement* m_renderableElement = nullptr;

	Array<RenderableQueueElement, MAX_INSTANCES> m_cachedRenderElements;
	Array<U8, MAX_INSTANCES> m_cachedRenderElementLods;
	Array<const void*, MAX_INSTANCES> m_userData;
	U m_cachedRenderElementCount = 0;
};

/// Check if the drawcalls can be merged.
static Bool canMergeRenderableQueueElements(const RenderableQueueElement& a, const RenderableQueueElement& b)
{
	return a.m_callback == b.m_callback && a.m_mergeKey != 0 && a.m_mergeKey == b.m_mergeKey;
}

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::drawRange(Pass pass,
	const Mat4& viewMat,
	const Mat4& viewProjMat,
	CommandBufferPtr cmdb,
	const RenderableQueueElement* begin,
	const RenderableQueueElement* end)
{
	ANKI_ASSERT(begin && end && begin < end);

	DrawContext ctx;
	ctx.m_queueCtx.m_viewMatrix = viewMat;
	ctx.m_queueCtx.m_viewProjectionMatrix = viewProjMat;
	ctx.m_queueCtx.m_projectionMatrix = Mat4::getIdentity(); // TODO
	ctx.m_queueCtx.m_cameraTransform = ctx.m_queueCtx.m_viewMatrix.getInverse();
	ctx.m_queueCtx.m_stagingGpuAllocator = &m_r->getStagingGpuMemoryManager();
	ctx.m_queueCtx.m_commandBuffer = cmdb;
	ctx.m_queueCtx.m_key = RenderingKey(pass, 0, 1);
	ctx.m_queueCtx.m_debugDraw = false;

	for(; begin != end; ++begin)
	{
		ctx.m_renderableElement = begin;

		drawSingle(ctx);
	}

	// Flush the last drawcall
	flushDrawcall(ctx);
}

void RenderableDrawer::flushDrawcall(DrawContext& ctx)
{
	ctx.m_queueCtx.m_key.m_lod = ctx.m_cachedRenderElementLods[0];
	ctx.m_queueCtx.m_key.m_instanceCount = ctx.m_cachedRenderElementCount;

	ctx.m_cachedRenderElements[0].m_callback(
		ctx.m_queueCtx, ConstWeakArray<void*>(const_cast<void**>(&ctx.m_userData[0]), ctx.m_cachedRenderElementCount));

	// Rendered something, reset the cached transforms
	if(ctx.m_cachedRenderElementCount > 1)
	{
		ANKI_TRACE_INC_COUNTER(RENDERER_MERGED_DRAWCALLS, ctx.m_cachedRenderElementCount - 1);
	}
	ctx.m_cachedRenderElementCount = 0;
}

void RenderableDrawer::drawSingle(DrawContext& ctx)
{
	if(ctx.m_cachedRenderElementCount == MAX_INSTANCES)
	{
		flushDrawcall(ctx);
	}

	const RenderableQueueElement& rqel = *ctx.m_renderableElement;

	U8 lod;
	if(ctx.m_queueCtx.m_key.m_pass == Pass::SM)
	{
		lod = MAX_LOD_COUNT - 1;
	}
	else
	{
		lod = min<U8>(m_r->calculateLod(rqel.m_distanceFromCamera), MAX_LOD_COUNT - 1);
	}

	const Bool shouldFlush =
		ctx.m_cachedRenderElementCount > 0
		&& (!canMergeRenderableQueueElements(ctx.m_cachedRenderElements[ctx.m_cachedRenderElementCount - 1], rqel)
			   || ctx.m_cachedRenderElementLods[ctx.m_cachedRenderElementCount - 1] != lod);

	if(shouldFlush)
	{
		flushDrawcall(ctx);
	}

	// Cache the new one
	ctx.m_cachedRenderElements[ctx.m_cachedRenderElementCount] = rqel;
	ctx.m_cachedRenderElementLods[ctx.m_cachedRenderElementCount] = lod;
	ctx.m_userData[ctx.m_cachedRenderElementCount] = rqel.m_userData;
	++ctx.m_cachedRenderElementCount;
}

} // end namespace anki
