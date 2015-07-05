// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Mesh.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/MeshLoader.h"
#include "anki/util/Functions.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
Mesh::~Mesh()
{
	m_subMeshes.destroy(getAllocator());
}

//==============================================================================
Bool Mesh::isCompatible(const Mesh& other) const
{
	return hasBoneWeights() == other.hasBoneWeights()
		&& getSubMeshesCount() == other.getSubMeshesCount()
		&& m_texChannelsCount == other.m_texChannelsCount;
}

//==============================================================================
Error Mesh::load(const ResourceFilename& filename)
{
	MeshLoader loader(&getManager());
	ANKI_CHECK(loader.load(filename));

	const MeshLoader::Header& header = loader.getHeader();
	m_indicesCount = header.m_totalIndicesCount;

	PtrSize vertexSize = loader.getVertexSize();
	m_obb.setFromPointCloud(loader.getVertexData(), header.m_totalVerticesCount,
		vertexSize, loader.getVertexDataSize());
	ANKI_ASSERT(m_indicesCount > 0);
	ANKI_ASSERT(m_indicesCount % 3 == 0 && "Expecting triangles");

	// Set the non-VBO members
	m_vertsCount = header.m_totalVerticesCount;
	ANKI_ASSERT(m_vertsCount > 0);

	m_texChannelsCount = header.m_uvsChannelCount;
	m_weights = loader.hasBoneInfo();

	createBuffers(loader);

	return ErrorCode::NONE;
}

//==============================================================================
void Mesh::createBuffers(const MeshLoader& loader)
{
	GrManager& gr = getManager().getGrManager();

	CommandBufferPtr cmdb = gr.newInstance<CommandBuffer>();

	// Create vertex buffer
	m_vertBuff = gr.newInstance<Buffer>(loader.getVertexDataSize(),
		BufferUsageBit::VERTEX_BUFFER, BufferAccessBit::CLIENT_WRITE);

	void* data = nullptr;
	cmdb->writeBuffer(m_vertBuff, 0, loader.getVertexDataSize(), data);
	memcpy(data, loader.getVertexData(), loader.getVertexDataSize());

	// Create index buffer
	m_indicesBuff = gr.newInstance<Buffer>(loader.getIndexDataSize(),
		BufferUsageBit::INDEX_BUFFER, BufferAccessBit::CLIENT_WRITE);

	cmdb->writeBuffer(m_indicesBuff, 0, loader.getIndexDataSize(), data);
	memcpy(data, loader.getIndexData(), loader.getIndexDataSize());

	cmdb->flush();
}

} // end namespace anki
