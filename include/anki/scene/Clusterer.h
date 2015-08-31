// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/Common.h"
#include "anki/Math.h"
#include "anki/collision/Aabb.h"
#include "anki/core/Timestamp.h"

namespace anki {

class PerspectiveFrustum;

/// @addtogroup scene
/// @{

/// The result of the cluster tests.
class ClustererTestResult
{
	friend class Clusterer;

public:
	ClustererTestResult()
	{}

	~ClustererTestResult()
	{
		m_clusterIds.destroy(m_alloc);
	}

	DArray<Array<U8, 3>>::ConstIterator getClustersBegin() const
	{
		return m_clusterIds.getBegin();
	}

	DArray<Array<U8, 3>>::ConstIterator getClustersEnd() const
	{
		return m_clusterIds.getBegin() + m_count;
	}

private:
	DArray<Array<U8, 3>> m_clusterIds;
	U32 m_count = 0;
	GenericMemoryPoolAllocator<U8> m_alloc;
};

/// Collection of clusters for visibility tests.
class Clusterer
{
public:
	static constexpr U TILE_SIZE = 64;
	static constexpr F32 NEAR_BIAS = 2.0;

	Clusterer(const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{}

	~Clusterer()
	{
		m_clusters.destroy(m_alloc);
	}

	void init(U clusterCountX, U clusterCountY, U clusterCountZ)
	{
		m_counts[0] = clusterCountX;
		m_counts[1] = clusterCountY;
		m_counts[2] = clusterCountZ;
	}

	void prepare(
		const PerspectiveFrustum& fr, const SArray<Vec2>& minMaxTileDepth);

	void initTempTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
		ClustererTestResult& rez) const;

	/// Bin collision shape.
	/// @param[in] cs The collision shape should be in view space.
	void bin(const CollisionShape& cs, ClustererTestResult& rez) const;

public:
	class Cluster
	{
	public:
		// Intead of Aabb use minimum size variables
		Vec3 m_min;
		Vec3 m_max;
	};

	GenericMemoryPoolAllocator<U8> m_alloc;

	Array<U8, 3> m_counts;

	/// [z][y][x]
	DArray<Cluster> m_clusters;

	F32 m_near = 0.0;
	F32 m_far = 0.0;
	F32 m_fovY = 0.0;
	F32 m_fovX = 0.0;

	F32 m_calcNearOpt = 0.0;
	Mat4 m_projMat;

	Cluster& cluster(U x, U y, U z)
	{
		ANKI_ASSERT(x < m_counts[0]);
		ANKI_ASSERT(y < m_counts[1]);
		ANKI_ASSERT(z < m_counts[2]);
		return m_clusters[m_counts[0] * (z * m_counts[1] + y) + x];
	}

	const Cluster& cluster(U x, U y, U z) const
	{
		ANKI_ASSERT(x < m_counts[0]);
		ANKI_ASSERT(y < m_counts[1]);
		ANKI_ASSERT(z < m_counts[2]);
		return m_clusters[m_counts[0] * (z * m_counts[1] + y) + x];
	}

	F32 calcNear(U k) const;
	U calcK(F32 zVspace) const;

	void initClusters();

	void binSphere(const Sphere& s, ClustererTestResult& rez) const;

	void binGeneric(const CollisionShape& cs, ClustererTestResult& rez) const;

	void findTilesFromAabb(const Aabb& box, U& tileBeginX, U& tileBeginY,
		U& tileEndX, U& tileEndY) const;

	void findSplitsFromAabb(const Aabb& box, U& zFrom, U& zTo) const;
};
/// @}

} // end namespace anki


