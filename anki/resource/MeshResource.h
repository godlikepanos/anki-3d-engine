// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/collision/Aabb.h>
#include <anki/shaders/include/ModelTypes.h>

namespace anki
{

// Forward
class MeshBinaryLoader;

/// @addtogroup resource
/// @{

/// Mesh Resource. It contains the geometry packed in GPU buffers.
class MeshResource : public ResourceObject
{
public:
	/// Default constructor
	MeshResource(ResourceManager* manager);

	~MeshResource();

	/// Helper function for correct loading
	Bool isCompatible(const MeshResource& other) const;

	/// Load from a mesh file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	/// Get the complete bounding box.
	const Aabb& getBoundingShape() const
	{
		return m_aabb;
	}

	/// Get submesh info.
	void getSubMeshInfo(U32 subMeshId, U32& firstIndex, U32& indexCount, Aabb& aabb) const
	{
		const SubMesh& sm = m_subMeshes[subMeshId];
		firstIndex = sm.m_firstIndex;
		indexCount = sm.m_indexCount;
		aabb = sm.m_aabb;
	}

	U32 getSubMeshCount() const
	{
		return m_subMeshes.getSize();
	}

	/// Get all info around vertex indices.
	void getIndexBufferInfo(BufferPtr& buff, PtrSize& buffOffset, U32& indexCount, IndexType& indexType) const
	{
		buff = m_indexBuffer;
		buffOffset = 0;
		indexCount = m_indexCount;
		indexType = m_indexType;
	}

	/// Get the number of logical vertex buffers.
	U32 getVertexBufferCount() const
	{
		return m_vertexBufferInfos.getSize();
	}

	/// Get vertex buffer info.
	void getVertexBufferInfo(const U32 buffIdx, BufferPtr& buff, PtrSize& offset, PtrSize& stride) const
	{
		buff = m_vertexBuffer;
		offset = m_vertexBufferInfos[buffIdx].m_offset;
		stride = m_vertexBufferInfos[buffIdx].m_stride;
	}

	/// Get attribute info. You need to check if the attribute is preset first (isVertexAttributePresent)
	void getVertexAttributeInfo(const VertexAttributeLocation attrib, U32& bufferIdx, Format& format,
								U32& relativeOffset) const
	{
		ANKI_ASSERT(isVertexAttributePresent(attrib));
		bufferIdx = m_attributes[attrib].m_buffIdx;
		format = m_attributes[attrib].m_format;
		relativeOffset = m_attributes[attrib].m_relativeOffset;
	}

	/// Check if a vertex attribute is present.
	Bool isVertexAttributePresent(const VertexAttributeLocation attrib) const
	{
		return !!m_attributes[attrib].m_format;
	}

	/// Return true if it has bone weights.
	Bool hasBoneWeights() const
	{
		return isVertexAttributePresent(VertexAttributeLocation::BONE_WEIGHTS);
	}

	AccelerationStructurePtr getBottomLevelAccelerationStructure() const
	{
		ANKI_ASSERT(m_blas.isCreated());
		return m_blas;
	}

	const MeshGpuDescriptor& getMeshGpuDescriptor() const
	{
		return m_meshGpuDescriptor;
	}

	/// Get the buffer that contains all the indices of all submesses.
	BufferPtr getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	/// Get the buffer that contains all the vertices of all submesses.
	BufferPtr getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

private:
	class LoadTask;
	class LoadContext;

	class SubMesh
	{
	public:
		U32 m_firstIndex;
		U32 m_indexCount;
		Aabb m_aabb;
	};

	class VertBuffInfo
	{
	public:
		PtrSize m_offset; ///< Offset from the base of m_vertBuff.
		U32 m_stride;
	};

	class AttribInfo
	{
	public:
		Format m_format = Format::NONE;
		U32 m_relativeOffset = 0;
		U32 m_buffIdx = 0;
	};

	DynamicArray<SubMesh> m_subMeshes;
	DynamicArray<VertBuffInfo> m_vertexBufferInfos;
	Array<AttribInfo, U(VertexAttributeLocation::COUNT)> m_attributes;

	BufferPtr m_indexBuffer;
	BufferPtr m_vertexBuffer;
	U32 m_indexCount = 0;
	U32 m_vertexCount = 0;
	Aabb m_aabb;
	IndexType m_indexType;

	// RT
	AccelerationStructurePtr m_blas;
	MeshGpuDescriptor m_meshGpuDescriptor;

	ANKI_USE_RESULT Error loadAsync(MeshBinaryLoader& loader) const;
};
/// @}

} // end namespace anki
