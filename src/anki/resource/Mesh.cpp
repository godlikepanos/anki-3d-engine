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

class Mesh::LoadContext
{
public:
	ResourceManager* m_manager ANKI_DBG_NULLIFY;
	MeshLoader m_loader;
	BufferPtr m_vertBuff;
	BufferPtr m_indicesBuff;

	LoadContext(ResourceManager* manager, GenericMemoryPoolAllocator<U8> alloc)
		: m_manager(manager)
		, m_loader(manager, alloc)
	{
	}
};

/// Mesh upload async task.
class Mesh::LoadTask : public AsyncLoaderTask
{
public:
	Mesh::LoadContext m_ctx;

	LoadTask(ResourceManager* manager)
		: m_ctx(manager, manager->getAsyncLoader().getAllocator())
	{
	}

	Error operator()(AsyncLoaderTaskContext& ctx) final
	{
		return Mesh::load(m_ctx);
	}
};

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

Error Mesh::load(const ResourceFilename& filename, Bool async)
{
	LoadTask* task;
	LoadContext* ctx;
	LoadContext localCtx(&getManager(), getTempAllocator());

	if(async)
	{
		task = getManager().getAsyncLoader().newTask<LoadTask>(&getManager());
		ctx = &task->m_ctx;
	}
	else
	{
		task = nullptr;
		ctx = &localCtx;
	}

	MeshLoader& loader = ctx->m_loader;
	ANKI_CHECK(loader.load(filename));

	const MeshLoader::Header& header = loader.getHeader();
	m_indicesCount = header.m_totalIndicesCount;

	m_vertSize = loader.getVertexSize();

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

	m_vertBuff = gr.newInstance<Buffer>(BufferInitInfo(loader.getVertexDataSize(),
		BufferUsageBit::VERTEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION | BufferUsageBit::FILL,
		BufferMapAccessBit::NONE,
		"MeshVert"));

	m_indicesBuff = gr.newInstance<Buffer>(BufferInitInfo(loader.getIndexDataSize(),
		BufferUsageBit::INDEX | BufferUsageBit::BUFFER_UPLOAD_DESTINATION | BufferUsageBit::FILL,
		BufferMapAccessBit::NONE,
		"MeshIdx"));

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
	ctx->m_vertBuff = m_vertBuff;
	ctx->m_indicesBuff = m_indicesBuff;

	if(async)
	{
		getManager().getAsyncLoader().submitTask(task);
	}
	else
	{
		ANKI_CHECK(load(*ctx));
	}

	return Error::NONE;
}

Error Mesh::load(LoadContext& ctx)
{
	GrManager& gr = ctx.m_manager->getGrManager();
	TransferGpuAllocator& transferAlloc = ctx.m_manager->getTransferGpuAllocator();
	const MeshLoader& loader = ctx.m_loader;
	Array<TransferGpuAllocatorHandle, 2> handles;

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>(cmdbinit);

	// Set barriers
	cmdb->setBufferBarrier(
		ctx.m_vertBuff, BufferUsageBit::VERTEX, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, 0, MAX_PTR_SIZE);
	cmdb->setBufferBarrier(
		ctx.m_indicesBuff, BufferUsageBit::INDEX, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, 0, MAX_PTR_SIZE);

	// Write vert buff
	{
		ANKI_CHECK(transferAlloc.allocate(loader.getVertexDataSize(), handles[0]));
		void* data = handles[0].getMappedMemory();
		ANKI_ASSERT(data);

		memcpy(data, loader.getVertexData(), loader.getVertexDataSize());

		cmdb->copyBufferToBuffer(
			handles[0].getBuffer(), handles[0].getOffset(), ctx.m_vertBuff, 0, handles[0].getRange());
	}

	// Create index buffer
	{
		ANKI_CHECK(transferAlloc.allocate(loader.getIndexDataSize(), handles[1]));
		void* data = handles[1].getMappedMemory();
		ANKI_ASSERT(data);

		memcpy(data, loader.getIndexData(), loader.getIndexDataSize());

		cmdb->copyBufferToBuffer(
			handles[1].getBuffer(), handles[1].getOffset(), ctx.m_indicesBuff, 0, handles[1].getRange());
	}

	// Set barriers
	cmdb->setBufferBarrier(
		ctx.m_vertBuff, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferUsageBit::VERTEX, 0, MAX_PTR_SIZE);
	cmdb->setBufferBarrier(
		ctx.m_indicesBuff, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferUsageBit::INDEX, 0, MAX_PTR_SIZE);

	ctx.m_vertBuff.reset(nullptr);
	ctx.m_indicesBuff.reset(nullptr);

	FencePtr fence;
	cmdb->flush(&fence);

	transferAlloc.release(handles[0], fence);
	transferAlloc.release(handles[1], fence);

	return Error::NONE;
}

} // end namespace anki
