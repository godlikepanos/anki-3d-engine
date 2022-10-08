// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/MeshBinary.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/MeshBinary.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Shaders/Include/MeshTypes.h>

namespace anki {

/// @addtogroup resource
/// @{

/// This class loads the mesh binary file. It only supports a subset of combinations of vertex formats and buffers.
/// The file is layed out in memory:
/// * Header
/// * Submeshes
/// * Index buffer of max LOD
/// * Vertex buffer #0 of max LOD
/// * etc...
class MeshBinaryLoader
{
public:
	MeshBinaryLoader(ResourceManager* manager);

	MeshBinaryLoader(ResourceManager* manager, BaseMemoryPool* pool)
		: m_manager(manager)
		, m_pool(pool)
	{
		ANKI_ASSERT(manager && pool);
	}

	~MeshBinaryLoader();

	Error load(const ResourceFilename& filename);

	Error storeIndexBuffer(U32 lod, void* ptr, PtrSize size);

	Error storeVertexBuffer(U32 lod, U32 bufferIdx, void* ptr, PtrSize size);

	/// Instead of calling storeIndexBuffer and storeVertexBuffer use this method to get those buffers into the CPU.
	Error storeIndicesAndPosition(U32 lod, DynamicArrayRaii<U32>& indices, DynamicArrayRaii<Vec3>& positions);

	const MeshBinaryHeader& getHeader() const
	{
		ANKI_ASSERT(isLoaded());
		return m_header;
	}

	ConstWeakArray<MeshBinarySubMesh> getSubMeshes() const
	{
		return ConstWeakArray<MeshBinarySubMesh>(m_subMeshes);
	}

private:
	ResourceManager* m_manager = nullptr;
	BaseMemoryPool* m_pool = nullptr;
	ResourceFilePtr m_file;

	MeshBinaryHeader m_header;

	DynamicArray<MeshBinarySubMesh> m_subMeshes;

	Bool isLoaded() const
	{
		return m_file.get() != nullptr;
	}

	PtrSize getIndexBufferSize(U32 lod) const
	{
		ANKI_ASSERT(isLoaded());
		ANKI_ASSERT(lod < m_header.m_lodCount);
		return PtrSize(m_header.m_totalIndexCounts[lod]) * getIndexSize(m_header.m_indexType);
	}

	PtrSize getVertexBufferSize(U32 lod, U32 bufferIdx) const
	{
		ANKI_ASSERT(isLoaded());
		ANKI_ASSERT(lod < m_header.m_lodCount);
		return PtrSize(m_header.m_totalVertexCounts[lod]) * PtrSize(m_header.m_vertexBuffers[bufferIdx].m_vertexStride);
	}

	PtrSize getLodBuffersSize(U32 lod) const;

	Error checkHeader() const;
	Error checkFormat(VertexStreamId stream, Bool isOptional) const;
	Error loadSubmeshes();
};
/// @}

} // end namespace anki
