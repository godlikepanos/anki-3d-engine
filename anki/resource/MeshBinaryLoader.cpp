// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MeshBinaryLoader.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

MeshBinaryLoader::MeshBinaryLoader(ResourceManager* manager)
	: MeshBinaryLoader(manager, manager->getTempAllocator())
{
}

MeshBinaryLoader::~MeshBinaryLoader()
{
	m_subMeshes.destroy(m_alloc);
}

Error MeshBinaryLoader::load(const ResourceFilename& filename)
{
	auto& alloc = m_alloc;

	// Load header
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, m_file));
	ANKI_CHECK(m_file->read(&m_header, sizeof(m_header)));
	ANKI_CHECK(checkHeader());

	// Read submesh info
	{
		m_subMeshes.create(alloc, m_header.m_subMeshCount);
		ANKI_CHECK(m_file->read(&m_subMeshes[0], m_subMeshes.getSizeInBytes()));

		// Checks
		const U32 indicesPerFace = !!(m_header.m_flags & MeshBinaryFlag::QUAD) ? 4 : 3;
		U idxSum = 0;
		for(U i = 0; i < m_subMeshes.getSize(); i++)
		{
			const MeshBinarySubMesh& sm = m_subMeshes[0];
			if(sm.m_firstIndex != idxSum || (sm.m_indexCount % indicesPerFace) != 0)
			{
				ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
				return Error::USER_DATA;
			}

			for(U d = 0; d < 3; ++d)
			{
				if(sm.m_aabbMin[i] >= sm.m_aabbMax[i])
				{
					ANKI_RESOURCE_LOGE("Wrong bounding box");
					return Error::USER_DATA;
				}
			}

			idxSum += sm.m_indexCount;
		}

		if(idxSum != m_header.m_totalIndexCount)
		{
			ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
			return Error::USER_DATA;
		}
	}

	return Error::NONE;
}

Error MeshBinaryLoader::checkFormat(VertexAttributeLocation type, ConstWeakArray<Format> supportedFormats,
									U32 vertexBufferIdx, U32 relativeOffset) const
{
	const MeshBinaryVertexAttribute& attrib = m_header.m_vertexAttributes[type];

	// Check format
	Bool found = false;
	for(Format fmt : supportedFormats)
	{
		if(fmt == attrib.m_format)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u has unsupported format %u", U32(type),
						   U32(m_header.m_vertexAttributes[type].m_format));
		return Error::USER_DATA;
	}

	if(!attrib.m_format)
	{
		// Attrib is not in use, no more checks
		return Error::NONE;
	}

	if(attrib.m_bufferBinding != vertexBufferIdx)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should belong to the %u vertex buffer", U32(type), vertexBufferIdx);
		return Error::USER_DATA;
	}

	if(attrib.m_relativeOffset != relativeOffset)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have relative vertex offset equal to %u", U32(type),
						   relativeOffset);
		return Error::USER_DATA;
	}

	// Scale should be 1.0 for now
	if(attrib.m_scale != 1.0f)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have 1.0 scale", U32(type));
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error MeshBinaryLoader::checkHeader() const
{
	const MeshBinaryHeader& h = m_header;

	// Header
	if(memcmp(&h.m_magic[0], MESH_MAGIC, 8) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong magic word");
		return Error::USER_DATA;
	}

	// Flags
	if((h.m_flags & ~MeshBinaryFlag::ALL) != MeshBinaryFlag::NONE)
	{
		ANKI_RESOURCE_LOGE("Wrong header flags");
		return Error::USER_DATA;
	}

	// Attributes
	ANKI_CHECK(checkFormat(VertexAttributeLocation::POSITION, Array<Format, 1>{{Format::R32G32B32_SFLOAT}}, 0, 0));
	ANKI_CHECK(
		checkFormat(VertexAttributeLocation::NORMAL, Array<Format, 1>{{Format::A2B10G10R10_SNORM_PACK32}}, 1, 0));
	ANKI_CHECK(
		checkFormat(VertexAttributeLocation::TANGENT, Array<Format, 1>{{Format::A2B10G10R10_SNORM_PACK32}}, 1, 4));
	ANKI_CHECK(checkFormat(VertexAttributeLocation::UV, Array<Format, 1>{{Format::R32G32_SFLOAT}}, 1, 8));
	ANKI_CHECK(checkFormat(VertexAttributeLocation::UV2, Array<Format, 1>{{Format::NONE}}, 1, 0));
	ANKI_CHECK(checkFormat(VertexAttributeLocation::BONE_INDICES,
						   Array<Format, 2>{{Format::NONE, Format::R16G16B16A16_UINT}}, 2, 0));
	ANKI_CHECK(checkFormat(VertexAttributeLocation::BONE_WEIGHTS,
						   Array<Format, 2>{{Format::NONE, Format::R8G8B8A8_UNORM}}, 2, 8));

	// Vertex buffers
	if(m_header.m_vertexBufferCount != 2 + hasBoneInfo())
	{
		ANKI_RESOURCE_LOGE("Wrong number of vertex buffers");
		return Error::USER_DATA;
	}

	if(m_header.m_vertexBuffers[0].m_vertexStride != sizeof(Vec3) || m_header.m_vertexBuffers[1].m_vertexStride != 16
	   || (hasBoneInfo() && m_header.m_vertexBuffers[2].m_vertexStride != 12))
	{
		ANKI_RESOURCE_LOGE("Some of the vertex buffers have incorrect vertex stride");
		return Error::USER_DATA;
	}

	// Indices format
	if(h.m_indexType != IndexType::U16)
	{
		ANKI_RESOURCE_LOGE("Wrong format for indices");
		return Error::USER_DATA;
	}

	// m_totalIndexCount
	const U indicesPerFace = !!(h.m_flags & MeshBinaryFlag::QUAD) ? 4 : 3;
	if(h.m_totalIndexCount == 0 || (h.m_totalIndexCount % indicesPerFace) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong index count");
		return Error::USER_DATA;
	}

	// m_totalVertexCount
	if(h.m_totalVertexCount == 0)
	{
		ANKI_RESOURCE_LOGE("Wrong vertex count");
		return Error::USER_DATA;
	}

	// m_subMeshCount
	if(h.m_subMeshCount == 0)
	{
		ANKI_RESOURCE_LOGE("Wrong submesh count");
		return Error::USER_DATA;
	}

	// AABB
	for(U d = 0; d < 3; ++d)
	{
		if(h.m_aabbMin[d] >= h.m_aabbMax[d])
		{
			ANKI_RESOURCE_LOGE("Wrong bounding box");
			return Error::USER_DATA;
		}
	}

	// Check the file size
	PtrSize totalSize = sizeof(m_header);

	totalSize += sizeof(MeshBinarySubMesh) * m_header.m_subMeshCount;
	totalSize += getIndexBufferSize();

	for(U32 i = 0; i < m_header.m_vertexBufferCount; ++i)
	{
		totalSize += getVertexBufferSize(i);
	}

	if(totalSize != m_file->getSize())
	{
		ANKI_RESOURCE_LOGE("Unexpected file size");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error MeshBinaryLoader::storeIndexBuffer(void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(size == getIndexBufferSize());

	const PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes();
	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::BEGINNING));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::NONE;
}

Error MeshBinaryLoader::storeVertexBuffer(U32 bufferIdx, void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(bufferIdx < m_header.m_vertexBufferCount);
	ANKI_ASSERT(size == getVertexBufferSize(bufferIdx));

	PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes() + getIndexBufferSize();
	for(U32 i = 0; i < bufferIdx; ++i)
	{
		seek += getVertexBufferSize(i);
	}

	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::BEGINNING));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::NONE;
}

Error MeshBinaryLoader::storeIndicesAndPosition(DynamicArrayAuto<U32>& indices, DynamicArrayAuto<Vec3>& positions)
{
	ANKI_ASSERT(isLoaded());

	// Store indices
	{
		indices.resize(m_header.m_totalIndexCount);

		// Store to staging buff
		DynamicArrayAuto<U8, PtrSize> staging(m_alloc);
		staging.create(getIndexBufferSize());
		ANKI_CHECK(storeIndexBuffer(&staging[0], staging.getSizeInBytes()));

		// Copy from staging
		ANKI_ASSERT(m_header.m_indexType == IndexType::U16);
		for(U32 i = 0; i < m_header.m_totalIndexCount; ++i)
		{
			indices[i] = *reinterpret_cast<U16*>(&staging[PtrSize(i) * 2]);
		}
	}

	// Store positions
	{
		positions.resize(m_header.m_totalVertexCount);
		const MeshBinaryVertexAttribute& attrib = m_header.m_vertexAttributes[VertexAttributeLocation::POSITION];
		ANKI_ASSERT(attrib.m_format == Format::R32G32B32_SFLOAT);
		ANKI_CHECK(storeVertexBuffer(attrib.m_bufferBinding, &positions[0], positions.getSizeInBytes()));
	}

	return Error::NONE;
}

} // end namespace anki
