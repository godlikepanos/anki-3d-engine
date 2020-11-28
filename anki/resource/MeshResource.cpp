// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MeshResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MeshLoader.h>
#include <anki/resource/AsyncLoader.h>
#include <anki/util/Functions.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Xml.h>

namespace anki
{

class MeshResource::LoadContext
{
public:
	MeshResource* m_mesh;
	MeshLoader m_loader;

	LoadContext(MeshResource* mesh, GenericMemoryPoolAllocator<U8> alloc)
		: m_mesh(mesh)
		, m_loader(&mesh->getManager(), alloc)
	{
	}
};

/// Mesh upload async task.
class MeshResource::LoadTask : public AsyncLoaderTask
{
public:
	MeshResource::LoadContext m_ctx;

	LoadTask(MeshResource* mesh)
		: m_ctx(mesh, mesh->getManager().getAsyncLoader().getAllocator())
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx) final
	{
		return m_ctx.m_mesh->loadAsync(m_ctx.m_loader);
	}
};

MeshResource::MeshResource(ResourceManager* manager)
	: ResourceObject(manager)
{
	memset(&m_meshGpuDescriptor, 0, sizeof(m_meshGpuDescriptor));
}

MeshResource::~MeshResource()
{
	m_subMeshes.destroy(getAllocator());
	m_vertBufferInfos.destroy(getAllocator());
}

Bool MeshResource::isCompatible(const MeshResource& other) const
{
	return hasBoneWeights() == other.hasBoneWeights() && getSubMeshCount() == other.getSubMeshCount()
		   && m_texChannelCount == other.m_texChannelCount;
}

Error MeshResource::load(const ResourceFilename& filename, Bool async)
{
	LoadTask* task;
	LoadContext* ctx;
	LoadContext localCtx(this, getTempAllocator());

	StringAuto basename(getTempAllocator());
	getFilepathFilename(filename, basename);

	const Bool rayTracingEnabled = getManager().getGrManager().getDeviceCapabilities().m_rayTracingEnabled;
	if(rayTracingEnabled)
	{
		async = false; // TODO Find a better way
	}

	if(async)
	{
		task = getManager().getAsyncLoader().newTask<LoadTask>(this);
		ctx = &task->m_ctx;
	}
	else
	{
		task = nullptr;
		ctx = &localCtx;
	}

	// Open file
	MeshLoader& loader = ctx->m_loader;
	ANKI_CHECK(loader.load(filename));
	const MeshBinaryFile::Header& header = loader.getHeader();

	// Get submeshes
	m_subMeshes.create(getAllocator(), header.m_subMeshCount);
	for(U32 i = 0; i < m_subMeshes.getSize(); ++i)
	{
		m_subMeshes[i].m_firstIndex = loader.getSubMeshes()[i].m_firstIndex;
		m_subMeshes[i].m_indexCount = loader.getSubMeshes()[i].m_indexCount;

		const Vec3 obbCenter = (loader.getSubMeshes()[i].m_aabbMax + loader.getSubMeshes()[i].m_aabbMin) / 2.0f;
		const Vec3 obbExtend = loader.getSubMeshes()[i].m_aabbMax - obbCenter;
		m_subMeshes[i].m_obb = Obb(obbCenter.xyz0(), Mat3x4::getIdentity(), obbExtend.xyz0());
	}

	// Index stuff
	m_indexCount = header.m_totalIndexCount;
	ANKI_ASSERT((m_indexCount % 3) == 0 && "Expecting triangles");
	m_indexType = header.m_indexType;

	const PtrSize indexBuffSize = PtrSize(m_indexCount) * ((m_indexType == IndexType::U32) ? 4 : 2);

	BufferUsageBit indexBufferUsage = BufferUsageBit::INDEX | BufferUsageBit::TRANSFER_DESTINATION;
	if(rayTracingEnabled)
	{
		indexBufferUsage |= BufferUsageBit::ACCELERATION_STRUCTURE_BUILD;
	}
	m_indexBuff = getManager().getGrManager().newBuffer(
		BufferInitInfo(indexBuffSize, indexBufferUsage, BufferMapAccessBit::NONE,
					   StringAuto(getTempAllocator()).sprintf("%s_%s", "Idx", basename.cstr())));

	// Vertex stuff
	m_vertCount = header.m_totalVertexCount;
	m_vertBufferInfos.create(getAllocator(), header.m_vertexBufferCount);

	U32 totalVertexBuffSize = 0;
	for(U32 i = 0; i < header.m_vertexBufferCount; ++i)
	{
		alignRoundUp(VERTEX_BUFFER_ALIGNMENT, totalVertexBuffSize);

		m_vertBufferInfos[i].m_offset = totalVertexBuffSize;
		m_vertBufferInfos[i].m_stride = header.m_vertexBuffers[i].m_vertexStride;

		totalVertexBuffSize += m_vertCount * m_vertBufferInfos[i].m_stride;
	}

	BufferUsageBit vertexBufferUsage = BufferUsageBit::VERTEX | BufferUsageBit::TRANSFER_DESTINATION;
	if(rayTracingEnabled)
	{
		vertexBufferUsage |= BufferUsageBit::ACCELERATION_STRUCTURE_BUILD;
	}
	m_vertBuff = getManager().getGrManager().newBuffer(
		BufferInitInfo(totalVertexBuffSize, vertexBufferUsage, BufferMapAccessBit::NONE,
					   StringAuto(getTempAllocator()).sprintf("%s_%s", "Vert", basename.cstr())));

	m_texChannelCount = !!header.m_vertexAttributes[VertexAttributeLocation::UV2].m_format ? 2 : 1;

	for(VertexAttributeLocation attrib = VertexAttributeLocation::FIRST; attrib < VertexAttributeLocation::COUNT;
		++attrib)
	{
		AttribInfo& out = m_attribs[attrib];
		const MeshBinaryFile::VertexAttribute& in = header.m_vertexAttributes[attrib];

		if(!!in.m_format)
		{
			out.m_fmt = in.m_format;
			out.m_relativeOffset = in.m_relativeOffset;
			out.m_buffIdx = U8(in.m_bufferBinding);
			ANKI_ASSERT(in.m_scale == 1.0f && "Not supported ATM");
		}
	}

	// Other
	const Vec3 obbCenter = (header.m_aabbMax + header.m_aabbMin) / 2.0f;
	const Vec3 obbExtend = header.m_aabbMax - obbCenter;
	m_obb = Obb(obbCenter.xyz0(), Mat3x4::getIdentity(), obbExtend.xyz0());

	// Clear the buffers
	if(!async)
	{
		CommandBufferInitInfo cmdbinit;
		cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH;
		CommandBufferPtr cmdb = getManager().getGrManager().newCommandBuffer(cmdbinit);

		cmdb->fillBuffer(m_vertBuff, 0, MAX_PTR_SIZE, 0);
		cmdb->fillBuffer(m_indexBuff, 0, MAX_PTR_SIZE, 0);

		cmdb->setBufferBarrier(m_vertBuff, BufferUsageBit::TRANSFER_DESTINATION, BufferUsageBit::VERTEX, 0,
							   MAX_PTR_SIZE);
		cmdb->setBufferBarrier(m_indexBuff, BufferUsageBit::TRANSFER_DESTINATION, BufferUsageBit::INDEX, 0,
							   MAX_PTR_SIZE);

		cmdb->flush();
	}

	// Create the BLAS
	if(rayTracingEnabled)
	{
		AccelerationStructureInitInfo inf(StringAuto(getTempAllocator()).sprintf("%s_%s", "Blas", basename.cstr()));
		inf.m_type = AccelerationStructureType::BOTTOM_LEVEL;

		inf.m_bottomLevel.m_indexBuffer = m_indexBuff;
		inf.m_bottomLevel.m_indexBufferOffset = 0;
		inf.m_bottomLevel.m_indexCount = m_indexCount;
		inf.m_bottomLevel.m_indexType = m_indexType;

		U32 bufferIdx;
		Format format;
		PtrSize relativeOffset;
		getVertexAttributeInfo(VertexAttributeLocation::POSITION, bufferIdx, format, relativeOffset);

		BufferPtr buffer;
		PtrSize offset;
		PtrSize stride;
		getVertexBufferInfo(bufferIdx, buffer, offset, stride);

		inf.m_bottomLevel.m_positionBuffer = buffer;
		inf.m_bottomLevel.m_positionBufferOffset = offset;
		inf.m_bottomLevel.m_positionStride = U32(stride);
		inf.m_bottomLevel.m_positionsFormat = format;
		inf.m_bottomLevel.m_positionCount = m_vertCount;

		m_blas = getManager().getGrManager().newAccelerationStructure(inf);
	}

	// Fill the GPU descriptor
	if(rayTracingEnabled)
	{
		U32 bufferIdx;
		Format format;
		PtrSize relativeOffset;
		getVertexAttributeInfo(VertexAttributeLocation::POSITION, bufferIdx, format, relativeOffset);
		BufferPtr buffer;
		PtrSize offset;
		PtrSize stride;
		getVertexBufferInfo(bufferIdx, buffer, offset, stride);
		m_meshGpuDescriptor.m_indexBufferPtr = m_indexBuff->getGpuAddress();
		m_meshGpuDescriptor.m_positionBufferPtr = buffer->getGpuAddress();

		getVertexAttributeInfo(VertexAttributeLocation::NORMAL, bufferIdx, format, relativeOffset);
		getVertexBufferInfo(bufferIdx, buffer, offset, stride);
		m_meshGpuDescriptor.m_mainVertexBufferPtr = buffer->getGpuAddress();

		if(hasBoneWeights())
		{
			getVertexAttributeInfo(VertexAttributeLocation::BONE_WEIGHTS, bufferIdx, format, relativeOffset);
			getVertexBufferInfo(bufferIdx, buffer, offset, stride);
			m_meshGpuDescriptor.m_boneInfoVertexBufferPtr = buffer->getGpuAddress();
		}

		m_meshGpuDescriptor.m_indexCount = m_indexCount;
		m_meshGpuDescriptor.m_vertexCount = m_vertCount;
		m_meshGpuDescriptor.m_aabbMin = header.m_aabbMin;
		m_meshGpuDescriptor.m_aabbMax = header.m_aabbMax;
	}

	// Submit the loading task
	if(async)
	{
		getManager().getAsyncLoader().submitTask(task);
	}
	else
	{
		ANKI_CHECK(loadAsync(loader));
	}

	return Error::NONE;
}

Error MeshResource::loadAsync(MeshLoader& loader) const
{
	GrManager& gr = getManager().getGrManager();
	TransferGpuAllocator& transferAlloc = getManager().getTransferGpuAllocator();
	Array<TransferGpuAllocatorHandle, 2> handles;

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = gr.newCommandBuffer(cmdbinit);

	// Set barriers
	cmdb->setBufferBarrier(m_vertBuff, BufferUsageBit::VERTEX, BufferUsageBit::TRANSFER_DESTINATION, 0, MAX_PTR_SIZE);
	cmdb->setBufferBarrier(m_indexBuff, BufferUsageBit::INDEX, BufferUsageBit::TRANSFER_DESTINATION, 0, MAX_PTR_SIZE);

	// Write index buffer
	{
		ANKI_CHECK(transferAlloc.allocate(m_indexBuff->getSize(), handles[1]));
		void* data = handles[1].getMappedMemory();
		ANKI_ASSERT(data);

		ANKI_CHECK(loader.storeIndexBuffer(data, m_indexBuff->getSize()));

		cmdb->copyBufferToBuffer(handles[1].getBuffer(), handles[1].getOffset(), m_indexBuff, 0, handles[1].getRange());
	}

	// Write vert buff
	{
		ANKI_CHECK(transferAlloc.allocate(m_vertBuff->getSize(), handles[0]));
		U8* data = static_cast<U8*>(handles[0].getMappedMemory());
		ANKI_ASSERT(data);

		// Load to staging
		PtrSize offset = 0;
		for(U32 i = 0; i < m_vertBufferInfos.getSize(); ++i)
		{
			alignRoundUp(VERTEX_BUFFER_ALIGNMENT, offset);
			ANKI_CHECK(
				loader.storeVertexBuffer(i, data + offset, PtrSize(m_vertBufferInfos[i].m_stride) * m_vertCount));

			offset += PtrSize(m_vertBufferInfos[i].m_stride) * m_vertCount;
		}

		ANKI_ASSERT(offset == m_vertBuff->getSize());

		// Copy
		cmdb->copyBufferToBuffer(handles[0].getBuffer(), handles[0].getOffset(), m_vertBuff, 0, handles[0].getRange());
	}

	// Build the BLAS
	if(gr.getDeviceCapabilities().m_rayTracingEnabled)
	{
		cmdb->setBufferBarrier(m_vertBuff, BufferUsageBit::TRANSFER_DESTINATION,
							   BufferUsageBit::ACCELERATION_STRUCTURE_BUILD | BufferUsageBit::VERTEX, 0, MAX_PTR_SIZE);
		cmdb->setBufferBarrier(m_indexBuff, BufferUsageBit::TRANSFER_DESTINATION,
							   BufferUsageBit::ACCELERATION_STRUCTURE_BUILD | BufferUsageBit::INDEX, 0, MAX_PTR_SIZE);

		cmdb->setAccelerationStructureBarrier(m_blas, AccelerationStructureUsageBit::NONE,
											  AccelerationStructureUsageBit::BUILD);

		cmdb->buildAccelerationStructure(m_blas);

		cmdb->setAccelerationStructureBarrier(m_blas, AccelerationStructureUsageBit::BUILD,
											  AccelerationStructureUsageBit::ALL_READ);
	}
	else
	{
		cmdb->setBufferBarrier(m_vertBuff, BufferUsageBit::TRANSFER_DESTINATION, BufferUsageBit::VERTEX, 0,
							   MAX_PTR_SIZE);
		cmdb->setBufferBarrier(m_indexBuff, BufferUsageBit::TRANSFER_DESTINATION, BufferUsageBit::INDEX, 0,
							   MAX_PTR_SIZE);
	}

	// Finalize
	FencePtr fence;
	cmdb->flush(&fence);

	transferAlloc.release(handles[0], fence);
	transferAlloc.release(handles[1], fence);

	return Error::NONE;
}

} // end namespace anki
