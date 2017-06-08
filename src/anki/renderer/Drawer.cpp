// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Drawer.h>
#include <anki/resource/ShaderResource.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/resource/Material.h>
#include <anki/scene/RenderComponent.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/Trace.h>
#include <anki/util/Logger.h>

namespace anki
{

class CompleteRenderingBuildInfo
{
public:
	F32 m_flod = 0.0;
	RenderComponent* m_rc = nullptr;
	RenderingBuildInfoIn m_in;
	RenderingBuildInfoOut m_out;
};

/// Drawer's context
class DrawContext
{
public:
	Pass m_pass;
	Mat4 m_viewMat;
	Mat4 m_viewProjMat;
	CommandBufferPtr m_cmdb;

	const VisibleNode* m_visibleNode = nullptr;

	Array<Mat4, MAX_INSTANCES> m_cachedTrfs;
	U m_cachedTrfCount = 0;

	StagingGpuMemoryToken m_uboToken;

	U m_nodeProcessedCount = 0;

	Array<CompleteRenderingBuildInfo, 2> m_buildInfo;
	U m_crntBuildInfo = 0;
};

/// Check if the drawcalls can be merged.
static Bool canMergeBuildInfo(const CompleteRenderingBuildInfo& abi, const CompleteRenderingBuildInfo& bbi)
{
	const RenderingBuildInfoOut& a = abi.m_out;
	const RenderingBuildInfoOut& b = bbi.m_out;

	if(!a.m_hasTransform || !b.m_hasTransform)
	{
		// Cannot merge if there is no transform
		return false;
	}

	ANKI_ASSERT(a.m_hasTransform == b.m_hasTransform);

	if(abi.m_rc->getMaterial().getUuid() != bbi.m_rc->getMaterial().getUuid())
	{
		return false;
	}

	if(a.m_program != b.m_program)
	{
		return false;
	}

	if(a.m_vertexBufferBindingCount != b.m_vertexBufferBindingCount
		|| a.m_vertexAttributeCount != b.m_vertexAttributeCount)
	{
		return false;
	}

	for(U i = 0; i < a.m_vertexBufferBindingCount; ++i)
	{
		if(a.m_vertexBufferBindings[i] != b.m_vertexBufferBindings[i])
		{
			return false;
		}
	}

	for(U i = 0; i < a.m_vertexAttributeCount; ++i)
	{
		if(a.m_vertexAttributes[i] != b.m_vertexAttributes[i])
		{
			return false;
		}
	}

	if(a.m_indexBuffer != b.m_indexBuffer)
	{
		return false;
	}

	if(a.m_indexBufferToken != b.m_indexBufferToken)
	{
		return false;
	}

	// Drawcall
	if(a.m_drawArrays != b.m_drawArrays)
	{
		return false;
	}

	if(a.m_drawArrays && a.m_drawcall.m_arrays != b.m_drawcall.m_arrays)
	{
		return false;
	}

	if(!a.m_drawArrays && a.m_drawcall.m_elements != b.m_drawcall.m_elements)
	{
		return false;
	}

	if(a.m_topology != b.m_topology)
	{
		return false;
	}

	return true;
}

static void resetRenderingBuildInfoOut(RenderingBuildInfoOut& b)
{
	b.m_hasTransform = false;
	b.m_program = {};

	b.m_vertexBufferBindingCount = 0;
	b.m_vertexAttributeCount = 0;

	b.m_indexBuffer = {};
	b.m_indexBufferToken = {};

	b.m_drawcall.m_elements = DrawElementsIndirectInfo();
	b.m_drawArrays = false;
	b.m_topology = PrimitiveTopology::TRIANGLES;
}

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::setupUniforms(DrawContext& ctx, CompleteRenderingBuildInfo& build)
{
	RenderQueueDrawContext dctx;
	dctx.m_cameraTransform = ctx.m_viewMat.getInverse();
	dctx.m_commandBuffer = ctx.m_cmdb;
	dctx.m_key = build.m_in.m_key;
	dctx.m_projectionMatrix = Mat4::getIdentity();
	dctx.m_viewMatrix = ctx.m_viewMat;
	dctx.m_viewProjectionMatrix = ctx.m_viewProjMat;

	build.m_rc->allocateAndSetupUniforms(dctx,
		WeakArray<const Mat4>(&ctx.m_cachedTrfs[0], ctx.m_cachedTrfCount),
		m_r->getStagingGpuMemoryManager(),
		ctx.m_uboToken);
}

Error RenderableDrawer::drawRange(Pass pass,
	const Mat4& viewMat,
	const Mat4& viewProjMat,
	CommandBufferPtr cmdb,
	const VisibleNode* begin,
	const VisibleNode* end)
{
	ANKI_ASSERT(begin && end && begin < end);

	DrawContext ctx;
	ctx.m_viewMat = viewMat;
	ctx.m_viewProjMat = viewProjMat;
	ctx.m_pass = pass;
	ctx.m_cmdb = cmdb;

	for(; begin != end; ++begin)
	{
		ctx.m_visibleNode = begin;

		ANKI_CHECK(drawSingle(ctx));

		++ctx.m_nodeProcessedCount;
	}

	// Flush the last drawcall
	CompleteRenderingBuildInfo& build = ctx.m_buildInfo[!ctx.m_crntBuildInfo];
	ANKI_CHECK(flushDrawcall(ctx, build));

	return ErrorCode::NONE;
}

Error RenderableDrawer::flushDrawcall(DrawContext& ctx, CompleteRenderingBuildInfo& build)
{
	RenderComponent& rc = *build.m_rc;

	// Get the new info with the correct instance count
	if(ctx.m_cachedTrfCount > 1)
	{
		build.m_in.m_key.m_instanceCount = ctx.m_cachedTrfCount;
		ANKI_CHECK(rc.buildRendering(build.m_in, build.m_out));
	}

	// Enqueue uniform state updates
	setupUniforms(ctx, build);

	// Finaly, touch the command buffer
	CommandBufferPtr& cmdb = ctx.m_cmdb;

	cmdb->bindUniformBuffer(0, 0, ctx.m_uboToken.m_buffer, ctx.m_uboToken.m_offset, ctx.m_uboToken.m_range);
	cmdb->bindShaderProgram(build.m_out.m_program);

	for(U i = 0; i < build.m_out.m_vertexBufferBindingCount; ++i)
	{
		const RenderingVertexBufferBinding& binding = build.m_out.m_vertexBufferBindings[i];
		if(binding.m_buffer)
		{
			cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride, binding.m_stepRate);
		}
		else
		{
			ANKI_ASSERT(!!(binding.m_token));
			cmdb->bindVertexBuffer(
				i, binding.m_token.m_buffer, binding.m_token.m_offset, binding.m_stride, binding.m_stepRate);
		}
	}

	for(U i = 0; i < build.m_out.m_vertexAttributeCount; ++i)
	{
		const RenderingVertexAttributeInfo& attrib = build.m_out.m_vertexAttributes[i];

		cmdb->setVertexAttribute(i, attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);
	}

	if(!build.m_out.m_drawArrays)
	{
		const DrawElementsIndirectInfo& drawc = build.m_out.m_drawcall.m_elements;

		if(build.m_out.m_indexBuffer)
		{
			cmdb->bindIndexBuffer(build.m_out.m_indexBuffer, 0, IndexType::U16);
		}
		else
		{
			ANKI_ASSERT(!!(build.m_out.m_indexBufferToken));
			cmdb->bindIndexBuffer(
				build.m_out.m_indexBufferToken.m_buffer, build.m_out.m_indexBufferToken.m_offset, IndexType::U16);
		}

		cmdb->drawElements(build.m_out.m_topology,
			drawc.m_count,
			drawc.m_instanceCount,
			drawc.m_firstIndex,
			drawc.m_baseVertex,
			drawc.m_baseInstance);
	}
	else
	{
		const DrawArraysIndirectInfo& drawc = build.m_out.m_drawcall.m_arrays;

		cmdb->drawArrays(
			build.m_out.m_topology, drawc.m_count, drawc.m_instanceCount, drawc.m_first, drawc.m_baseInstance);
	}

	// Rendered something, reset the cached transforms
	if(ctx.m_cachedTrfCount > 1)
	{
		ANKI_TRACE_INC_COUNTER(RENDERER_MERGED_DRAWCALLS, ctx.m_cachedTrfCount - 1);
	}
	ctx.m_cachedTrfCount = 0;

	return ErrorCode::NONE;
}

Error RenderableDrawer::drawSingle(DrawContext& ctx)
{
	// Get components
	RenderComponent& renderable = ctx.m_visibleNode->m_node->getComponent<RenderComponent>();

	// Get info of current
	CompleteRenderingBuildInfo& crntBuild = ctx.m_buildInfo[ctx.m_crntBuildInfo];
	CompleteRenderingBuildInfo& prevBuild = ctx.m_buildInfo[!ctx.m_crntBuildInfo];
	ctx.m_crntBuildInfo = !ctx.m_crntBuildInfo;

	// Fill the crntBuild
	F32 flod = m_r->calculateLod(ctx.m_visibleNode->m_frustumDistance);
	flod = min<F32>(flod, MAX_LOD_COUNT - 1);

	crntBuild.m_rc = &renderable;
	crntBuild.m_flod = flod;

	crntBuild.m_in.m_key.m_lod = flod;
	crntBuild.m_in.m_key.m_pass = ctx.m_pass;
	crntBuild.m_in.m_key.m_instanceCount = 1;
	crntBuild.m_in.m_subMeshIndicesArray = &ctx.m_visibleNode->m_spatialIndices[0];
	crntBuild.m_in.m_subMeshIndicesCount = ctx.m_visibleNode->m_spatialsCount;

	resetRenderingBuildInfoOut(crntBuild.m_out);
	ANKI_CHECK(renderable.buildRendering(crntBuild.m_in, crntBuild.m_out));

	if(ANKI_UNLIKELY(ctx.m_nodeProcessedCount == 0))
	{
		// First drawcall, cache it

		if(crntBuild.m_out.m_hasTransform)
		{
			ctx.m_cachedTrfs[ctx.m_cachedTrfCount++] = crntBuild.m_out.m_transform;
		}
	}
	else if(crntBuild.m_out.m_hasTransform && canMergeBuildInfo(crntBuild, prevBuild)
		&& ctx.m_cachedTrfCount < MAX_INSTANCES - 1)
	{
		// Can merge, will cache the drawcall and skip the drawcall

		ctx.m_cachedTrfs[ctx.m_cachedTrfCount++] = crntBuild.m_out.m_transform;
	}
	else
	{
		// Cannot merge, flush the previous build info

		ANKI_CHECK(flushDrawcall(ctx, prevBuild));

		// Cache the current build
		if(crntBuild.m_out.m_hasTransform)
		{
			ctx.m_cachedTrfs[ctx.m_cachedTrfCount++] = crntBuild.m_out.m_transform;
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
