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

namespace anki {

/// Drawer's context
class RenderableDrawer::Context
{
public:
	CommandBufferPtr m_commandBuffer;

	Array<const RenderableQueueElement*, kMaxInstanceCount> m_cachedRenderElements;
	U32 m_cachedRenderElementCount = 0;
	ShaderProgram* m_lastBoundShaderProgram = nullptr;
};

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::drawRange(const RenderableDrawerArguments& args, const RenderableQueueElement* begin,
								 const RenderableQueueElement* end, CommandBufferPtr& cmdb)
{
	ANKI_ASSERT(begin && end && begin < end);

	// Allocate, set and bind global uniforms
	{
		RebarGpuMemoryToken globalUniformsToken;
		MaterialGlobalUniforms* globalUniforms =
			static_cast<MaterialGlobalUniforms*>(RebarStagingGpuMemoryPool::getSingleton().allocateFrame(
				sizeof(MaterialGlobalUniforms), globalUniformsToken));

		globalUniforms->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalUniforms->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalUniforms->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalUniforms->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalUniforms->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalUniforms->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		cmdb->bindUniformBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGlobalUniforms),
								RebarStagingGpuMemoryPool::getSingleton().getBuffer(), globalUniformsToken.m_offset,
								globalUniformsToken.m_range);
	}

	// More globals
	cmdb->bindAllBindless(U32(MaterialSet::kBindless));
	cmdb->bindSampler(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTrilinearRepeatSampler), args.m_sampler);
	cmdb->bindStorageBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGpuScene),
							GpuSceneMemoryPool::getSingleton().getBuffer(), 0, kMaxPtrSize);

#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) \
	cmdb->bindReadOnlyTextureBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kUnifiedGeometry_##fmt), \
									UnifiedGeometryMemoryPool::getSingleton().getBuffer(), 0, kMaxPtrSize, \
									Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

	// Misc
	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32A32_Uint, 0);
	cmdb->bindIndexBuffer(UnifiedGeometryMemoryPool::getSingleton().getBuffer(), 0, IndexType::kU16);

	// Set a few things
	Context ctx;
	ctx.m_commandBuffer = cmdb;

	for(; begin != end; ++begin)
	{
		drawSingle(begin, ctx);
	}

	// Flush the last drawcall
	flushDrawcall(ctx);
}

void RenderableDrawer::flushDrawcall(Context& ctx)
{
	CommandBufferPtr cmdb = ctx.m_commandBuffer;

	// Instance buffer
	RebarGpuMemoryToken token;
	GpuSceneRenderablePacked* instances =
		static_cast<GpuSceneRenderablePacked*>(RebarStagingGpuMemoryPool::getSingleton().allocateFrame(
			sizeof(GpuSceneRenderablePacked) * ctx.m_cachedRenderElementCount, token));
	for(U32 i = 0; i < ctx.m_cachedRenderElementCount; ++i)
	{
		GpuSceneRenderable renderable = {};
		renderable.m_worldTransformsOffset = ctx.m_cachedRenderElements[i]->m_worldTransformsOffset;
		renderable.m_uniformsOffset = ctx.m_cachedRenderElements[i]->m_uniformsOffset;
		renderable.m_geometryOffset = ctx.m_cachedRenderElements[i]->m_geometryOffset;
		renderable.m_boneTransformsOffset = ctx.m_cachedRenderElements[i]->m_boneTransformsOffset;
		instances[i] = packGpuSceneRenderable(renderable);
	}

	cmdb->bindVertexBuffer(0, RebarStagingGpuMemoryPool::getSingleton().getBuffer(), token.m_offset,
						   sizeof(GpuSceneRenderablePacked), VertexStepRate::kInstance);

	// Set state
	const RenderableQueueElement& firstElement = *ctx.m_cachedRenderElements[0];

	if(firstElement.m_program != ctx.m_lastBoundShaderProgram)
	{
		cmdb->bindShaderProgram(ShaderProgramPtr(firstElement.m_program));
		ctx.m_lastBoundShaderProgram = firstElement.m_program;
	}

	if(firstElement.m_indexed)
	{
		cmdb->drawElements(firstElement.m_primitiveTopology, firstElement.m_indexCount, ctx.m_cachedRenderElementCount,
						   firstElement.m_firstIndex);
	}
	else
	{
		cmdb->drawArrays(firstElement.m_primitiveTopology, firstElement.m_vertexCount, ctx.m_cachedRenderElementCount,
						 firstElement.m_firstVertex);
	}

	// Rendered something, reset the cached transforms
	if(ctx.m_cachedRenderElementCount > 1)
	{
		ANKI_TRACE_INC_COUNTER(RMergedDrawcalls, ctx.m_cachedRenderElementCount - 1);
	}
	ctx.m_cachedRenderElementCount = 0;
}

void RenderableDrawer::drawSingle(const RenderableQueueElement* renderEl, Context& ctx)
{
	if(ctx.m_cachedRenderElementCount == kMaxInstanceCount)
	{
		flushDrawcall(ctx);
	}

	const Bool shouldFlush =
		ctx.m_cachedRenderElementCount > 0
		&& !ctx.m_cachedRenderElements[ctx.m_cachedRenderElementCount - 1]->canMergeWith(*renderEl);

	if(shouldFlush)
	{
		flushDrawcall(ctx);
	}

	// Cache the new one
	ctx.m_cachedRenderElements[ctx.m_cachedRenderElementCount] = renderEl;
	++ctx.m_cachedRenderElementCount;
}

} // end namespace anki
