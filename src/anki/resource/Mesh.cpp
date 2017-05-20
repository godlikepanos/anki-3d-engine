// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Mesh.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MeshLoader.h>
#include <anki/resource/AsyncLoader.h>
#include <anki/util/Functions.h>
#include <anki/misc/Xml.h>

namespace anki
{

/// Mesh upload async task.
class MeshLoadTask : public AsyncLoaderTask
{
public:
	ResourceManager* m_manager ANKI_DBG_NULLIFY;
	MeshLoader m_loader;
	BufferPtr m_vertBuff;
	BufferPtr m_indicesBuff;

	MeshLoadTask(ResourceManager* manager)
		: m_manager(manager)
		, m_loader(manager, manager->getAsyncLoader().getAllocator())
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx) final;
};

Error MeshLoadTask::operator()(AsyncLoaderTaskContext& ctx)
{
	GrManager& gr = m_manager->getGrManager();
	TransferGpuAllocator& transferAlloc = m_manager->getTransferGpuAllocator();
	Array<TransferGpuAllocatorHandle, 2> handles;

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>(cmdbinit);

	// Write vert buff
	{
		ANKI_CHECK(transferAlloc.allocate(m_loader.getVertexDataSize(), handles[0]));
		void* data = handles[0].getMappedMemory();
		ANKI_ASSERT(data);

		memcpy(data, m_loader.getVertexData(), m_loader.getVertexDataSize());

		cmdb->setBufferBarrier(
			m_vertBuff, BufferUsageBit::VERTEX, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, 0, MAX_PTR_SIZE);

		cmdb->copyBufferToBuffer(handles[0].getBuffer(), handles[0].getOffset(), m_vertBuff, 0, handles[0].getRange());

		cmdb->setBufferBarrier(
			m_vertBuff, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferUsageBit::VERTEX, 0, MAX_PTR_SIZE);

		m_vertBuff.reset(nullptr);
	}

	// Create index buffer
	{
		ANKI_CHECK(transferAlloc.allocate(m_loader.getIndexDataSize(), handles[1]));
		void* data = handles[1].getMappedMemory();
		ANKI_ASSERT(data);

		memcpy(data, m_loader.getIndexData(), m_loader.getIndexDataSize());

		cmdb->setBufferBarrier(
			m_indicesBuff, BufferUsageBit::INDEX, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, 0, MAX_PTR_SIZE);

		cmdb->copyBufferToBuffer(
			handles[1].getBuffer(), handles[1].getOffset(), m_indicesBuff, 0, handles[1].getRange());

		cmdb->setBufferBarrier(
			m_indicesBuff, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferUsageBit::INDEX, 0, MAX_PTR_SIZE);

		m_indicesBuff.reset(nullptr);
	}

	FencePtr fence;
	cmdb->flush(&fence);

	transferAlloc.release(handles[0], fence);
	transferAlloc.release(handles[1], fence);

	return ErrorCode::NONE;
}

Mesh::Mesh(ResourceManager* manager)
	: ResourceObject(manager)
{
}

Mesh::~Mesh()
{
	m_subMeshes.destroy(getAllocator());
}

Bool Mesh::isCompatible(const Mesh& other) const
{
	return hasBoneWeights() == other.hasBoneWeights() && getSubMeshesCount() == other.getSubMeshesCount()
		&& m_texChannelsCount == other.m_texChannelsCount;
}

Error Mesh::load(const ResourceFilename& filename)
{
	MeshLoadTask* task = getManager().getAsyncLoader().newTask<MeshLoadTask>(&getManager());

	MeshLoader& loader = task->m_loader;
	ANKI_CHECK(loader.load(filename));

	const MeshLoader::Header& header = loader.getHeader();
	m_indicesCount = header.m_totalIndicesCount;

	PtrSize vertexSize = loader.getVertexSize();
	m_obb.setFromPointCloud(
		loader.getVertexData(), header.m_totalVerticesCount, vertexSize, loader.getVertexDataSize());
	ANKI_ASSERT(m_indicesCount > 0);
	ANKI_ASSERT(m_indicesCount % 3 == 0 && "Expecting triangles");

	// Set the non-VBO members
	m_vertsCount = header.m_totalVerticesCount;
	ANKI_ASSERT(m_vertsCount > 0);

	m_texChannelsCount = header.m_uvsChannelCount;
	m_weights = loader.hasBoneInfo();

	// Allocate the buffers
	GrManager& gr = getManager().getGrManager();

	m_vertBuff = gr.newInstance<Buffer>(loader.getVertexDataSize(),
		BufferUsageBit::VERTEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION | BufferUsageBit::FILL,
		BufferMapAccessBit::NONE);

	m_indicesBuff = gr.newInstance<Buffer>(loader.getIndexDataSize(),
		BufferUsageBit::INDEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION | BufferUsageBit::FILL,
		BufferMapAccessBit::NONE);

	// Clear them
	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>(cmdbinit);

	cmdb->fillBuffer(m_vertBuff, 0, MAX_PTR_SIZE, 0);
	cmdb->fillBuffer(m_indicesBuff, 0, MAX_PTR_SIZE, 0);

	cmdb->setBufferBarrier(m_vertBuff, BufferUsageBit::FILL, BufferUsageBit::VERTEX, 0, MAX_PTR_SIZE);
	cmdb->setBufferBarrier(m_indicesBuff, BufferUsageBit::FILL, BufferUsageBit::INDEX, 0, MAX_PTR_SIZE);

	cmdb->flush();

	// Submit the loading task
	task->m_indicesBuff = m_indicesBuff;
	task->m_vertBuff = m_vertBuff;
	getManager().getAsyncLoader().submitTask(task);

	return ErrorCode::NONE;
}

} // end namespace anki
