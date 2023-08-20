// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/Drawer.h>
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

RenderableDrawer::~RenderableDrawer()
{
}

void RenderableDrawer::setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	// Allocate, set and bind global uniforms
	{
		MaterialGlobalUniforms* globalUniforms;
		const RebarAllocation globalUniformsToken = RebarTransientMemoryPool::getSingleton().allocateFrame(1, globalUniforms);

		globalUniforms->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalUniforms->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalUniforms->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalUniforms->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalUniforms->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalUniforms->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		cmdb.bindUniformBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGlobalUniforms), globalUniformsToken);
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

void RenderableDrawer::drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	setState(args, cmdb);

	cmdb.bindVertexBuffer(0, args.m_instanceRateRenderablesBuffer.m_buffer, args.m_instanceRateRenderablesBuffer.m_offset,
						  sizeof(GpuSceneRenderableVertex), VertexStepRate::kInstance);

	U32 allUserCount = 0;
	U32 bucketCount = 0;
	RenderStateBucketContainer::getSingleton().iterateBuckets(args.m_renderingTechinuqe, [&](const RenderStateInfo& state, U32 userCount) {
		if(userCount == 0)
		{
			++bucketCount;
			return;
		}

		ShaderProgramPtr prog = state.m_program;
		cmdb.bindShaderProgram(prog.get());

		const U32 maxDrawCount = userCount;

		if(state.m_indexedDrawcall)
		{
			cmdb.drawIndexedIndirectCount(state.m_primitiveTopology, args.m_drawIndexedIndirectArgsBuffer.m_buffer,
										  args.m_drawIndexedIndirectArgsBuffer.m_offset + sizeof(DrawIndexedIndirectArgs) * allUserCount,
										  sizeof(DrawIndexedIndirectArgs), args.m_mdiDrawCountsBuffer.m_buffer,
										  args.m_mdiDrawCountsBuffer.m_offset + sizeof(U32) * bucketCount, maxDrawCount);
		}
		else
		{
			// Yes, the DrawIndexedIndirectArgs is intentional
			cmdb.drawIndirectCount(state.m_primitiveTopology, args.m_drawIndexedIndirectArgsBuffer.m_buffer,
								   args.m_drawIndexedIndirectArgsBuffer.m_offset + sizeof(DrawIndexedIndirectArgs) * allUserCount,
								   sizeof(DrawIndexedIndirectArgs), args.m_mdiDrawCountsBuffer.m_buffer,
								   args.m_mdiDrawCountsBuffer.m_offset + sizeof(U32) * bucketCount, maxDrawCount);
		}

		++bucketCount;
		allUserCount += userCount;
	});

	ANKI_ASSERT(bucketCount == RenderStateBucketContainer::getSingleton().getBucketCount(args.m_renderingTechinuqe));
}

} // end namespace anki
