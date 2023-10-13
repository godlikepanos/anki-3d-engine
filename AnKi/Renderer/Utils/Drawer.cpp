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
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Scene/RenderStateBucket.h>

namespace anki {

static StatCounter g_executedDrawcallsStatVar(StatCategory::kRenderer, "Drawcalls executed", StatFlag::kZeroEveryFrame);
static StatCounter g_maxDrawcallsStatVar(StatCategory::kRenderer, "Drawcalls possible", StatFlag::kZeroEveryFrame);
static StatCounter g_renderedPrimitivesStatVar(StatCategory::kRenderer, "Rendered primitives", StatFlag::kZeroEveryFrame);

RenderableDrawer::~RenderableDrawer()
{
}

Error RenderableDrawer::init()
{
#if ANKI_STATS_ENABLED
	constexpr Array<MutatorValue, 3> kColorAttachmentCounts = {0, 1, 4};

	U32 count = 0;
	for(MutatorValue attachmentCount : kColorAttachmentCounts)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/DrawerStats.ankiprogbin", Array<SubMutation, 1>{{{"COLOR_ATTACHMENT_COUNT", attachmentCount}}},
									 m_stats.m_statsProg, m_stats.m_updateStatsGrProgs[count]));
		++count;
	}
#endif

	return Error::kNone;
}

void RenderableDrawer::setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	// Allocate, set and bind global uniforms
	{
		MaterialGlobalConstants* globalUniforms;
		const RebarAllocation globalUniformsToken = RebarTransientMemoryPool::getSingleton().allocateFrame(1, globalUniforms);

		globalUniforms->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalUniforms->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalUniforms->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalUniforms->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalUniforms->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalUniforms->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		cmdb.bindConstantBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGlobalConstants), globalUniformsToken);
	}

	// More globals
	cmdb.bindAllBindless(U32(MaterialSet::kBindless));
	cmdb.bindSampler(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTrilinearRepeatSampler), args.m_sampler);
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kGpuScene), &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

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
	ANKI_ASSERT(args.m_viewport != UVec4(0u));

	if(RenderStateBucketContainer::getSingleton().getBucketCount(args.m_renderingTechinuqe) == 0) [[unlikely]]
	{
		return;
	}

#if ANKI_STATS_ENABLED
	U32 variant = 0;
	switch(args.m_renderingTechinuqe)
	{
	case RenderingTechnique::kGBuffer:
		variant = 2;
		break;
	case RenderingTechnique::kForward:
		variant = 1;
		break;
	case RenderingTechnique::kDepth:
		variant = 0;
		break;
	default:
		ANKI_ASSERT(0);
	}

	{
		constexpr U32 kFragmentThreadCount = 16;
		using StatsArray = Array<U32, kFragmentThreadCount * 2>;

		LockGuard lock(m_stats.m_mtx);

		if(m_stats.m_frameIdx != getRenderer().getFrameCount())
		{
			m_stats.m_frameIdx = getRenderer().getFrameCount();

			// Get previous stats
			StatsArray prevFrameStats;
			PtrSize dataRead;
			getRenderer().getReadbackManager().readMostRecentData(m_stats.m_readback, &prevFrameStats, sizeof(prevFrameStats), dataRead);
			if(dataRead > 0) [[likely]]
			{
				U32 drawCount = 0;
				U32 primitiveCount = 0;
				for(U32 tid = 0; tid < kFragmentThreadCount; ++tid)
				{
					drawCount += prevFrameStats[tid];
					primitiveCount += prevFrameStats[kFragmentThreadCount + tid] / 3;
				}

				g_executedDrawcallsStatVar.set(drawCount);
				g_renderedPrimitivesStatVar.set(primitiveCount);
			}

			// Get place to write new stats
			getRenderer().getReadbackManager().allocateData(m_stats.m_readback, sizeof(prevFrameStats), m_stats.m_statsBuffer,
															m_stats.m_statsBufferOffset);

			// Allocate another atomic to count the passes. Do that because the calls to drawMdi might not be in the same order as they run on the GPU
			U32* counter;
			m_stats.m_threadCountBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame(sizeof(U32), counter);
			*counter = 0;
		}

		cmdb.pushDebugMarker("Draw stats", Vec3(0.0f, 1.0f, 0.0f));

		cmdb.bindShaderProgram(m_stats.m_updateStatsGrProgs[variant].get());

		cmdb.bindUavBuffer(0, 0, args.m_mdiDrawCountsBuffer);
		cmdb.bindUavBuffer(0, 1, args.m_drawIndexedIndirectArgsBuffer);

		DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> offsets(&getRenderer().getFrameMemoryPool());
		U32 allUserCount = 0;
		RenderStateBucketContainer::getSingleton().iterateBuckets(args.m_renderingTechinuqe,
																  [&]([[maybe_unused]] const RenderStateInfo& state, U32 userCount) {
																	  offsets.emplaceBack(allUserCount);
																	  allUserCount += userCount;
																  });
		U32* firstDrawArgIndices;
		BufferOffsetRange firstDrawArgIndicesBuffer = RebarTransientMemoryPool::getSingleton().allocateFrame(offsets.getSize(), firstDrawArgIndices);
		memcpy(firstDrawArgIndices, &offsets[0], offsets.getSizeInBytes());
		cmdb.bindUavBuffer(0, 2, firstDrawArgIndicesBuffer);

		cmdb.bindUavBuffer(0, 3, m_stats.m_statsBuffer, m_stats.m_statsBufferOffset, sizeof(StatsArray));
		cmdb.bindUavBuffer(0, 4, m_stats.m_threadCountBuffer);

		cmdb.setPushConstants(&args.m_viewport, sizeof(args.m_viewport));

		cmdb.draw(PrimitiveTopology::kTriangles, 6);

		cmdb.popDebugMarker();
	}
#endif

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

	g_maxDrawcallsStatVar.increment(allUserCount);
}

} // end namespace anki
