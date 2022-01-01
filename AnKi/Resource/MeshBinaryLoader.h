// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/MeshBinary.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/MeshBinary.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup resource
/// @{

/// This class loads the mesh binary file. It only supports a subset of combinations of vertex formats and buffers.
class MeshBinaryLoader
{
public:
	MeshBinaryLoader(ResourceManager* manager);

	MeshBinaryLoader(ResourceManager* manager, GenericMemoryPoolAllocator<U8> alloc)
		: m_manager(manager)
		, m_alloc(alloc)
	{
	}

	~MeshBinaryLoader();

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	ANKI_USE_RESULT Error storeIndexBuffer(void* ptr, PtrSize size);

	ANKI_USE_RESULT Error storeVertexBuffer(U32 bufferIdx, void* ptr, PtrSize size);

	/// Instead of calling storeIndexBuffer and storeVertexBuffer use this method to get those buffers into the CPU.
	ANKI_USE_RESULT Error storeIndicesAndPosition(DynamicArrayAuto<U32>& indices, DynamicArrayAuto<Vec3>& positions);

	const MeshBinaryHeader& getHeader() const
	{
		ANKI_ASSERT(isLoaded());
		return m_header;
	}

	Bool hasBoneInfo() const
	{
		ANKI_ASSERT(isLoaded());
		return m_header.m_vertexAttributes[VertexAttributeId::BONE_INDICES].m_format != Format::NONE;
	}

	ConstWeakArray<MeshBinarySubMesh> getSubMeshes() const
	{
		return ConstWeakArray<MeshBinarySubMesh>(m_subMeshes);
	}

private:
	ResourceManager* m_manager;
	GenericMemoryPoolAllocator<U8> m_alloc;
	ResourceFilePtr m_file;

	MeshBinaryHeader m_header;

	DynamicArray<MeshBinarySubMesh> m_subMeshes;

	Bool isLoaded() const
	{
		return m_file.get() != nullptr;
	}

	PtrSize getIndexBufferSize() const
	{
		ANKI_ASSERT(isLoaded());
		return PtrSize(m_header.m_totalIndexCount) * ((m_header.m_indexType == IndexType::U16) ? 2 : 4);
	}

	PtrSize getAlignedIndexBufferSize() const
	{
		ANKI_ASSERT(isLoaded());
		return getAlignedRoundUp(MESH_BINARY_BUFFER_ALIGNMENT, getIndexBufferSize());
	}

	PtrSize getVertexBufferSize(U32 bufferIdx) const
	{
		ANKI_ASSERT(isLoaded());
		ANKI_ASSERT(bufferIdx < m_header.m_vertexBufferCount);
		return PtrSize(m_header.m_totalVertexCount) * PtrSize(m_header.m_vertexBuffers[bufferIdx].m_vertexStride);
	}

	PtrSize getAlignedVertexBufferSize(U32 bufferIdx) const
	{
		ANKI_ASSERT(isLoaded());
		ANKI_ASSERT(bufferIdx < m_header.m_vertexBufferCount);
		return getAlignedRoundUp(MESH_BINARY_BUFFER_ALIGNMENT, getVertexBufferSize(bufferIdx));
	}

	ANKI_USE_RESULT Error checkHeader() const;
	ANKI_USE_RESULT Error checkFormat(VertexAttributeId type, ConstWeakArray<Format> supportedFormats,
									  U32 vertexBufferIdx, U32 relativeOffset) const;
};
/// @}

} // end namespace anki
