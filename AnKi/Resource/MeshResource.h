// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>
#include <AnKi/Gr.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <AnKi/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>

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
	MeshResource(CString fname, U32 uuid)
		: ResourceObject(fname, uuid)
	{
	}

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
	void getSubMeshInfo(U32 lod, U32 subMeshId, U32& firstIndex, U32& indexCount, U32& firstMeshlet, U32& meshletCount, Aabb& aabb) const
	{
		const SubMesh& sm = m_subMeshes[subMeshId];
		firstIndex = sm.m_firstIndices[lod];
		indexCount = sm.m_indexCounts[lod];
		firstMeshlet = sm.m_firstMeshlet[lod];
		meshletCount = sm.m_meshletCounts[lod];
		aabb = sm.m_aabb;
	}

	/// Get all info around vertex indices.
	void getIndexBufferInfo(U32 lod, PtrSize& buffOffset, U32& indexCount, IndexType& indexType) const
	{
		buffOffset = m_lods[lod].m_indexBufferAllocationToken.getOffset();
		ANKI_ASSERT(isAligned(getIndexSize(m_indexType), buffOffset));
		indexCount = m_lods[lod].m_indexCount;
		indexType = m_indexType;
	}

	/// Get vertex buffer info.
	void getVertexBufferInfo(U32 lod, VertexStreamId stream, PtrSize& ugbOffset, U32& vertexCount) const
	{
		ugbOffset = m_lods[lod].m_vertexBuffersAllocationToken[stream].getOffset();
		vertexCount = m_lods[lod].m_vertexCount;
	}

	void getMeshletBufferInfo(U32 lod, PtrSize& meshletBoundingVolumesUgbOffset, PtrSize& meshletGeometryDescriptorsUgbOffset, U32& meshletCount)
	{
		meshletBoundingVolumesUgbOffset = m_lods[lod].m_meshletBoundingVolumes.getOffset();
		meshletGeometryDescriptorsUgbOffset = m_lods[lod].m_meshletGeometryDescriptors.getOffset();
		ANKI_ASSERT(m_lods[lod].m_meshletCount);
		meshletCount = m_lods[lod].m_meshletCount;
	}

	const AccelerationStructurePtr& getBottomLevelAccelerationStructure(U32 lod, U32 subMeshId) const
	{
		ANKI_ASSERT(m_subMeshes[subMeshId].m_blas[lod]);
		return m_subMeshes[subMeshId].m_blas[lod];
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

	Error getOrCreateCollisionShape(Bool wantStatic, U32 lod, PhysicsCollisionShapePtr& out) const;

private:
	class LoadTask;
	class LoadContext;

	class Lod
	{
	public:
		UnifiedGeometryBufferAllocation m_indexBufferAllocationToken;
		Array<UnifiedGeometryBufferAllocation, U32(VertexStreamId::kMeshRelatedCount)> m_vertexBuffersAllocationToken;

		UnifiedGeometryBufferAllocation m_meshletIndices;
		UnifiedGeometryBufferAllocation m_meshletBoundingVolumes;
		UnifiedGeometryBufferAllocation m_meshletGeometryDescriptors;

		mutable Array<PhysicsCollisionShapePtr, 2> m_collisionShapes;
		mutable SpinLock m_collisionShapeMtx;

		U32 m_indexCount = 0;
		U32 m_vertexCount = 0;
		U32 m_meshletCount = 0;
	};

	class SubMesh
	{
	public:
		Array<U32, kMaxLodCount> m_firstIndices = {};
		Array<U32, kMaxLodCount> m_indexCounts = {};
		Array<U32, kMaxLodCount> m_firstMeshlet = {};
		Array<U32, kMaxLodCount> m_meshletCounts = {};

		Array<UnifiedGeometryBufferAllocation, kMaxLodCount> m_blasAllocationTokens;
		Array<AccelerationStructurePtr, kMaxLodCount> m_blas;

		Aabb m_aabb;
	};

	ResourceDynamicArray<SubMesh> m_subMeshes;
	ResourceDynamicArray<Lod> m_lods;
	Aabb m_aabb;
	IndexType m_indexType;
	VertexStreamMask m_presentVertStreams = VertexStreamMask::kNone;

	F32 m_positionsScale = 0.0f;
	Vec3 m_positionsTranslation = Vec3(0.0f);

	Bool m_isConvex = false;

	Error loadAsync(MeshBinaryLoader& loader) const;
};
/// @}

} // end namespace anki
