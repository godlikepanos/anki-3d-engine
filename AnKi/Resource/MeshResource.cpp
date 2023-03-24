// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

class MeshResource::LoadContext
{
public:
	MeshResourcePtr m_mesh;
	MeshBinaryLoader m_loader;

	LoadContext(MeshResource* mesh, BaseMemoryPool* pool)
		: m_mesh(mesh)
		, m_loader(mesh->getExternalSubsystems().m_resourceFilesystem, pool)
	{
	}
};

/// Mesh upload async task.
class MeshResource::LoadTask : public AsyncLoaderTask
{
public:
	MeshResource::LoadContext m_ctx;

	LoadTask(MeshResource* mesh)
		: m_ctx(mesh, &mesh->getManager().getAsyncLoader().getMemoryPool())
	{
	}

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
	{
		return m_ctx.m_mesh->loadAsync(m_ctx.m_loader);
	}

	BaseMemoryPool& getMemoryPool() const
	{
		return m_ctx.m_mesh->getManager().getAsyncLoader().getMemoryPool();
	}
};

MeshResource::MeshResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

MeshResource::~MeshResource()
{
	m_subMeshes.destroy(getMemoryPool());

	for(Lod& lod : m_lods)
	{
		UnifiedGeometryMemoryPool::getSingleton().deferredFree(lod.m_indexBufferAllocationToken);

		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			UnifiedGeometryMemoryPool::getSingleton().deferredFree(lod.m_vertexBuffersAllocationToken[stream]);
		}
	}

	m_lods.destroy(getMemoryPool());
}

Error MeshResource::load(const ResourceFilename& filename, Bool async)
{
	UniquePtr<LoadTask> task;
	LoadContext* ctx;
	LoadContext localCtx(this, &getTempMemoryPool());

	StringRaii basename(&getTempMemoryPool());
	getFilepathFilename(filename, basename);

	const Bool rayTracingEnabled = getExternalSubsystems().m_grManager->getDeviceCapabilities().m_rayTracingEnabled;

	if(async)
	{
		task.reset(getManager().getAsyncLoader().newTask<LoadTask>(this));
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
	m_aabb.setMin(header.m_aabbMin);
	m_aabb.setMax(header.m_aabbMax);
	m_positionsScale = header.m_vertexAttributes[VertexStreamId::kPosition].m_scale[0];
	m_positionsTranslation = Vec3(&header.m_vertexAttributes[VertexStreamId::kPosition].m_translation[0]);

	// Submeshes
	m_subMeshes.create(getMemoryPool(), header.m_subMeshCount);
	for(U32 i = 0; i < m_subMeshes.getSize(); ++i)
	{
		m_subMeshes[i].m_firstIndices = loader.getSubMeshes()[i].m_firstIndices;
		m_subMeshes[i].m_indexCounts = loader.getSubMeshes()[i].m_indexCounts;
		m_subMeshes[i].m_aabb.setMin(loader.getSubMeshes()[i].m_aabbMin);
		m_subMeshes[i].m_aabb.setMax(loader.getSubMeshes()[i].m_aabbMax);
	}

	// LODs
	m_lods.create(getMemoryPool(), header.m_lodCount);
	for(I32 l = I32(header.m_lodCount - 1); l >= 0; --l)
	{
		Lod& lod = m_lods[l];

		// Index stuff
		lod.m_indexCount = header.m_totalIndexCounts[l];
		ANKI_ASSERT((lod.m_indexCount % 3) == 0 && "Expecting triangles");
		const PtrSize indexBufferSize = PtrSize(lod.m_indexCount) * getIndexSize(m_indexType);
		UnifiedGeometryMemoryPool::getSingleton().allocate(indexBufferSize, getIndexSize(m_indexType),
														   lod.m_indexBufferAllocationToken);

		// Vertex stuff
		lod.m_vertexCount = header.m_totalVertexCounts[l];
		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(header.m_vertexAttributes[stream].m_format == Format::kNone)
			{
				continue;
			}

			m_presentVertStreams |= VertexStreamMask(1 << stream);

			const U32 texelSize = getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;
			const U32 alignment = max(4u, nextPowerOfTwo(texelSize));
			const PtrSize vertexBufferSize = PtrSize(lod.m_vertexCount) * texelSize + alignment;

			UnifiedGeometryMemoryPool::getSingleton().allocate(vertexBufferSize, alignment,
															   lod.m_vertexBuffersAllocationToken[stream]);

			// We need to align the actual offset to the texel size
			const PtrSize remainder = lod.m_vertexBuffersAllocationToken[stream].m_offset % texelSize;
			lod.m_fixedUnifiedGeometryBufferOffset[stream] = U8(texelSize - remainder);

			ANKI_ASSERT(
				(lod.m_vertexBuffersAllocationToken[stream].m_offset + lod.m_fixedUnifiedGeometryBufferOffset[stream])
					% texelSize
				== 0);
			ANKI_ASSERT(lod.m_fixedUnifiedGeometryBufferOffset[stream] + PtrSize(lod.m_vertexCount) * texelSize
						<= lod.m_vertexBuffersAllocationToken[stream].m_size);
		}

		// BLAS
		if(rayTracingEnabled)
		{
			AccelerationStructureInitInfo inf(
				StringRaii(&getTempMemoryPool()).sprintf("%s_%s", "Blas", basename.cstr()));
			inf.m_type = AccelerationStructureType::kBottomLevel;

			inf.m_bottomLevel.m_indexBuffer = UnifiedGeometryMemoryPool::getSingleton().getBuffer();
			inf.m_bottomLevel.m_indexBufferOffset = lod.m_indexBufferAllocationToken.m_offset;
			inf.m_bottomLevel.m_indexCount = lod.m_indexCount;
			inf.m_bottomLevel.m_indexType = m_indexType;
			inf.m_bottomLevel.m_positionBuffer = UnifiedGeometryMemoryPool::getSingleton().getBuffer();
			inf.m_bottomLevel.m_positionBufferOffset =
				lod.m_vertexBuffersAllocationToken[VertexStreamId::kPosition].m_offset
				+ lod.m_fixedUnifiedGeometryBufferOffset[VertexStreamId::kPosition];
			inf.m_bottomLevel.m_positionStride =
				getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition]).m_texelSize;
			inf.m_bottomLevel.m_positionsFormat = kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition];
			inf.m_bottomLevel.m_positionCount = lod.m_vertexCount;

			lod.m_blas = getExternalSubsystems().m_grManager->newAccelerationStructure(inf);
		}
	}

	// Clear the buffers
	if(async)
	{
		CommandBufferInitInfo cmdbinit("MeshResourceClear");
		cmdbinit.m_flags = CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork;
		CommandBufferPtr cmdb = getExternalSubsystems().m_grManager->newCommandBuffer(cmdbinit);

		for(const Lod& lod : m_lods)
		{
			cmdb->fillBuffer(UnifiedGeometryMemoryPool::getSingleton().getBuffer(),
							 lod.m_indexBufferAllocationToken.m_offset,
							 PtrSize(lod.m_indexCount) * getIndexSize(m_indexType), 0);

			for(VertexStreamId stream :
				EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
			{
				if(header.m_vertexAttributes[stream].m_format != Format::kNone)
				{
					cmdb->fillBuffer(UnifiedGeometryMemoryPool::getSingleton().getBuffer(),
									 lod.m_vertexBuffersAllocationToken[stream].m_offset,
									 lod.m_vertexBuffersAllocationToken[stream].m_size, 0);
				}
			}
		}

		const BufferBarrierInfo barrier = {UnifiedGeometryMemoryPool::getSingleton().getBuffer().get(),
										   BufferUsageBit::kTransferDestination, BufferUsageBit::kVertex, 0,
										   kMaxPtrSize};

		cmdb->setPipelineBarrier({}, {&barrier, 1}, {});

		cmdb->flush();
	}

	// Submit the loading task
	if(async)
	{
		getManager().getAsyncLoader().submitTask(task.get());
		LoadTask* pTask;
		task.moveAndReset(pTask);
	}
	else
	{
		ANKI_CHECK(loadAsync(loader));
	}

	return Error::kNone;
}

Error MeshResource::loadAsync(MeshBinaryLoader& loader) const
{
	GrManager& gr = *getExternalSubsystems().m_grManager;
	TransferGpuAllocator& transferAlloc = getManager().getTransferGpuAllocator();

	Array<TransferGpuAllocatorHandle, kMaxLodCount*(U32(VertexStreamId::kMeshRelatedCount) + 1)> handles;
	U32 handleCount = 0;

	BufferPtr unifiedGeometryBuffer = UnifiedGeometryMemoryPool::getSingleton().getBuffer();
	const BufferUsageBit unifiedGeometryBufferNonTransferUsage =
		unifiedGeometryBuffer->getBufferUsage() ^ BufferUsageBit::kTransferDestination;

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::kSmallBatch | CommandBufferFlag::kGeneralWork;
	CommandBufferPtr cmdb = gr.newCommandBuffer(cmdbinit);

	// Set transfer to transfer barrier because of the clear that happened while sync loading
	const BufferBarrierInfo barrier = {unifiedGeometryBuffer.get(), unifiedGeometryBufferNonTransferUsage,
									   BufferUsageBit::kTransferDestination, 0, kMaxPtrSize};
	cmdb->setPipelineBarrier({}, {&barrier, 1}, {});

	// Upload index and vertex buffers
	for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
	{
		const Lod& lod = m_lods[lodIdx];

		// Upload index buffer
		{
			TransferGpuAllocatorHandle& handle = handles[handleCount++];
			const PtrSize indexBufferSize = PtrSize(lod.m_indexCount) * getIndexSize(m_indexType);

			ANKI_CHECK(transferAlloc.allocate(indexBufferSize, handle));
			void* data = handle.getMappedMemory();
			ANKI_ASSERT(data);

			ANKI_CHECK(loader.storeIndexBuffer(lodIdx, data, indexBufferSize));

			cmdb->copyBufferToBuffer(handle.getBuffer(), handle.getOffset(), unifiedGeometryBuffer,
									 lod.m_indexBufferAllocationToken.m_offset, handle.getRange());
		}

		// Upload vert buffers
		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(!(m_presentVertStreams & VertexStreamMask(1 << stream)))
			{
				continue;
			}

			TransferGpuAllocatorHandle& handle = handles[handleCount++];
			const PtrSize vertexBufferSize =
				PtrSize(lod.m_vertexCount) * getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;

			ANKI_CHECK(transferAlloc.allocate(vertexBufferSize, handle));
			U8* data = static_cast<U8*>(handle.getMappedMemory());
			ANKI_ASSERT(data);

			// Load to staging
			ANKI_CHECK(loader.storeVertexBuffer(lodIdx, U32(stream), data, vertexBufferSize));

			// Copy
			cmdb->copyBufferToBuffer(handle.getBuffer(), handle.getOffset(), unifiedGeometryBuffer,
									 lod.m_vertexBuffersAllocationToken[stream].m_offset
										 + lod.m_fixedUnifiedGeometryBufferOffset[stream],
									 handle.getRange());
		}
	}

	if(gr.getDeviceCapabilities().m_rayTracingEnabled)
	{
		// Build BLASes

		// Set the barriers
		BufferBarrierInfo bufferBarrier;
		bufferBarrier.m_buffer = unifiedGeometryBuffer.get();
		bufferBarrier.m_offset = 0;
		bufferBarrier.m_size = kMaxPtrSize;
		bufferBarrier.m_previousUsage = BufferUsageBit::kTransferDestination;
		bufferBarrier.m_nextUsage = unifiedGeometryBufferNonTransferUsage;

		Array<AccelerationStructureBarrierInfo, kMaxLodCount> asBarriers;
		for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
		{
			asBarriers[lodIdx].m_as = m_lods[lodIdx].m_blas.get();
			asBarriers[lodIdx].m_previousUsage = AccelerationStructureUsageBit::kNone;
			asBarriers[lodIdx].m_nextUsage = AccelerationStructureUsageBit::kBuild;
		}

		cmdb->setPipelineBarrier({}, {&bufferBarrier, 1}, {&asBarriers[0], m_lods.getSize()});

		// Build BLASes
		for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
		{
			cmdb->buildAccelerationStructure(m_lods[lodIdx].m_blas);
		}

		// Barriers again
		for(U32 lodIdx = 0; lodIdx < m_lods.getSize(); ++lodIdx)
		{
			asBarriers[lodIdx].m_as = m_lods[lodIdx].m_blas.get();
			asBarriers[lodIdx].m_previousUsage = AccelerationStructureUsageBit::kBuild;
			asBarriers[lodIdx].m_nextUsage = AccelerationStructureUsageBit::kAllRead;
		}

		cmdb->setPipelineBarrier({}, {}, {&asBarriers[0], m_lods.getSize()});
	}
	else
	{
		// Only set a barrier
		BufferBarrierInfo bufferBarrier;
		bufferBarrier.m_buffer = unifiedGeometryBuffer.get();
		bufferBarrier.m_offset = 0;
		bufferBarrier.m_size = kMaxPtrSize;
		bufferBarrier.m_previousUsage = BufferUsageBit::kTransferDestination;
		bufferBarrier.m_nextUsage = unifiedGeometryBufferNonTransferUsage;

		cmdb->setPipelineBarrier({}, {&bufferBarrier, 1}, {});
	}

	// Finalize
	FencePtr fence;
	cmdb->flush({}, &fence);

	for(U32 i = 0; i < handleCount; ++i)
	{
		transferAlloc.release(handles[i], fence);
	}

	return Error::kNone;
}

} // end namespace anki
