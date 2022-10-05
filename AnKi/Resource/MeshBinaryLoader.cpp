// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

MeshBinaryLoader::MeshBinaryLoader(ResourceManager* manager)
	: MeshBinaryLoader(manager, &manager->getTempMemoryPool())
{
}

MeshBinaryLoader::~MeshBinaryLoader()
{
	m_subMeshes.destroy(*m_pool);
}

Error MeshBinaryLoader::load(const ResourceFilename& filename)
{
	BaseMemoryPool& pool = *m_pool;

	// Load header
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, m_file));
	ANKI_CHECK(m_file->read(&m_header, sizeof(m_header)));
	ANKI_CHECK(checkHeader());

	// Read submesh info
	{
		m_subMeshes.create(pool, m_header.m_subMeshCount);
		ANKI_CHECK(m_file->read(&m_subMeshes[0], m_subMeshes.getSizeInBytes()));

		// Checks
		const U32 indicesPerFace = !!(m_header.m_flags & MeshBinaryFlag::kQuad) ? 4 : 3;
		U idxSum = 0;
		for(U32 i = 0; i < m_subMeshes.getSize(); i++)
		{
			const MeshBinarySubMesh& sm = m_subMeshes[i];
			if(sm.m_firstIndex != idxSum || (sm.m_indexCount % indicesPerFace) != 0)
			{
				ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
				return Error::kUserData;
			}

			for(U d = 0; d < 3; ++d)
			{
				if(sm.m_aabbMin[d] >= sm.m_aabbMax[d])
				{
					ANKI_RESOURCE_LOGE("Wrong bounding box");
					return Error::kUserData;
				}
			}

			idxSum += sm.m_indexCount;
		}

		if(idxSum != m_header.m_totalIndexCount)
		{
			ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
			return Error::kUserData;
		}
	}

	return Error::kNone;
}

Error MeshBinaryLoader::checkFormat(VertexAttributeId type, ConstWeakArray<Format> supportedFormats,
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
		return Error::kUserData;
	}

	if(!attrib.m_format)
	{
		// Attrib is not in use, no more checks
		return Error::kNone;
	}

	if(attrib.m_bufferBinding != vertexBufferIdx)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should belong to the %u vertex buffer", U32(type), vertexBufferIdx);
		return Error::kUserData;
	}

	if(attrib.m_relativeOffset != relativeOffset)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have relative vertex offset equal to %u", U32(type),
						   relativeOffset);
		return Error::kUserData;
	}

	// Scale should be 1.0 for now
	if(attrib.m_scale != 1.0f)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have 1.0 scale", U32(type));
		return Error::kUserData;
	}

	return Error::kNone;
}

Error MeshBinaryLoader::checkHeader() const
{
	const MeshBinaryHeader& h = m_header;

	// Header
	if(memcmp(&h.m_magic[0], kMeshMagic, 8) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong magic word");
		return Error::kUserData;
	}

	// Flags
	if((h.m_flags & ~MeshBinaryFlag::kAll) != MeshBinaryFlag::kNone)
	{
		ANKI_RESOURCE_LOGE("Wrong header flags");
		return Error::kUserData;
	}

	// Attributes
	ANKI_CHECK(checkFormat(VertexAttributeId::kPosition, Array<Format, 1>{{Format::kR32G32B32_Sfloat}}, 0, 0));
	ANKI_CHECK(checkFormat(VertexAttributeId::kNormal, Array<Format, 1>{{Format::kA2B10G10R10_Snorm_Pack32}}, 1, 0));
	ANKI_CHECK(checkFormat(VertexAttributeId::kTangent, Array<Format, 1>{{Format::kA2B10G10R10_Snorm_Pack32}}, 1, 4));
	ANKI_CHECK(checkFormat(VertexAttributeId::kUv0, Array<Format, 1>{{Format::kR32G32_Sfloat}}, 1, 8));
	ANKI_CHECK(checkFormat(VertexAttributeId::kUv1, Array<Format, 1>{{Format::kNone}}, 1, 0));
	ANKI_CHECK(
		checkFormat(VertexAttributeId::kBoneIndices, Array<Format, 2>{{Format::kNone, Format::kR8G8B8A8_Uint}}, 2, 0));
	ANKI_CHECK(
		checkFormat(VertexAttributeId::kBoneWeights, Array<Format, 2>{{Format::kNone, Format::kR8G8B8A8_Unorm}}, 2, 4));

	// Vertex buffers
	if(m_header.m_vertexBufferCount != 2 + U32(hasBoneInfo()))
	{
		ANKI_RESOURCE_LOGE("Wrong number of vertex buffers");
		return Error::kUserData;
	}

	if(m_header.m_vertexBuffers[0].m_vertexStride != sizeof(Vec3) || m_header.m_vertexBuffers[1].m_vertexStride != 16
	   || (hasBoneInfo() && m_header.m_vertexBuffers[2].m_vertexStride != 8))
	{
		ANKI_RESOURCE_LOGE("Some of the vertex buffers have incorrect vertex stride");
		return Error::kUserData;
	}

	// Indices format
	if(h.m_indexType != IndexType::kU16)
	{
		ANKI_RESOURCE_LOGE("Wrong format for indices");
		return Error::kUserData;
	}

	// m_totalIndexCount
	const U indicesPerFace = !!(h.m_flags & MeshBinaryFlag::kQuad) ? 4 : 3;
	if(h.m_totalIndexCount == 0 || (h.m_totalIndexCount % indicesPerFace) != 0)
	{
		ANKI_RESOURCE_LOGE("Wrong index count");
		return Error::kUserData;
	}

	// m_totalVertexCount
	if(h.m_totalVertexCount == 0)
	{
		ANKI_RESOURCE_LOGE("Wrong vertex count");
		return Error::kUserData;
	}

	// m_subMeshCount
	if(h.m_subMeshCount == 0)
	{
		ANKI_RESOURCE_LOGE("Wrong submesh count");
		return Error::kUserData;
	}

	// AABB
	for(U d = 0; d < 3; ++d)
	{
		if(h.m_aabbMin[d] >= h.m_aabbMax[d])
		{
			ANKI_RESOURCE_LOGE("Wrong bounding box");
			return Error::kUserData;
		}
	}

	// Check the file size
	PtrSize totalSize = sizeof(m_header);

	totalSize += sizeof(MeshBinarySubMesh) * m_header.m_subMeshCount;
	totalSize += getAlignedIndexBufferSize();

	for(U32 i = 0; i < m_header.m_vertexBufferCount; ++i)
	{
		totalSize += getAlignedVertexBufferSize(i);
	}

	if(totalSize != m_file->getSize())
	{
		ANKI_RESOURCE_LOGE("Unexpected file size");
		return Error::kUserData;
	}

	return Error::kNone;
}

Error MeshBinaryLoader::storeIndexBuffer(void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(size == getIndexBufferSize());

	const PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes();
	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::kNone;
}

Error MeshBinaryLoader::storeVertexBuffer(U32 bufferIdx, void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(bufferIdx < m_header.m_vertexBufferCount);
	ANKI_ASSERT(size == getVertexBufferSize(bufferIdx));

	PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes() + getAlignedIndexBufferSize();
	for(U32 i = 0; i < bufferIdx; ++i)
	{
		seek += getAlignedVertexBufferSize(i);
	}

	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::kNone;
}

Error MeshBinaryLoader::storeIndicesAndPosition(DynamicArrayRaii<U32>& indices, DynamicArrayRaii<Vec3>& positions)
{
	ANKI_ASSERT(isLoaded());

	// Store indices
	{
		indices.resize(m_header.m_totalIndexCount);

		// Store to staging buff
		DynamicArrayRaii<U8, PtrSize> staging(m_pool);
		staging.create(getIndexBufferSize());
		ANKI_CHECK(storeIndexBuffer(&staging[0], staging.getSizeInBytes()));

		// Copy from staging
		ANKI_ASSERT(m_header.m_indexType == IndexType::kU16);
		for(U32 i = 0; i < m_header.m_totalIndexCount; ++i)
		{
			indices[i] = *reinterpret_cast<U16*>(&staging[PtrSize(i) * 2]);
		}
	}

	// Store positions
	{
		positions.resize(m_header.m_totalVertexCount);
		const MeshBinaryVertexAttribute& attrib = m_header.m_vertexAttributes[VertexAttributeId::kPosition];
		ANKI_ASSERT(attrib.m_format == Format::kR32G32B32_Sfloat);
		ANKI_CHECK(storeVertexBuffer(attrib.m_bufferBinding, &positions[0], positions.getSizeInBytes()));
	}

	return Error::kNone;
}

} // end namespace anki
