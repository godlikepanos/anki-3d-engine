// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

void RenderableDrawer::setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	// Allocate, set and bind global uniforms
	{
		MaterialGlobalConstants* globalConstants;
		const BufferView globalConstantsToken = RebarTransientMemoryPool::getSingleton().allocateConstantBuffer(globalConstants);

		globalConstants->m_viewProjectionMatrix = args.m_viewProjectionMatrix;
		globalConstants->m_previousViewProjectionMatrix = args.m_previousViewProjectionMatrix;
		static_assert(sizeof(globalConstants->m_viewTransform) == sizeof(args.m_viewMatrix));
		memcpy(&globalConstants->m_viewTransform, &args.m_viewMatrix, sizeof(args.m_viewMatrix));
		static_assert(sizeof(globalConstants->m_cameraTransform) == sizeof(args.m_cameraTransform));
		memcpy(&globalConstants->m_cameraTransform, &args.m_cameraTransform, sizeof(args.m_cameraTransform));

		ANKI_ASSERT(args.m_viewport != UVec4(0u));
		globalConstants->m_viewport = Vec4(args.m_viewport);

		cmdb.bindConstantBuffer(ANKI_MATERIAL_REGISTER_GLOBAL_CONSTANTS, 0, globalConstantsToken);
	}

	// More globals
	cmdb.bindSampler(ANKI_MATERIAL_REGISTER_TILINEAR_REPEAT_SAMPLER, 0, args.m_sampler);
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_GPU_SCENE, 0, GpuSceneBuffer::getSingleton().getBufferView());

	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_UNIFIED_GEOMETRY, 0, UnifiedGeometryBuffer::getSingleton().getBufferView());
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType, reg) \
	cmdb.bindSrv( \
		reg, 0, \
		BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
				   getAlignedRoundDown(getFormatInfo(Format::k##fmt).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())), \
		Format::k##fmt);
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.def.h>

	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESHLET_BOUNDING_VOLUMES, 0, UnifiedGeometryBuffer::getSingleton().getBufferView());
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESHLET_GEOMETRY_DESCRIPTORS, 0, UnifiedGeometryBuffer::getSingleton().getBufferView());
	if(args.m_mesh.m_meshletInstancesBuffer)
	{
		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESHLET_INSTANCES, 0, args.m_mesh.m_meshletInstancesBuffer);
	}
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_RENDERABLES, 0, GpuSceneArrays::Renderable::getSingleton().getBufferView());
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_MESH_LODS, 0, GpuSceneArrays::MeshLod::getSingleton().getBufferView());
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_TRANSFORMS, 0, GpuSceneArrays::Transform::getSingleton().getBufferView());
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_PARTICLE_EMITTERS, 0, GpuSceneArrays::ParticleEmitter::getSingleton().getBufferViewSafe());
	cmdb.bindSrv(ANKI_MATERIAL_REGISTER_PARTICLE_EMITTERS2, 0, GpuSceneArrays::ParticleEmitter2::getSingleton().getBufferViewSafe());
	cmdb.bindSampler(ANKI_MATERIAL_REGISTER_NEAREST_CLAMP_SAMPLER, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());
	if(args.m_legacy.m_perDrawBuffer)
	{
		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_PER_DRAW, 0, args.m_legacy.m_perDrawBuffer);
	}

	if(args.m_mesh.m_firstMeshletBuffer)
	{
		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_FIRST_MESHLET, 0, args.m_mesh.m_firstMeshletBuffer);
	}

	if(args.m_legacy.m_firstPerDrawBuffer)
	{
		cmdb.bindSrv(ANKI_MATERIAL_REGISTER_PER_DRAW_OFFSET, 0, args.m_legacy.m_firstPerDrawBuffer);
	}

	// Misc
	cmdb.bindIndexBuffer(UnifiedGeometryBuffer::getSingleton().getBufferView(), IndexType::kU16);
}

void RenderableDrawer::drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb)
{
	ANKI_TRACE_SCOPED_EVENT(DrawMdi);
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

	cmdb.setVertexAttribute(VertexAttributeSemantic::kMisc0, 0, Format::kR32G32B32A32_Uint, 0);

	RenderStateBucketContainer::getSingleton().iterateBucketsPerformanceOrder(args.m_renderingTechinuqe, [&](const RenderStateInfo& state,
																											 U32 bucketIdx, U32 userCount,
																											 U32 meshletCount) {
		if(userCount == 0)
		{
			return;
		}

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
	});

#if ANKI_STATS_ENABLED
	if(pplineQuery.isCreated())
	{
		cmdb.endPipelineQuery(pplineQuery.get());
	}
#endif
}

} // end namespace anki
