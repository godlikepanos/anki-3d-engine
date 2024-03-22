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
	if(args.m_mesh.m_meshletGroupInstancesBuffer.m_range)
	{
		cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kMeshletGroups), args.m_mesh.m_meshletGroupInstancesBuffer);
	}
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kRenderables),
					   GpuSceneArrays::Renderable::getSingleton().getBufferOffsetRange());
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kMeshLods), GpuSceneArrays::MeshLod::getSingleton().getBufferOffsetRange());
	cmdb.bindUavBuffer(U32(MaterialSet::kGlobal), U32(MaterialBinding::kTransforms),
					   GpuSceneArrays::Transform::getSingleton().getBufferOffsetRange());
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

	cmdb.setVertexAttribute(0, 0, Format::kR32G32B32A32_Uint, 0);

	RenderStateBucketContainer::getSingleton().iterateBucketsPerformanceOrder(
		args.m_renderingTechinuqe,
		[&](const RenderStateInfo& state, U32 bucketIdx, U32 userCount, U32 meshletGroupCount, [[maybe_unused]] U32 meshletCount) {
			if(userCount == 0)
			{
				return;
			}

			cmdb.bindShaderProgram(state.m_program.get());

			const Bool meshlets = meshletGroupCount > 0;

			if(meshlets && meshShaderHwSupport)
			{
				const UVec4 firstPayload(args.m_mesh.m_bucketMeshletGroupInstanceRanges[bucketIdx].getFirstInstance());
				cmdb.setPushConstants(&firstPayload, sizeof(firstPayload));

				cmdb.drawMeshTasksIndirect(args.m_mesh.m_taskShaderIndirectArgsBuffer.m_buffer,
										   args.m_mesh.m_taskShaderIndirectArgsBuffer.m_offset + sizeof(DispatchIndirectArgs) * bucketIdx);
			}
			else if(meshlets)
			{
				cmdb.bindVertexBuffer(0, args.m_softwareMesh.m_meshletInstancesBuffer.m_buffer,
									  args.m_softwareMesh.m_meshletInstancesBuffer.m_offset
										  + args.m_softwareMesh.m_bucketMeshletInstanceRanges[bucketIdx].getFirstInstance()
												* sizeof(GpuSceneMeshletInstance),
									  sizeof(GpuSceneMeshletInstance), VertexStepRate::kInstance);

				cmdb.drawIndirect(PrimitiveTopology::kTriangles, 1,
								  args.m_softwareMesh.m_drawIndirectArgsBuffer.m_offset + sizeof(DrawIndirectArgs) * bucketIdx,
								  args.m_softwareMesh.m_drawIndirectArgsBuffer.m_buffer);
			}
			else
			{
				const U32 maxDrawCount = args.m_legacy.m_bucketRenderableInstanceRanges[bucketIdx].getInstanceCount();

				if(state.m_indexedDrawcall)
				{
					cmdb.bindVertexBuffer(0, args.m_legacy.m_renderableInstancesBuffer.m_buffer,
										  args.m_legacy.m_renderableInstancesBuffer.m_offset
											  + args.m_legacy.m_bucketRenderableInstanceRanges[bucketIdx].getFirstInstance()
													* sizeof(GpuSceneRenderableInstance),
										  sizeof(GpuSceneRenderableInstance), VertexStepRate::kInstance);

					cmdb.drawIndexedIndirectCount(state.m_primitiveTopology, args.m_legacy.m_drawIndexedIndirectArgsBuffer.m_buffer,
												  args.m_legacy.m_drawIndexedIndirectArgsBuffer.m_offset
													  + args.m_legacy.m_bucketRenderableInstanceRanges[bucketIdx].getFirstInstance()
															* sizeof(DrawIndexedIndirectArgs),
												  sizeof(DrawIndexedIndirectArgs), args.m_legacy.m_mdiDrawCountsBuffer.m_buffer,
												  args.m_legacy.m_mdiDrawCountsBuffer.m_offset + sizeof(U32) * bucketIdx, maxDrawCount);
				}
				else
				{
					cmdb.bindVertexBuffer(0, args.m_legacy.m_renderableInstancesBuffer.m_buffer,
										  args.m_legacy.m_renderableInstancesBuffer.m_offset
											  + args.m_legacy.m_bucketRenderableInstanceRanges[bucketIdx].getFirstInstance()
													* sizeof(GpuSceneRenderableInstance),
										  sizeof(GpuSceneRenderableInstance), VertexStepRate::kInstance);

					// Yes, the DrawIndexedIndirectArgs is intentional
					cmdb.drawIndirectCount(state.m_primitiveTopology, args.m_legacy.m_drawIndexedIndirectArgsBuffer.m_buffer,
										   args.m_legacy.m_drawIndexedIndirectArgsBuffer.m_offset
											   + args.m_legacy.m_bucketRenderableInstanceRanges[bucketIdx].getFirstInstance()
													 * sizeof(DrawIndexedIndirectArgs),
										   sizeof(DrawIndexedIndirectArgs), args.m_legacy.m_mdiDrawCountsBuffer.m_buffer,
										   args.m_legacy.m_mdiDrawCountsBuffer.m_offset + sizeof(U32) * bucketIdx, maxDrawCount);
				}
			}
		});

#if ANKI_STATS_ENABLED
	if(pplineQuery.isCreated())
	{
		cmdb.endPipelineQuery(pplineQuery.get());
	}
#endif
}

} // end namespace anki
