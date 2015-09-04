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
	Clusterer()
	{}

	~Clusterer()
	{
		m_clusters.destroy(m_alloc);
		m_splitInfo.destroy(m_alloc);
	}

	void init(const GenericMemoryPoolAllocator<U8>& alloc, U clusterCountX,
		U clusterCountY, U clusterCountZ)
	{
		m_alloc = alloc;
		m_counts[0] = clusterCountX;
		m_counts[1] = clusterCountY;
		m_counts[2] = clusterCountZ;
	}

	U getClusterCount() const
	{
		return U(m_counts[0]) * U(m_counts[1]) * U(m_counts[2]);
	}

	void prepare(const PerspectiveFrustum& fr);

	void initTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
		ClustererTestResult& rez) const;

	/// Bin collision shape.
	/// @param[in] cs The collision shape should be in view space.
	void bin(const CollisionShape& cs, ClustererTestResult& rez) const;

	void fillShaderParams(Vec4& params) const
	{
		params = Vec4(m_near, m_calcNearOpt, 0.0, 0.0);
	}

public:
	GenericMemoryPoolAllocator<U8> m_alloc;

	Array<U8, 3> m_counts;

	class Cluster
	{
	public:
		// Intead of Aabb use minimum size variables
		Vec3 m_min;
		Vec3 m_max;
	};

	/// [z][y][x]
	DArray<Cluster> m_clusters;

	class SplitInfo
	{
	public:
		Vec2 m_xy;
		Vec2 m_sizes;
	};

	DArray<SplitInfo> m_splitInfo;

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

	void findSplitsFromAabb(const Aabb& box, U& zFrom, U& zTo) const;
};
/// @}

} // end namespace anki


