// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MeshBinaryLoader.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

static Error checkBoundingVolume(const MeshBinaryBoundingVolume& vol)
{
	U32 wrong = 0;

	if(vol.m_aabbMax <= vol.m_aabbMin)
	{
		++wrong;
	}

	if(vol.m_sphereRadius <= 0.0f)
	{
		++wrong;
	}

	if(wrong)
	{
		ANKI_RESOURCE_LOGE("Wrong bounding volume");
		return Error::kUserData;
	}

	return Error::kNone;
}

Error MeshBinaryLoader::load(const ResourceFilename& filename)
{
	// Load header + submeshes
	ANKI_CHECK(ResourceFilesystem::getSingleton().openFile(filename, m_file));
	ANKI_CHECK(m_file->read(&m_header, sizeof(m_header)));
	ANKI_CHECK(checkHeader());
	ANKI_CHECK(loadSubmeshes());

	return Error::kNone;
}

Error MeshBinaryLoader::loadSubmeshes()
{
	m_subMeshes.resize(m_header.m_subMeshCount);
	ANKI_CHECK(m_file->read(&m_subMeshes[0], m_subMeshes.getSizeInBytes()));

	// Checks
	const U32 indicesPerFace = 3;

	for(U32 lod = 0; lod < m_header.m_lodCount; ++lod)
	{
		U idxSum = 0;
		U meshletCount = 0;
		for(U32 i = 0; i < m_subMeshes.getSize(); i++)
		{
			const MeshBinarySubMesh& sm = m_subMeshes[i];
			if(sm.m_lods[lod].m_firstIndex != idxSum || (sm.m_lods[lod].m_indexCount % indicesPerFace) != 0)
			{
				ANKI_RESOURCE_LOGE("Incorrect sub mesh info");
				return Error::kUserData;
			}

			ANKI_CHECK(checkBoundingVolume(sm.m_boundingVolume));

			idxSum += sm.m_lods[lod].m_indexCount;
			meshletCount += sm.m_lods[lod].m_meshletCount;
		}

		if(idxSum != m_header.m_indexCounts[lod])
		{
			ANKI_RESOURCE_LOGE("Submesh index count doesn't add up to the total");
			return Error::kUserData;
		}

		if(meshletCount != m_header.m_meshletCounts[lod])
		{
			ANKI_RESOURCE_LOGE("Submesh meshlet count doesn't add up to the total");
			return Error::kUserData;
		}
	}

	return Error::kNone;
}

Error MeshBinaryLoader::checkFormat(VertexStreamId stream, Bool isOptional, Bool canBeTransformed) const
{
	const U32 vertexAttribIdx = U32(stream);
	const U32 vertexBufferIdx = U32(stream);
	const MeshBinaryVertexAttribute& attrib = m_header.m_vertexAttributes[vertexAttribIdx];

	// Check format
	if(isOptional && attrib.m_format == Format::kNone)
	{
		// Attrib is not in use, no more checks
		return Error::kNone;
	}

	if(attrib.m_format != kMeshRelatedVertexStreamFormats[stream])
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u has unsupported format %s", vertexAttribIdx, getFormatInfo(attrib.m_format).m_name);
		return Error::kUserData;
	}

	if(attrib.m_bufferIndex != vertexBufferIdx)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should belong to the %u vertex buffer", vertexAttribIdx, vertexBufferIdx);
		return Error::kUserData;
	}

	if(attrib.m_relativeOffset != 0)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have relative vertex offset equal to 0", vertexAttribIdx);
		return Error::kUserData;
	}

	if(!canBeTransformed && Vec4(attrib.m_scale) != Vec4(1.0f))
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have 1.0 scale", vertexAttribIdx);
		return Error::kUserData;
	}

	if(canBeTransformed && Vec4(attrib.m_scale) <= kEpsilonf)
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have positive scale", vertexAttribIdx);
		return Error::kUserData;
	}

	if(canBeTransformed
	   && (attrib.m_scale[0] != attrib.m_scale[1] || attrib.m_scale[0] != attrib.m_scale[2] || attrib.m_scale[0] != attrib.m_scale[3]))
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have uniform scale", vertexAttribIdx);
		return Error::kUserData;
	}

	if(!canBeTransformed && Vec4(attrib.m_translation) != Vec4(0.0f))
	{
		ANKI_RESOURCE_LOGE("Vertex attribute %u should have 0.0 translation", vertexAttribIdx);
		return Error::kUserData;
	}

	const U32 vertexBufferStride = getFormatInfo(attrib.m_format).m_texelSize;
	if(m_header.m_vertexBuffers[vertexBufferIdx].m_vertexStride != vertexBufferStride)
	{
		ANKI_RESOURCE_LOGE("Vertex buffer %u doesn't have the expected stride of %u", vertexBufferIdx, vertexBufferStride);
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
	ANKI_CHECK(checkFormat(VertexStreamId::kPosition, false, true));
	ANKI_CHECK(checkFormat(VertexStreamId::kNormal, false, false));
	ANKI_CHECK(checkFormat(VertexStreamId::kUv, false, false));
	ANKI_CHECK(checkFormat(VertexStreamId::kBoneIds, true, false));
	ANKI_CHECK(checkFormat(VertexStreamId::kBoneWeights, true, false));

	// Vertex buffers
	const Format boneIdxFormat = m_header.m_vertexAttributes[VertexStreamId::kBoneIds].m_format;
	const Format boneWeightsFormat = m_header.m_vertexAttributes[VertexStreamId::kBoneWeights].m_format;
	if((boneIdxFormat == Format::kNone && boneWeightsFormat != Format::kNone)
	   || (boneWeightsFormat == Format::kNone && boneIdxFormat != Format::kNone))
	{
		ANKI_RESOURCE_LOGE("Bone buffers are partially present");
		return Error::kUserData;
	}

	// LOD
	if(h.m_lodCount == 0 || h.m_lodCount > kMaxLodCount)
	{
		ANKI_RESOURCE_LOGE("Wrong LOD count");
		return Error::kUserData;
	}

	// Indices format
	if(h.m_indexType != IndexType::kU16)
	{
		ANKI_RESOURCE_LOGE("Wrong format for indices");
		return Error::kUserData;
	}

	if(h.m_meshletPrimitiveFormat != kMeshletPrimitiveFormat)
	{
		ANKI_RESOURCE_LOGE("Wrong format for meshlet primitives");
		return Error::kUserData;
	}

	// m_totalIndexCount
	for(U32 lod = 0; lod < h.m_lodCount; ++lod)
	{
		const U indicesPerFace = 3;
		if(h.m_indexCounts[lod] == 0 || (h.m_indexCounts[lod] % indicesPerFace) != 0)
		{
			ANKI_RESOURCE_LOGE("Wrong index count");
			return Error::kUserData;
		}
	}

	// m_totalVertexCount
	for(U32 lod = 0; lod < h.m_lodCount; ++lod)
	{
		if(h.m_vertexCounts[lod] == 0)
		{
			ANKI_RESOURCE_LOGE("Wrong vertex count");
			return Error::kUserData;
		}
	}

	for(U32 lod = 0; lod < h.m_lodCount; ++lod)
	{
		if(h.m_meshletPrimitiveCounts[lod] == 0 || h.m_meshletCounts[lod] == 0)
		{
			ANKI_RESOURCE_LOGE("Wrong meshlet counts");
			return Error::kUserData;
		}
	}

	// m_subMeshCount
	if(h.m_subMeshCount == 0)
	{
		ANKI_RESOURCE_LOGE("Wrong submesh count");
		return Error::kUserData;
	}

	if(h.m_maxPrimitivesPerMeshlet != kMaxPrimitivesPerMeshlet || h.m_maxVerticesPerMeshlet != kMaxVerticesPerMeshlet)
	{
		ANKI_RESOURCE_LOGE("Wrong max primitives or max vertices per meshlet");
		return Error::kUserData;
	}

	// AABB
	ANKI_CHECK(checkBoundingVolume(h.m_boundingVolume));

	// Check the file size
	PtrSize totalSize = sizeof(m_header);
	totalSize += sizeof(MeshBinarySubMesh) * m_header.m_subMeshCount;

	for(U32 lod = 0; lod < h.m_lodCount; ++lod)
	{
		totalSize += getLodBuffersSize(lod);
	}

	if(totalSize != m_file->getSize())
	{
		ANKI_RESOURCE_LOGE("Unexpected file size");
		return Error::kUserData;
	}

	return Error::kNone;
}

Error MeshBinaryLoader::storeIndexBuffer(U32 lod, void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(lod < m_header.m_lodCount);
	ANKI_ASSERT(size == getIndexBufferSize(lod));

	PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes();
	for(U32 l = lod + 1; l < m_header.m_lodCount; ++l)
	{
		seek += getLodBuffersSize(l);
	}

	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::kNone;
}

Error MeshBinaryLoader::storeVertexBuffer(U32 lod, U32 bufferIdx, void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(size == getVertexBufferSize(lod, bufferIdx));
	ANKI_ASSERT(lod < m_header.m_lodCount);

	PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes();

	for(U32 l = lod + 1; l < m_header.m_lodCount; ++l)
	{
		seek += getLodBuffersSize(l);
	}

	seek += getIndexBufferSize(lod);

	for(U32 i = 0; i < bufferIdx; ++i)
	{
		seek += getVertexBufferSize(lod, i);
	}

	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::kNone;
}

Error MeshBinaryLoader::storeMeshletIndicesBuffer(U32 lod, void* ptr, PtrSize size)
{
	ANKI_ASSERT(ptr);
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(size == getMeshletPrimitivesBufferSize(lod));
	ANKI_ASSERT(lod < m_header.m_lodCount);

	PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes();

	for(U32 l = lod + 1; l < m_header.m_lodCount; ++l)
	{
		seek += getLodBuffersSize(l);
	}

	seek += getIndexBufferSize(lod);

	for(U32 i = 0; i < m_header.m_vertexBuffers.getSize(); ++i)
	{
		seek += getVertexBufferSize(lod, i);
	}

	seek += getMeshletsBufferSize(lod);

	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->read(ptr, size));

	return Error::kNone;
}

Error MeshBinaryLoader::storeMeshletBuffer(U32 lod, WeakArray<MeshBinaryMeshlet> out)
{
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(out.getSizeInBytes() == getMeshletsBufferSize(lod));
	ANKI_ASSERT(lod < m_header.m_lodCount);

	PtrSize seek = sizeof(m_header) + m_subMeshes.getSizeInBytes();

	for(U32 l = lod + 1; l < m_header.m_lodCount; ++l)
	{
		seek += getLodBuffersSize(l);
	}

	seek += getIndexBufferSize(lod);

	for(U32 i = 0; i < m_header.m_vertexBuffers.getSize(); ++i)
	{
		seek += getVertexBufferSize(lod, i);
	}

	ANKI_CHECK(m_file->seek(seek, FileSeekOrigin::kBeginning));
	ANKI_CHECK(m_file->read(&out[0], out.getSizeInBytes()));

	return Error::kNone;
}

Error MeshBinaryLoader::storeIndicesAndPosition(U32 lod, ResourceDynamicArray<U32>& indices, ResourceDynamicArray<Vec3>& positions)
{
	ANKI_ASSERT(isLoaded());
	ANKI_ASSERT(lod < m_header.m_lodCount);

	// Store indices
	{
		indices.resize(m_header.m_indexCounts[lod]);

		// Store to staging buff
		DynamicArray<U8, MemoryPoolPtrWrapper<BaseMemoryPool>, PtrSize> staging(m_subMeshes.getMemoryPool());
		staging.resize(getIndexBufferSize(lod));
		ANKI_CHECK(storeIndexBuffer(lod, &staging[0], staging.getSizeInBytes()));

		// Copy from staging
		ANKI_ASSERT(m_header.m_indexType == IndexType::kU16);
		for(U32 i = 0; i < m_header.m_indexCounts[lod]; ++i)
		{
			indices[i] = *reinterpret_cast<U16*>(&staging[PtrSize(i) * 2]);
		}
	}

	// Store positions
	{
		const MeshBinaryVertexAttribute& attrib = m_header.m_vertexAttributes[VertexStreamId::kPosition];
		DynamicArray<U16Vec4, MemoryPoolPtrWrapper<BaseMemoryPool>> tempPositions(m_subMeshes.getMemoryPool());
		tempPositions.resize(m_header.m_vertexCounts[lod]);
		static_assert(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition] == Format::kR16G16B16A16_Unorm, "Incorrect format");
		ANKI_CHECK(storeVertexBuffer(lod, attrib.m_bufferIndex, &tempPositions[0], tempPositions.getSizeInBytes()));

		positions.resize(m_header.m_vertexCounts[lod]);

		for(U32 i = 0; i < tempPositions.getSize(); ++i)
		{
			positions[i] = Vec3(tempPositions[i].xyz()) / F32(kMaxU16);
			positions[i] *= Vec3(&attrib.m_scale[0]);
			positions[i] += Vec3(&attrib.m_translation[0]);
		}
	}

	return Error::kNone;
}

PtrSize MeshBinaryLoader::getLodBuffersSize(U32 lod) const
{
	ANKI_ASSERT(lod < m_header.m_lodCount);

	PtrSize size = getIndexBufferSize(lod);

	size += getMeshletsBufferSize(lod);
	size += getMeshletPrimitivesBufferSize(lod);

	for(U32 vertBufferIdx = 0; vertBufferIdx < m_header.m_vertexBuffers.getSize(); ++vertBufferIdx)
	{
		if(m_header.m_vertexBuffers[vertBufferIdx].m_vertexStride > 0)
		{
			size += getVertexBufferSize(lod, vertBufferIdx);
		}
	}

	return size;
}

} // end namespace anki
