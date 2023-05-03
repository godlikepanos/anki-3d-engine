// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Drawer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>
#include <AnKi/Core/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Scene/RenderStateBucket.h>

namespace anki {

/// Drawer's context
class RenderableDrawer::Context
{
public:
	Array<const RenderableQueueElement*, kMaxInstanceCount> m_cachedRenderElements;
	U32 m_cachedRenderElementCount = 0;
	ShaderProgram* m_lastBoundShaderProgram = nullptr;
};

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	// Allocate, set and bind global uniforms
	{
		RebarAllocation globalUniformsToken;
		MaterialGlobalUniforms* globalUniforms = static_cast<MaterialGlobalUniforms*>(
			RebarTransientMemoryPool::getSingleton().allocateFrame(sizeof(MaterialGlobalUniforms), globalUniformsToken));

		globalUniforms->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalUniforms->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalUniforms->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalUniforms->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalUniforms->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalUniforms->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		cmdb.bindUniformBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGlobalUniforms),
							   &RebarTransientMemoryPool::getSingleton().getBuffer(), globalUniformsToken.m_offset, globalUniformsToken.m_range);
	}

	// More globals
	cmdb.bindAllBindless(U32(MaterialSet::kBindless));
	cmdb.bindSampler(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTrilinearRepeatSampler), args.m_sampler);
	cmdb.bindStorageBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGpuScene), &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) \
	cmdb.bindReadOnlyTextureBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kUnifiedGeometry_##fmt), \
								   &UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize, Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

	// Misc
	cmdb.setVertexAttribute(0, 0, Format::kR32G32B32A32_Uint, 0);
	cmdb.bindIndexBuffer(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, IndexType::kU16);
}

void RenderableDrawer::drawRange(const RenderableDrawerArguments& args, const RenderableQueueElement* begin, const RenderableQueueElement* end,
								 CommandBuffer& cmdb)
{
	ANKI_ASSERT(begin && end && begin < end);

	setState(args, cmdb);

	Context ctx;
	for(; begin != end; ++begin)
	{
		drawSingle(begin, ctx, cmdb);
	}

	// Flush the last drawcall
	flushDrawcall(ctx, cmdb);
}

void RenderableDrawer::flushDrawcall(Context& ctx, CommandBuffer& cmdb)
{
	// Instance buffer
	RebarAllocation token;
	GpuSceneRenderablePacked* instances = static_cast<GpuSceneRenderablePacked*>(
		RebarTransientMemoryPool::getSingleton().allocateFrame(sizeof(GpuSceneRenderablePacked) * ctx.m_cachedRenderElementCount, token));
	for(U32 i = 0; i < ctx.m_cachedRenderElementCount; ++i)
	{
		GpuSceneRenderable renderable = {};
		renderable.m_worldTransformsOffset = ctx.m_cachedRenderElements[i]->m_worldTransformsOffset;
		renderable.m_uniformsOffset = ctx.m_cachedRenderElements[i]->m_uniformsOffset;
		renderable.m_geometryOffset = ctx.m_cachedRenderElements[i]->m_geometryOffset;
		renderable.m_boneTransformsOffset = ctx.m_cachedRenderElements[i]->m_boneTransformsOffset;
		instances[i] = packGpuSceneRenderable(renderable);
	}

	cmdb.bindVertexBuffer(0, &RebarTransientMemoryPool::getSingleton().getBuffer(), token.m_offset, sizeof(GpuSceneRenderablePacked),
						  VertexStepRate::kInstance);

	// Set state
	const RenderableQueueElement& firstElement = *ctx.m_cachedRenderElements[0];

	if(firstElement.m_program != ctx.m_lastBoundShaderProgram)
	{
		cmdb.bindShaderProgram(firstElement.m_program);
		ctx.m_lastBoundShaderProgram = firstElement.m_program;
	}

	if(firstElement.m_indexed)
	{
		cmdb.drawIndexed(firstElement.m_primitiveTopology, firstElement.m_indexCount, ctx.m_cachedRenderElementCount, firstElement.m_firstIndex);
	}
	else
	{
		cmdb.draw(firstElement.m_primitiveTopology, firstElement.m_vertexCount, ctx.m_cachedRenderElementCount, firstElement.m_firstVertex);
	}

	// Rendered something, reset the cached transforms
	if(ctx.m_cachedRenderElementCount > 1)
	{
		ANKI_TRACE_INC_COUNTER(RMergedDrawcalls, ctx.m_cachedRenderElementCount - 1);
	}
	ctx.m_cachedRenderElementCount = 0;
}

void RenderableDrawer::drawSingle(const RenderableQueueElement* renderEl, Context& ctx, CommandBuffer& cmdb)
{
	if(ctx.m_cachedRenderElementCount == kMaxInstanceCount)
	{
		flushDrawcall(ctx, cmdb);
	}

	const Bool shouldFlush =
		ctx.m_cachedRenderElementCount > 0 && !ctx.m_cachedRenderElements[ctx.m_cachedRenderElementCount - 1]->canMergeWith(*renderEl);

	if(shouldFlush)
	{
		flushDrawcall(ctx, cmdb);
	}

	// Cache the new one
	ctx.m_cachedRenderElements[ctx.m_cachedRenderElementCount] = renderEl;
	++ctx.m_cachedRenderElementCount;
}

void RenderableDrawer::drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	setState(args, cmdb);

	cmdb.bindVertexBuffer(0, args.m_instaceRateRenderablesBuffer, args.m_instaceRateRenderablesOffset, sizeof(GpuSceneRenderablePacked),
						  VertexStepRate::kInstance);

	U32 allUserCount = 0;
	U32 bucketCount = 0;
	RenderStateBucketContainer::getSingleton().iterateBuckets(args.m_renderingTechinuqe, [&](const RenderStateInfo& state, U32 userCount) {
		if(userCount == 0)
		{
			++bucketCount;
			return;
		}

		ANKI_ASSERT(state.m_indexedDrawcall && "TODO non-indexed");

		ShaderProgramPtr prog = state.m_program;
		cmdb.bindShaderProgram(prog.get());

		const U32 maxDrawCount = userCount;

		cmdb.drawIndexedIndirectCount(state.m_primitiveTopology, args.m_drawIndexedIndirectArgsBuffer,
									  args.m_drawIndexedIndirectArgsBufferOffset + sizeof(DrawIndexedIndirectArgs) * allUserCount,
									  args.m_mdiDrawCountsBuffer, args.m_mdiDrawCountsBufferOffset + sizeof(U32) * bucketCount, maxDrawCount);

		++bucketCount;
		allUserCount += userCount;
	});

	ANKI_ASSERT(bucketCount == RenderStateBucketContainer::getSingleton().getBucketCount(args.m_renderingTechinuqe));
}

} // end namespace anki
