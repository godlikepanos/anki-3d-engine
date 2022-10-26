// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>
#include <AnKi/Gr.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Shaders/Include/MeshTypes.h>

namespace anki {

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

	/// Load from a mesh file
	Error load(const ResourceFilename& filename, Bool async);

	/// Get the complete bounding box.
	const Aabb& getBoundingShape() const
	{
		return m_aabb;
	}

	U32 getSubMeshCount() const
	{
		return m_subMeshes.getSize();
	}

	/// Get submesh info.
	void getSubMeshInfo(U32 lod, U32 subMeshId, U32& firstIndex, U32& indexCount, Aabb& aabb) const
	{
		const SubMesh& sm = m_subMeshes[subMeshId];
		firstIndex = sm.m_firstIndices[lod];
		indexCount = sm.m_indexCounts[lod];
		aabb = sm.m_aabb;
	}

	/// Get all info around vertex indices.
	void getIndexBufferInfo(U32 lod, PtrSize& buffOffset, U32& indexCount, IndexType& indexType) const
	{
		buffOffset = m_lods[lod].m_unifiedGeometryIndexBufferOffset;
		ANKI_ASSERT(isAligned(getIndexSize(m_indexType), buffOffset));
		indexCount = m_lods[lod].m_indexCount;
		indexType = m_indexType;
	}

	/// Get vertex buffer info.
	void getVertexStreamInfo(U32 lod, VertexStreamId stream, PtrSize& bufferOffset, U32& vertexCount) const
	{
		bufferOffset = m_lods[lod].m_unifiedGeometryVertBufferOffsets[stream];
		vertexCount = m_lods[lod].m_vertexCount;
	}

	const AccelerationStructurePtr& getBottomLevelAccelerationStructure(U32 lod) const
	{
		ANKI_ASSERT(m_lods[lod].m_blas);
		return m_lods[lod].m_blas;
	}

	/// Check if a vertex stream is present.
	Bool isVertexStreamPresent(const VertexStreamId stream) const
	{
		return !!(m_presentVertStreams & VertexStreamMask(1 << stream));
	}

	U32 getLodCount() const
	{
		return m_lods.getSize();
	}

	F32 getPositionsScale() const
	{
		return m_positionsScale;
	}

	Vec3 getPositionsTranslation() const
	{
		return m_positionsTranslation;
	}

private:
	class LoadTask;
	class LoadContext;

	class Lod
	{
	public:
		PtrSize m_unifiedGeometryIndexBufferOffset = kMaxPtrSize;
		Array<PtrSize, U32(VertexStreamId::kMeshRelatedCount)> m_unifiedGeometryVertBufferOffsets;

		U32 m_indexCount = 0;
		U32 m_vertexCount = 0;

		AccelerationStructurePtr m_blas;

		Lod()
		{
			m_unifiedGeometryVertBufferOffsets.fill(m_unifiedGeometryVertBufferOffsets.getBegin(),
													m_unifiedGeometryVertBufferOffsets.getEnd(), kMaxPtrSize);
		}
	};

	class SubMesh
	{
	public:
		Array<U32, kMaxLodCount> m_firstIndices;
		Array<U32, kMaxLodCount> m_indexCounts;
		Aabb m_aabb;
	};

	DynamicArray<SubMesh> m_subMeshes;
	DynamicArray<Lod> m_lods;
	Aabb m_aabb;
	IndexType m_indexType;
	VertexStreamMask m_presentVertStreams = VertexStreamMask::kNone;

	F32 m_positionsScale = 0.0f;
	Vec3 m_positionsTranslation = Vec3(0.0f);

	Error loadAsync(MeshBinaryLoader& loader) const;
};
/// @}

} // end namespace anki
