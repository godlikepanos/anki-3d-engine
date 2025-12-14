// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Resource/AccelerationStructureScratchAllocator.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Core/App.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

class MeshResource::LoadContext
{
public:
	MeshResourcePtr m_mesh;
	MeshBinaryLoader m_loader;

	LoadContext(MeshResource* mesh)
		: m_mesh(mesh)
		, m_loader(&ResourceMemoryPool::getSingleton())
	{
	}
};

/// Mesh upload async task.
class MeshResource::LoadTask : public AsyncLoaderTask
{
public:
	MeshResource::LoadContext m_ctx;

	LoadTask(MeshResource* mesh)
		: m_ctx(mesh)
	{
	}

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
	{
		return m_ctx.m_mesh->loadAsync(m_ctx.m_loader);
	}

	static BaseMemoryPool& getMemoryPool()
	{
		return ResourceMemoryPool::getSingleton();
	}
};

MeshResource::~MeshResource()
{
	for(Lod& lod : m_lods)
	{
		UnifiedGeometryBuffer::getSingleton().deferredFree(lod.m_indexBufferAllocationToken);

		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			UnifiedGeometryBuffer::getSingleton().deferredFree(lod.m_vertexBuffersAllocationToken[stream]);
		}
	}
}

Error MeshResource::load(const ResourceFilename& filename, Bool async)
{
	UniquePtr<LoadTask> task;
	LoadContext* ctx;
	LoadContext localCtx(this);

	String basename;
	getFilepathFilename(filename, basename);

	const Bool rayTracingEnabled = GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled;

	if(async)
	{
		task.reset(AsyncLoader::getSingleton().newTask<LoadTask>(this));
		ctx = &task->m_ctx;
	}
	else
	{
		task.reset(nullptr);
		ctx = &localCtx;
	}

	// Open file
	MeshBinaryLoader& loader = ctx->m_loader;
	ANKI_CHECK(loader.load(filename));
	const MeshBinaryHeader& header = loader.getHeader();

	// Misc
	m_indexType = header.m_indexType;
	m_aabb.setMin(header.m_boundingVolume.m_aabbMin);
	m_aabb.setMax(header.m_boundingVolume.m_aabbMax);
	m_positionsScale = header.m_vertexAttributes[VertexStreamId::kPosition].m_scale[0];
	m_positionsTranslation = Vec3(&header.m_vertexAttributes[VertexStreamId::kPosition].m_translation[0]);
	m_isConvex = !!(loader.getHeader().m_flags & MeshBinaryFlag::kConvex);

	// Submeshes
	m_subMeshes.resize(header.m_subMeshCount);
	for(U32 i = 0; i < m_subMeshes.getSize(); ++i)
	{
		for(U32 lod = 0; lod < header.m_lodCount; ++lod)
		{
			m_subMeshes[i].m_firstIndices[lod] = loader.getSubMeshes()[i].m_lods[lod].m_firstIndex;
			m_subMeshes[i].m_indexCounts[lod] = loader.getSubMeshes()[i].m_lods[lod].m_indexCount;
			m_subMeshes[i].m_firstMeshlet[lod] = loader.getSubMeshes()[i].m_lods[lod].m_firstMeshlet;
			m_subMeshes[i].m_meshletCounts[lod] = loader.getSubMeshes()[i].m_lods[lod].m_meshletCount;
		}

		m_subMeshes[i].m_aabb.setMin(loader.getSubMeshes()[i].m_boundingVolume.m_aabbMin);
		m_subMeshes[i].m_aabb.setMax(loader.getSubMeshes()[i].m_boundingVolume.m_aabbMax);
	}

	// LODs
	m_lods.resize(header.m_lodCount);
	for(I32 l = I32(header.m_lodCount - 1); l >= 0; --l)
	{
		Lod& lod = m_lods[l];

		// Index stuff
		lod.m_indexCount = header.m_indexCounts[l];
		ANKI_ASSERT((lod.m_indexCount % 3) == 0 && "Expecting triangles");
		const PtrSize indexBufferSize = PtrSize(lod.m_indexCount) * getIndexSize(m_indexType);
		lod.m_indexBufferAllocationToken = UnifiedGeometryBuffer::getSingleton().allocate(indexBufferSize, getIndexSize(m_indexType));

		// Vertex stuff
		lod.m_vertexCount = header.m_vertexCounts[l];
		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(header.m_vertexAttributes[stream].m_format == Format::kNone)
			{
				continue;
			}

			m_presentVertStreams |= VertexStreamMask(1 << stream);

			lod.m_vertexBuffersAllocationToken[stream] =
				UnifiedGeometryBuffer::getSingleton().allocateFormat(kMeshRelatedVertexStreamFormats[stream], lod.m_vertexCount);
		}

		// Meshlet
		if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_cvarCoreMeshletRendering)
		{
			const PtrSize meshletIndicesSize = header.m_meshletPrimitiveCounts[l] * sizeof(U8Vec4);
			lod.m_meshletIndices = UnifiedGeometryBuffer::getSingleton().allocate(meshletIndicesSize, sizeof(U8Vec4));

			const PtrSize meshletBoundingVolumesSize = header.m_meshletCounts[l] * sizeof(MeshletBoundingVolume);
			lod.m_meshletBoundingVolumes = UnifiedGeometryBuffer::getSingleton().allocate(meshletBoundingVolumesSize, sizeof(MeshletBoundingVolume));

			const PtrSize meshletGeomDescriptorsSize = header.m_meshletCounts[l] * sizeof(MeshletGeometryDescriptor);
			lod.m_meshletGeometryDescriptors =
				UnifiedGeometryBuffer::getSingleton().allocate(meshletGeomDescriptorsSize, sizeof(MeshletGeometryDescriptor));

			lod.m_meshletCount = header.m_meshletCounts[l];
		}
	}

	// BLAS
	if(rayTracingEnabled)
	{
		for(U32 submeshIdx = 0; submeshIdx < m_subMeshes.getSize(); ++submeshIdx)
		{
			SubMesh& subMesh = m_subMeshes[submeshIdx];

			for(U32 lodIdx = 0; lodIdx < header.m_lodCount; ++lodIdx)
			{
				Lod& lod = m_lods[lodIdx];

				AccelerationStructureInitInfo inf(ResourceString().sprintf("%s_%s", "BLAS", basename.cstr()));
				inf.m_type = AccelerationStructureType::kBottomLevel;

				inf.m_bottomLevel.m_indexBuffer = BufferView(lod.m_indexBufferAllocationToken)
													  .incrementOffset(getIndexSize(m_indexType) * subMesh.m_firstIndices[lodIdx])
													  .setRange(getIndexSize(m_indexType) * subMesh.m_indexCounts[lodIdx]);
				inf.m_bottomLevel.m_indexCount = subMesh.m_indexCounts[lodIdx];
				inf.m_bottomLevel.m_indexType = m_indexType;
				inf.m_bottomLevel.m_positionBuffer = lod.m_vertexBuffersAllocationToken[VertexStreamId::kPosition];
				inf.m_bottomLevel.m_positionStride = getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition]).m_texelSize;
				inf.m_bottomLevel.m_positionsFormat = kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition];
				inf.m_bottomLevel.m_positionCount = lod.m_vertexCount;

				const PtrSize requiredMemory = GrManager::getSingleton().getAccelerationStructureMemoryRequirement(inf);
				subMesh.m_blasAllocationTokens[lodIdx] = UnifiedGeometryBuffer::getSingleton().allocate(requiredMemory, 1);
				inf.m_accelerationStructureBuffer = subMesh.m_blasAllocationTokens[lodIdx];

				subMesh.m_blas[lodIdx] = GrManager::getSingleton().newAccelerationStructure(inf);
			}
		}
	}

	// Submit the loading task
	if(async)
	{
		LoadTask* pTask;
		task.moveAndReset(pTask);
		AsyncLoader::getSingleton().submitTask(pTask, AsyncLoaderPriority::kMedium);
	}
	else
	{
		ANKI_CHECK(loadAsync(loader));
	}

	return Error::kNone;
}

Error MeshResource::loadAsync(MeshBinaryLoader& loader) const
{
	GrManager& gr = GrManager::getSingleton();
	TransferGpuAllocator& transferAlloc = TransferGpuAllocator::getSingleton();

	Array<TransferGpuAllocatorHandle, kMaxLodCount*(U32(VertexStreamId::kMeshRelatedCount) + 1 + 3)> handles;
	U32 handleCount = 0;

	Buffer* unifiedGeometryBuffer = &UnifiedGeometryBuffer::getSingleton().getBuffer();
	const BufferUsageBit unifiedGeometryBufferNonTransferUsage = unifiedGeometryBuffer->getBufferUsage() ^ BufferUsageBit::kCopyDestination;

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = gr.newCommandBuffer(cmdbinit);

	// Set transfer to transfer barrier because of the clear that happened while sync loading
	const BufferBarrierInfo barrier = {UnifiedGeometryBuffer::getSingleton().getBufferView(), unifiedGeometryBufferNonTransferUsage,
									   BufferUsageBit::kCopyDestination};
	cmdb->setPipelineBarrier({}, {&barrier, 1}, {});

	// Upload index and vertex buffers
	for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
	{
		const Lod& lod = m_lods[lodIdx];

		// Upload index buffer
		{
			TransferGpuAllocatorHandle& handle = handles[handleCount++];

			ANKI_CHECK(transferAlloc.allocate(lod.m_indexBufferAllocationToken.getAllocatedSize(), handle));
			void* data = handle.getMappedMemory();
			ANKI_ASSERT(data);

			ANKI_CHECK(loader.storeIndexBuffer(lodIdx, data, handle.getRange()));

			cmdb->copyBufferToBuffer(handle, lod.m_indexBufferAllocationToken);
		}

		// Upload vert buffers
		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(!(m_presentVertStreams & VertexStreamMask(1 << stream)))
			{
				continue;
			}

			TransferGpuAllocatorHandle& handle = handles[handleCount++];

			ANKI_CHECK(transferAlloc.allocate(lod.m_vertexBuffersAllocationToken[stream].getAllocatedSize(), handle));
			U8* data = static_cast<U8*>(handle.getMappedMemory());
			ANKI_ASSERT(data);

			// Load to staging
			ANKI_CHECK(loader.storeVertexBuffer(lodIdx, U32(stream), data, handle.getRange()));

			// Copy
			cmdb->copyBufferToBuffer(handle, lod.m_vertexBuffersAllocationToken[stream]);
		}

		if(lod.m_meshletBoundingVolumes.isValid())
		{
			// Indices
			TransferGpuAllocatorHandle& handle = handles[handleCount++];
			const PtrSize primitivesSize = lod.m_meshletIndices.getAllocatedSize();
			ANKI_CHECK(transferAlloc.allocate(primitivesSize, handle));
			ANKI_CHECK(loader.storeMeshletIndicesBuffer(lodIdx, handle.getMappedMemory(), primitivesSize));

			cmdb->copyBufferToBuffer(handle, lod.m_meshletIndices);

			// Meshlets
			ResourceDynamicArray<MeshBinaryMeshlet> binaryMeshlets;
			binaryMeshlets.resize(loader.getHeader().m_meshletCounts[lodIdx]);
			ANKI_CHECK(loader.storeMeshletBuffer(lodIdx, WeakArray(binaryMeshlets)));

			TransferGpuAllocatorHandle& handle2 = handles[handleCount++];
			ANKI_CHECK(transferAlloc.allocate(lod.m_meshletBoundingVolumes.getAllocatedSize(), handle2));
			WeakArray<MeshletBoundingVolume> outMeshletBoundingVolumes(static_cast<MeshletBoundingVolume*>(handle2.getMappedMemory()),
																	   loader.getHeader().m_meshletCounts[lodIdx]);

			TransferGpuAllocatorHandle& handle3 = handles[handleCount++];
			ANKI_CHECK(transferAlloc.allocate(lod.m_meshletGeometryDescriptors.getAllocatedSize(), handle3));
			WeakArray<MeshletGeometryDescriptor> outMeshletGeomDescriptors(static_cast<MeshletGeometryDescriptor*>(handle3.getMappedMemory()),
																		   loader.getHeader().m_meshletCounts[lodIdx]);

			for(U32 i = 0; i < binaryMeshlets.getSize(); ++i)
			{
				const MeshBinaryMeshlet& inMeshlet = binaryMeshlets[i];
				MeshletGeometryDescriptor& outMeshletGeom = outMeshletGeomDescriptors[i];
				MeshletBoundingVolume& outMeshletBoundingVolume = outMeshletBoundingVolumes[i];

				outMeshletBoundingVolume = {};
				outMeshletGeom = {};
				for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
				{
					if(!(m_presentVertStreams & VertexStreamMask(1u << stream)))
					{
						continue;
					}

					outMeshletGeom.m_vertexOffsets[U32(stream)] =
						lod.m_vertexBuffersAllocationToken[stream].getOffset() / getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize
						+ inMeshlet.m_firstVertex;
				}

				outMeshletGeom.m_firstPrimitive =
					lod.m_meshletIndices.getOffset() / getFormatInfo(kMeshletPrimitiveFormat).m_texelSize + inMeshlet.m_firstPrimitive;
				outMeshletGeom.m_primitiveCount = inMeshlet.m_primitiveCount;
				outMeshletGeom.m_vertexCount = inMeshlet.m_vertexCount;
				outMeshletGeom.m_positionTranslation = m_positionsTranslation;
				outMeshletGeom.m_positionScale = m_positionsScale;

				outMeshletBoundingVolume.m_aabbMin = inMeshlet.m_boundingVolume.m_aabbMin;
				outMeshletBoundingVolume.m_aabbMax = inMeshlet.m_boundingVolume.m_aabbMax;
				outMeshletBoundingVolume.m_coneDirection_R8G8B8_Snorm_cosHalfAngle_R8_Snorm =
					Vec4(inMeshlet.m_coneDirection, cos(inMeshlet.m_coneAngle / 2.0f)).packSnorm4x8();
				outMeshletBoundingVolume.m_coneApex = inMeshlet.m_coneApex;
				outMeshletBoundingVolume.m_sphereRadius =
					((outMeshletBoundingVolume.m_aabbMin + outMeshletBoundingVolume.m_aabbMax) / 2.0f - outMeshletBoundingVolume.m_aabbMax).length();
				outMeshletBoundingVolume.m_primitiveCount = inMeshlet.m_primitiveCount;
			}

			cmdb->copyBufferToBuffer(handle2, lod.m_meshletBoundingVolumes);
			cmdb->copyBufferToBuffer(handle3, lod.m_meshletGeometryDescriptors);
		}
	}

	if(gr.getDeviceCapabilities().m_rayTracingEnabled)
	{
		// Build BLASes

		for(U32 submeshIdx = 0; submeshIdx < m_subMeshes.getSize(); ++submeshIdx)
		{
			const SubMesh& submesh = m_subMeshes[submeshIdx];

			// Set the barriers
			BufferBarrierInfo bufferBarrier;
			bufferBarrier.m_bufferView = UnifiedGeometryBuffer::getSingleton().getBufferView();
			bufferBarrier.m_previousUsage = BufferUsageBit::kCopyDestination;
			bufferBarrier.m_nextUsage = unifiedGeometryBufferNonTransferUsage;

			Array<AccelerationStructureBarrierInfo, kMaxLodCount> asBarriers;
			for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
			{
				asBarriers[lodIdx].m_as = submesh.m_blas[lodIdx].get();
				asBarriers[lodIdx].m_previousUsage = AccelerationStructureUsageBit::kNone;
				asBarriers[lodIdx].m_nextUsage = AccelerationStructureUsageBit::kBuild;
			}

			cmdb->setPipelineBarrier({}, {&bufferBarrier, 1}, {&asBarriers[0], m_lods.getSize()});

			// Build BLASes
			for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
			{
				Bool addBarrier;
				const BufferView scratchBuff =
					AccelerationStructureScratchAllocator::getSingleton().allocate(submesh.m_blas[lodIdx]->getBuildScratchBufferSize(), addBarrier);

				if(addBarrier)
				{
					BufferBarrierInfo barr;
					barr.m_bufferView = scratchBuff;
					barr.m_previousUsage = BufferUsageBit::kAccelerationStructureBuildScratch;
					barr.m_nextUsage = BufferUsageBit::kAccelerationStructureBuildScratch;
					cmdb->setPipelineBarrier({}, {&barr, 1}, {});
				}

				cmdb->buildAccelerationStructure(submesh.m_blas[lodIdx].get(), scratchBuff);
			}

			// Barriers again
			for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
			{
				asBarriers[lodIdx].m_as = submesh.m_blas[lodIdx].get();
				asBarriers[lodIdx].m_previousUsage = AccelerationStructureUsageBit::kBuild;
				asBarriers[lodIdx].m_nextUsage = AccelerationStructureUsageBit::kAllRead;
			}

			cmdb->setPipelineBarrier({}, {}, {&asBarriers[0], m_lods.getSize()});
		}
	}
	else
	{
		// Only set a barrier
		BufferBarrierInfo bufferBarrier;
		bufferBarrier.m_bufferView = UnifiedGeometryBuffer::getSingleton().getBufferView();
		bufferBarrier.m_previousUsage = BufferUsageBit::kCopyDestination;
		bufferBarrier.m_nextUsage = unifiedGeometryBufferNonTransferUsage;

		cmdb->setPipelineBarrier({}, {&bufferBarrier, 1}, {});
	}

	// Finalize
	FencePtr fence;
	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);

	for(U32 i = 0; i < handleCount; ++i)
	{
		transferAlloc.release(handles[i], fence);
	}

	m_loadedLodCount.store(m_lods.getSize());

	return Error::kNone;
}

Error MeshResource::getOrCreateCollisionShape(Bool wantStatic, U32 lod, PhysicsCollisionShapePtr& out) const
{
	lod = min<U32>(lod, getLodCount() - 1);

	Bool isConvex = m_isConvex;
	if(!isConvex && !wantStatic)
	{
		ANKI_RESOURCE_LOGW("Requesting a non-static non-convex collision shape is not supported. Will create a convex hull instead");
		isConvex = true;
	}

	const Lod& l = m_lods[lod];

	LockGuard lock(l.m_collisionShapeMtx);

	if(!l.m_collisionShapes[isConvex])
	{
		MeshBinaryLoader loader(&ResourceMemoryPool::getSingleton());
		ANKI_CHECK(loader.load(getFilename()));

		ResourceDynamicArray<Vec3> positions;
		ResourceDynamicArray<U32> indices;
		ANKI_CHECK(loader.storeIndicesAndPosition(lod, indices, positions));

		if(isConvex)
		{
			l.m_collisionShapes[isConvex] = PhysicsWorld::getSingleton().newConvexHullShape(positions);
		}
		else
		{
			l.m_collisionShapes[isConvex] = PhysicsWorld::getSingleton().newStaticMeshShape(positions, indices);
		}
	}

	out = l.m_collisionShapes[isConvex];
	return Error::kNone;
}

} // end namespace anki
