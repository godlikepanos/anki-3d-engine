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

RenderableDrawer::~RenderableDrawer()
{
}

Error RenderableDrawer::init()
{
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

		ANKI_ASSERT(args.m_viewport != UVec4(0u));
		globalUniforms->m_viewport = Vec4(args.m_viewport);

		globalUniforms->m_enableHzbTesting = args.m_hzbTexture != nullptr;

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

	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kMeshletBoundingVolumes),
					   UnifiedGeometryBuffer::getSingleton().getBufferOffsetRange());
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kMeshletGeometryDescriptors),
					   UnifiedGeometryBuffer::getSingleton().getBufferOffsetRange());
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTaskShaderPayloads), args.m_mesh.m_taskShaderPayloadsBuffer);
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kRenderables),
					   GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kMeshLods), GpuSceneArrays::MeshLod::getSingleton().getBufferOffsetRange());
	cmdb.bindTexture(U32(MaterialSet::kGlobal), U32(MaterialBinding::kHzbTexture),
					 (args.m_hzbTexture) ? args.m_hzbTexture : &getRenderer().getDummyTextureView2d());
	cmdb.bindSampler(U32(MaterialSet::kGlobal), U32(MaterialBinding::kNearestClampSampler), getRenderer().getSamplers().m_nearestNearestClamp.get());

	// Misc
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
	PipelineQueryPtr pplineQuery;

	if(GrManager::getSingleton().getDeviceCapabilities().m_pipelineQuery)
	{
		PipelineQueryInitInfo queryInit("Drawer");
		queryInit.m_type = PipelineQueryType::kPrimitivesPassedClipping;
		pplineQuery = GrManager::getSingleton().newPipelineQuery(queryInit);
		getRenderer().appendPipelineQuery(pplineQuery.get());

		cmdb.beginPipelineQuery(pplineQuery.get());
	}
#endif

	setState(args, cmdb);

	const Bool meshShaderHwSupport = GrManager::getSingleton().getDeviceCapabilities().m_meshShaders;

	// Gather the drawcalls
	class Command
	{
	public:
		class LegacyDraw
		{
		public:
			Buffer* m_drawIndirectArgsBuffer;
			PtrSize m_drawIndirectArgsBufferOffset;
			Buffer* m_mdiDrawCountsBuffer;
			PtrSize m_mdiDrawCountsBufferOffset;
			Buffer* m_instancesBuffer;
			PtrSize m_instancesBufferOffset;
			U32 m_maxDrawCount;
			PrimitiveTopology m_primitiveTopology;
		};

		class MeshDraw
		{
		public:
			U32 m_firstPayload;
			Buffer* m_taskShaderIndirectArgsBuffer;
			PtrSize m_taskShaderIndirectArgsBufferOffset;
		};

		class SwMeshDraw
		{
		public:
			Buffer* m_drawIndirectArgsBuffer;
			PtrSize m_drawIndirectArgsBufferOffset;
			Buffer* m_instancesBuffer;
			PtrSize m_instancesBufferOffset;
		};

		union
		{
			LegacyDraw m_legacyDraw;
			MeshDraw m_meshDraw;
			SwMeshDraw m_swMeshDraw;
		};

		ShaderProgram* m_program;
		U64 m_shaderBinarySize;
		U8 m_drawType;
		Bool m_hasDiscard;
	};

	Array<Command, 16> commands;
	U32 commandCount = 0;

	U32 allUserCount = 0;
	U32 bucketCount = 0;
	U32 allMeshletGroupCount = 0;
	U32 legacyGeometryFlowUserCount = 0;
	PtrSize meshletInstancesBufferOffset = 0;
	RenderStateBucketContainer::getSingleton().iterateBuckets(
		args.m_renderingTechinuqe, [&](const RenderStateInfo& state, U32 userCount, U32 meshletGroupCount, U32 meshletCount) {
			if(userCount == 0)
			{
				++bucketCount;
				return;
			}

			Command& cmd = commands[commandCount++];

			cmd.m_program = state.m_program.get();
			cmd.m_shaderBinarySize = U64(state.m_program->getShaderBinarySize(ShaderType::kFragment)) << 32u;
			cmd.m_hasDiscard = state.m_program->hasDiscard();

			const Bool meshlets = meshletGroupCount > 0;

			if(meshlets && meshShaderHwSupport)
			{
				cmd.m_drawType = 2;
				cmd.m_shaderBinarySize |= state.m_program->getShaderBinarySize(ShaderType::kMesh);

				cmd.m_meshDraw.m_firstPayload = allMeshletGroupCount;
				cmd.m_meshDraw.m_taskShaderIndirectArgsBuffer = args.m_mesh.m_taskShaderIndirectArgsBuffer.m_buffer;
				cmd.m_meshDraw.m_taskShaderIndirectArgsBufferOffset =
					args.m_mesh.m_taskShaderIndirectArgsBuffer.m_offset + sizeof(DispatchIndirectArgs) * bucketCount;

				allMeshletGroupCount += min(meshletGroupCount, kMaxMeshletGroupCountPerRenderStateBucket);
			}
			else if(meshlets)
			{
				cmd.m_drawType = 3;
				cmd.m_shaderBinarySize |= state.m_program->getShaderBinarySize(ShaderType::kVertex);

				cmd.m_swMeshDraw.m_drawIndirectArgsBuffer = args.m_softwareMesh.m_drawIndirectArgsBuffer.m_buffer;
				cmd.m_swMeshDraw.m_drawIndirectArgsBufferOffset =
					args.m_softwareMesh.m_drawIndirectArgsBuffer.m_offset + sizeof(DrawIndirectArgs) * bucketCount;

				cmd.m_swMeshDraw.m_instancesBuffer = args.m_softwareMesh.m_meshletInstancesBuffer.m_buffer;
				cmd.m_swMeshDraw.m_instancesBufferOffset = args.m_softwareMesh.m_meshletInstancesBuffer.m_offset + meshletInstancesBufferOffset;
			}
			else
			{
				const U32 maxDrawCount = userCount;

				cmd.m_drawType = (state.m_indexedDrawcall) ? 0 : 1;
				cmd.m_shaderBinarySize |= state.m_program->getShaderBinarySize(ShaderType::kVertex);

				cmd.m_legacyDraw.m_primitiveTopology = state.m_primitiveTopology;
				cmd.m_legacyDraw.m_drawIndirectArgsBuffer = args.m_legacy.m_drawIndexedIndirectArgsBuffer.m_buffer;
				cmd.m_legacyDraw.m_drawIndirectArgsBufferOffset =
					args.m_legacy.m_drawIndexedIndirectArgsBuffer.m_offset + sizeof(DrawIndexedIndirectArgs) * legacyGeometryFlowUserCount;
				cmd.m_legacyDraw.m_maxDrawCount = maxDrawCount;
				cmd.m_legacyDraw.m_mdiDrawCountsBuffer = args.m_legacy.m_mdiDrawCountsBuffer.m_buffer;
				cmd.m_legacyDraw.m_mdiDrawCountsBufferOffset = args.m_legacy.m_mdiDrawCountsBuffer.m_offset + sizeof(U32) * bucketCount;
				cmd.m_legacyDraw.m_instancesBuffer = args.m_legacy.m_renderableInstancesBuffer.m_buffer;
				cmd.m_legacyDraw.m_instancesBufferOffset =
					args.m_legacy.m_renderableInstancesBuffer.m_offset + legacyGeometryFlowUserCount * sizeof(GpuSceneRenderableInstance);

				legacyGeometryFlowUserCount += userCount;
			}

			++bucketCount;
			allUserCount += userCount;
			meshletInstancesBufferOffset += sizeof(GpuSceneMeshletInstance) * min(meshletCount, kMaxVisibleMeshletsPerRenderStateBucket);
		});

	ANKI_ASSERT(bucketCount == RenderStateBucketContainer::getSingleton().getBucketCount(args.m_renderingTechinuqe));

	// Sort the drawcalls from the least expensive to the most expensive, leave alpha tested at the end
	std::sort(&commands[0], &commands[0] + commandCount, [](const Command& a, const Command& b) {
		if(a.m_hasDiscard != b.m_hasDiscard)
		{
			return !a.m_hasDiscard;
		}
		else
		{
			return a.m_shaderBinarySize < b.m_shaderBinarySize;
		}
	});

	cmdb.setVertexAttribute(0, 0, Format::kR32G32B32A32_Uint, 0);

	// Now draw
	for(const Command* it = commands.getBegin(); it < commands.getBegin() + commandCount; ++it)
	{
		cmdb.bindShaderProgram(it->m_program);

		if(it->m_drawType == 0)
		{
			cmdb.bindVertexBuffer(0, it->m_legacyDraw.m_instancesBuffer, it->m_legacyDraw.m_instancesBufferOffset, sizeof(GpuSceneRenderableInstance),
								  VertexStepRate::kInstance);

			cmdb.drawIndexedIndirectCount(it->m_legacyDraw.m_primitiveTopology, it->m_legacyDraw.m_drawIndirectArgsBuffer,
										  it->m_legacyDraw.m_drawIndirectArgsBufferOffset, sizeof(DrawIndexedIndirectArgs),
										  it->m_legacyDraw.m_mdiDrawCountsBuffer, it->m_legacyDraw.m_mdiDrawCountsBufferOffset,
										  it->m_legacyDraw.m_maxDrawCount);
		}
		else if(it->m_drawType == 1)
		{
			cmdb.bindVertexBuffer(0, it->m_legacyDraw.m_instancesBuffer, it->m_legacyDraw.m_instancesBufferOffset, sizeof(GpuSceneRenderableInstance),
								  VertexStepRate::kInstance);

			// Yes, the DrawIndexedIndirectArgs is intentional
			cmdb.drawIndirectCount(it->m_legacyDraw.m_primitiveTopology, it->m_legacyDraw.m_drawIndirectArgsBuffer,
								   it->m_legacyDraw.m_drawIndirectArgsBufferOffset, sizeof(DrawIndexedIndirectArgs),
								   it->m_legacyDraw.m_mdiDrawCountsBuffer, it->m_legacyDraw.m_mdiDrawCountsBufferOffset,
								   it->m_legacyDraw.m_maxDrawCount);
		}
		else if(it->m_drawType == 2)
		{
			const UVec4 firstPayload(it->m_meshDraw.m_firstPayload);
			cmdb.setPushConstants(&firstPayload, sizeof(firstPayload));

			cmdb.drawMeshTasksIndirect(it->m_meshDraw.m_taskShaderIndirectArgsBuffer, it->m_meshDraw.m_taskShaderIndirectArgsBufferOffset);
		}
		else
		{
			ANKI_ASSERT(it->m_drawType == 3);

			cmdb.bindVertexBuffer(0, it->m_swMeshDraw.m_instancesBuffer, it->m_swMeshDraw.m_instancesBufferOffset, sizeof(GpuSceneMeshletInstance),
								  VertexStepRate::kInstance);

			cmdb.drawIndirect(PrimitiveTopology::kTriangles, 1, it->m_swMeshDraw.m_drawIndirectArgsBufferOffset,
							  it->m_swMeshDraw.m_drawIndirectArgsBuffer);
		}
	}

#if ANKI_STATS_ENABLED
	if(pplineQuery.isCreated())
	{
		cmdb.endPipelineQuery(pplineQuery.get());
	}
#endif
}

} // end namespace anki
