// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/collision/Obb.h>
#include <anki/shaders/include/ModelTypes.h>

namespace anki
{

// Forward
class MeshLoader;

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
	const Obb& getBoundingShape() const
	{
		return m_obb;
	}

	/// Get submesh info.
	void getSubMeshInfo(U32 subMeshId, U32& firstIndex, U32& indexCount, const Obb*& obb) const
	{
		const SubMesh& sm = m_subMeshes[subMeshId];
		firstIndex = sm.m_firstIndex;
		indexCount = sm.m_indexCount;
		obb = &sm.m_obb;
	}

	U32 getSubMeshCount() const
	{
		return m_subMeshes.getSize();
	}

	/// Get all info around vertex indices.
	void getIndexBufferInfo(BufferPtr& buff, PtrSize& buffOffset, U32& indexCount, IndexType& indexType) const
	{
		buff = m_indexBuff;
		buffOffset = 0;
		indexCount = m_indexCount;
		indexType = m_indexType;
	}

	/// Get the number of logical vertex buffers.
	U32 getVertexBufferCount() const
	{
		return m_vertBufferInfos.getSize();
	}

	/// Get vertex buffer info.
	void getVertexBufferInfo(const U32 buffIdx, BufferPtr& buff, PtrSize& offset, PtrSize& stride) const
	{
		buff = m_vertBuff;
		offset = m_vertBufferInfos[buffIdx].m_offset;
		stride = m_vertBufferInfos[buffIdx].m_stride;
	}

	/// Get attribute info. You need to check if the attribute is preset first (isVertexAttributePresent)
	void getVertexAttributeInfo(const VertexAttributeLocation attrib, U32& bufferIdx, Format& format,
								PtrSize& relativeOffset) const
	{
		ANKI_ASSERT(!!m_attribs[attrib].m_fmt);
		bufferIdx = m_attribs[attrib].m_buffIdx;
		format = m_attribs[attrib].m_fmt;
		relativeOffset = m_attribs[attrib].m_relativeOffset;
	}

	/// Check if a vertex attribute is present.
	Bool isVertexAttributePresent(const VertexAttributeLocation attrib) const
	{
		return !!m_attribs[attrib].m_fmt;
	}

	/// Get the number of texture coordinates channels.
	U32 getTextureChannelCount() const
	{
		return m_texChannelCount;
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
		return m_indexBuff;
	}

	/// Get the buffer that contains all the vertices of all submesses.
	BufferPtr getVertexBuffer() const
	{
		return m_vertBuff;
	}

private:
	class LoadTask;
	class LoadContext;

	static constexpr U VERTEX_BUFFER_ALIGNMENT = 64;

	/// Sub-mesh data
	class SubMesh
	{
	public:
		U32 m_firstIndex;
		U32 m_indexCount;
		Obb m_obb;
	};
	DynamicArray<SubMesh> m_subMeshes;

	// Index stuff
	U32 m_indexCount = 0;
	BufferPtr m_indexBuff;
	IndexType m_indexType = IndexType::COUNT;

	// Vertex stuff
	U32 m_vertCount = 0;

	class VertBuffInfo
	{
	public:
		U32 m_offset; ///< Offset from the base of m_vertBuff.
		U32 m_stride;
	};
	DynamicArray<VertBuffInfo> m_vertBufferInfos;

	class AttribInfo
	{
	public:
		Format m_fmt = Format::NONE;
		U32 m_relativeOffset = 0;
		U8 m_buffIdx = 0;
	};
	Array<AttribInfo, U(VertexAttributeLocation::COUNT)> m_attribs;

	BufferPtr m_vertBuff;
	U8 m_texChannelCount = 0;

	// Other
	Obb m_obb;

	// RT
	AccelerationStructurePtr m_blas;
	MeshGpuDescriptor m_meshGpuDescriptor;

	ANKI_USE_RESULT Error loadAsync(MeshLoader& loader) const;
};
/// @}

} // end namespace anki
