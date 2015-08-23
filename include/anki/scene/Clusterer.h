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

class FrustumComponent;

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
	static const U TILE_SIZE = 64;

	Clusterer(const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{}

	~Clusterer()
	{
		m_clusters.destroy(m_alloc);
	}

	void init(U tileCountX, U tileCountY);

	void initTempTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
		ClustererTestResult& rez) const;

	void prepare(FrustumComponent* frc, const SArray<Vec2>& minMaxTileDepth);

private:
	class Cluster
	{
	public:
		Aabb m_box;
	};

	GenericMemoryPoolAllocator<U8> m_alloc;

	Array<U8, 3> m_counts;

	/// [z][y][x]
	DArray<Cluster> m_clusters;

	FrustumComponent* m_frc = nullptr;
	Timestamp m_frcTimestamp = 0;

	Cluster& cluster(U x, U y, U z)
	{
		return m_clusters[m_counts[0] * (z * m_counts[1] + y) + x];
	}
};
/// @}

} // end namespace anki
