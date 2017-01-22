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
#include <anki/core/StagingGpuMemoryManager.h>

namespace anki
{

/// Mesh upload async task.
class MeshLoadTask : public AsyncLoaderTask
{
public:
	ResourceManager* m_manager ANKI_DBG_NULLIFY;
	BufferPtr m_vertBuff;
	BufferPtr m_indicesBuff;
	MeshLoader m_loader;

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
	StagingGpuMemoryManager& stagingMem = m_manager->getStagingGpuMemoryManager();
	CommandBufferPtr cmdb;

	// Write vert buff
	if(m_vertBuff)
	{
		StagingGpuMemoryToken token;
		void* data = stagingMem.tryAllocateFrame(m_loader.getVertexDataSize(), StagingGpuMemoryType::TRANSFER, token);

		if(data)
		{
			memcpy(data, m_loader.getVertexData(), m_loader.getVertexDataSize());
			cmdb = gr.newInstance<CommandBuffer>(CommandBufferInitInfo());

			cmdb->setBufferBarrier(
				m_vertBuff, BufferUsageBit::VERTEX, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, 0, MAX_PTR_SIZE);

			cmdb->copyBufferToBuffer(token.m_buffer, token.m_offset, m_vertBuff, 0, token.m_range);

			cmdb->setBufferBarrier(
				m_vertBuff, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferUsageBit::VERTEX, 0, MAX_PTR_SIZE);

			m_vertBuff.reset(nullptr);
		}
		else
		{
			ctx.m_pause = true;
			ctx.m_resubmitTask = true;
			return ErrorCode::NONE;
		}
	}

	// Create index buffer
	{
		StagingGpuMemoryToken token;
		void* data = stagingMem.tryAllocateFrame(m_loader.getIndexDataSize(), StagingGpuMemoryType::TRANSFER, token);

		if(data)
		{
			memcpy(data, m_loader.getIndexData(), m_loader.getIndexDataSize());

			if(!cmdb)
			{
				cmdb = gr.newInstance<CommandBuffer>(CommandBufferInitInfo());
			}

			cmdb->setBufferBarrier(
				m_indicesBuff, BufferUsageBit::INDEX, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, 0, MAX_PTR_SIZE);

			cmdb->copyBufferToBuffer(token.m_buffer, token.m_offset, m_indicesBuff, 0, token.m_range);

			cmdb->setBufferBarrier(
				m_indicesBuff, BufferUsageBit::BUFFER_UPLOAD_DESTINATION, BufferUsageBit::INDEX, 0, MAX_PTR_SIZE);

			cmdb->flush();
		}
		else
		{
			// Submit prev work
			if(cmdb)
			{
				cmdb->flush();
			}

			ctx.m_pause = true;
			ctx.m_resubmitTask = true;
			return ErrorCode::NONE;
		}
	}

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
