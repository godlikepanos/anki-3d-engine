// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

	RebarStagingGpuMemoryPool* m_rebarStagingPool = nullptr;

	Array<const RenderableQueueElement*, kMaxInstanceCount> m_cachedRenderElements;
	U32 m_cachedRenderElementCount = 0;
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
			static_cast<MaterialGlobalUniforms*>(m_r->getExternalSubsystems().m_rebarStagingPool->allocateFrame(
				sizeof(MaterialGlobalUniforms), globalUniformsToken));

		globalUniforms->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalUniforms->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalUniforms->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalUniforms->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalUniforms->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalUniforms->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		cmdb->bindUniformBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGlobalUniforms),
								m_r->getExternalSubsystems().m_rebarStagingPool->getBuffer(),
								globalUniformsToken.m_offset, globalUniformsToken.m_range);
	}

	// More globals
	cmdb->bindAllBindless(U32(MaterialSet::kBindless));
	cmdb->bindSampler(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTrilinearRepeatSampler), args.m_sampler);
	cmdb->bindStorageBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGpuScene), args.m_gpuSceneBuffer, 0,
							kMaxPtrSize);

#define _ANKI_BIND_TEXTURE_BUFFER(format) \
	cmdb->bindReadOnlyTextureBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kUnifiedGeometry_##format), \
									args.m_unifiedGeometryBuffer, 0, kMaxPtrSize, Format::k##format)

	_ANKI_BIND_TEXTURE_BUFFER(R32_Sfloat);
	_ANKI_BIND_TEXTURE_BUFFER(R32G32_Sfloat);
	_ANKI_BIND_TEXTURE_BUFFER(R32G32B32_Sfloat);
	_ANKI_BIND_TEXTURE_BUFFER(R32G32B32A32_Sfloat);
	_ANKI_BIND_TEXTURE_BUFFER(R16G16B16A16_Unorm);
	_ANKI_BIND_TEXTURE_BUFFER(R8G8B8A8_Snorm);
	_ANKI_BIND_TEXTURE_BUFFER(R8G8B8A8_Uint);

#undef _ANKI_BIND_TEXTURE_BUFFER

	// Misc
	cmdb->setVertexAttribute(0, 0, Format::kR32G32B32A32_Uint, 0);
	cmdb->bindIndexBuffer(args.m_unifiedGeometryBuffer, 0, IndexType::kU16);

	// Set a few things
	Context ctx;
	ctx.m_rebarStagingPool = m_r->getExternalSubsystems().m_rebarStagingPool;
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
	GpuSceneRenderablePacked* instances = static_cast<GpuSceneRenderablePacked*>(ctx.m_rebarStagingPool->allocateFrame(
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

	cmdb->bindVertexBuffer(0, ctx.m_rebarStagingPool->getBuffer(), token.m_offset, sizeof(GpuSceneRenderablePacked),
						   VertexStepRate::kInstance);

	// Set state
	const RenderableQueueElement& firstElement = *ctx.m_cachedRenderElements[0];
	cmdb->bindShaderProgram(ShaderProgramPtr(firstElement.m_program));

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
		ANKI_TRACE_INC_COUNTER(R_MERGED_DRAWCALLS, ctx.m_cachedRenderElementCount - 1);
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
