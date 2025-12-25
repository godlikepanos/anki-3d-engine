// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/Drawer.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/DepthDownscale.h>
#include <AnKi/Renderer/VolumetricLightingAccumulation.h>
#include <AnKi/Renderer/ClusterBinning.h>
#include <AnKi/Renderer/ShadowMapping.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>
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

void RenderableDrawer::setState(const RenderableDrawerArguments& args, RenderPassWorkContext& rgraphCtx)
{
	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

	// Allocate, set and bind global uniforms
	BufferView globalConstantsToken;
	{
		MaterialGlobalConstants* globalConstants;
		globalConstantsToken = RebarTransientMemoryPool::getSingleton().allocateConstantBuffer(globalConstants);

		globalConstants->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalConstants->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalConstants->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalConstants->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalConstants->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalConstants->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		ANKI_ASSERT(args.m_viewport != UVec4(0u));
		globalConstants->m_viewport = Vec4(args.m_viewport);
	}

	// Space 0 globals
	const Bool bForwardShading = args.m_renderingTechinuqe == RenderingTechnique::kForward;
#define ANKI_RASTER_PATH 1
#include <AnKi/Shaders/Include/MaterialBindings.def.h>

	// Misc
	cmdb.bindIndexBuffer(UnifiedGeometryBuffer::getSingleton().getBufferView(), IndexType::kU16);
}

void RenderableDrawer::drawMdi(const RenderableDrawerArguments& args, RenderPassWorkContext& rgraphCtx)
{
	ANKI_TRACE_SCOPED_EVENT(DrawMdi);
	ANKI_ASSERT(args.m_viewport != UVec4(0u));

	if(RenderStateBucketContainer::getSingleton().getBucketCount(args.m_renderingTechinuqe) == 0) [[unlikely]]
	{
		return;
	}

	CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

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

	setState(args, rgraphCtx);

	const Bool meshShaderHwSupport = GrManager::getSingleton().getDeviceCapabilities().m_meshShaders;

	cmdb.setVertexAttribute(VertexAttributeSemantic::kMisc0, 0, Format::kR32G32B32A32_Uint, 0);

	RenderStateBucketContainer::getSingleton().iterateBucketsPerformanceOrder(args.m_renderingTechinuqe, [&](const RenderStateInfo& state,
																											 U32 bucketIdx, U32 userCount,
																											 U32 meshletCount) {
		if(userCount == 0)
		{
			return;
		}

		cmdb.pushDebugMarker(state.m_program->getName(), Vec3(0.0f, 1.0f, 0.0f));

		cmdb.bindShaderProgram(state.m_program.get());

		const Bool bMeshlets = meshletCount > 0;

		if(bMeshlets && meshShaderHwSupport)
		{
			const UVec4 consts(bucketIdx);
			cmdb.setFastConstants(&consts, sizeof(consts));

			cmdb.drawMeshTasksIndirect(BufferView(args.m_mesh.m_dispatchMeshIndirectArgsBuffer)
										   .incrementOffset(sizeof(DispatchIndirectArgs) * bucketIdx)
										   .setRange(sizeof(DispatchIndirectArgs)));
		}
		else if(bMeshlets)
		{
			cmdb.bindVertexBuffer(0, args.m_mesh.m_meshletInstancesBuffer, sizeof(GpuSceneMeshletInstance), VertexStepRate::kInstance);

			const BufferView indirectArgsBuffView =
				BufferView(args.m_mesh.m_indirectDrawArgs).incrementOffset(sizeof(DrawIndirectArgs) * bucketIdx).setRange(sizeof(DrawIndirectArgs));
			cmdb.drawIndirect(PrimitiveTopology::kTriangles, indirectArgsBuffView);
		}
		else
		{
			// Legacy indexed

			const InstanceRange& instanceRange = args.m_legacy.m_bucketRenderableInstanceRanges[bucketIdx];

			const UVec4 consts(bucketIdx);
			cmdb.setFastConstants(&consts, sizeof(consts));

			const U32 maxDrawCount = instanceRange.getInstanceCount();

			const BufferView indirectArgsBuffView = BufferView(args.m_legacy.m_drawIndexedIndirectArgsBuffer)
														.incrementOffset(instanceRange.getFirstInstance() * sizeof(DrawIndexedIndirectArgs))
														.setRange(instanceRange.getInstanceCount() * sizeof(DrawIndexedIndirectArgs));
			const BufferView mdiCountBuffView =
				BufferView(args.m_legacy.m_mdiDrawCountsBuffer).incrementOffset(sizeof(U32) * bucketIdx).setRange(sizeof(U32));
			cmdb.drawIndexedIndirectCount(state.m_primitiveTopology, indirectArgsBuffView, sizeof(DrawIndexedIndirectArgs), mdiCountBuffView,
										  maxDrawCount);
		}

		cmdb.popDebugMarker();
	});

#if ANKI_STATS_ENABLED
	if(pplineQuery.isCreated())
	{
		cmdb.endPipelineQuery(pplineQuery.get());
	}
#endif
}

} // end namespace anki
