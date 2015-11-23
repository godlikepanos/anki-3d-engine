// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/Math.h>
#include <anki/collision/Aabb.h>
#include <anki/core/Timestamp.h>

namespace anki {

class FrustumComponent;
class SceneNode;

/// @addtogroup renderer
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

	void pushBack(U x, U y, U z)
	{
		ANKI_ASSERT(x <= 0xFF && y <= 0xFF && z <= 0xFF);
		m_clusterIds[m_count++] = Array<U8, 3>{U8(x), U8(y), U8(z)};
	}
};

/// Collection of clusters for visibility tests.
class Clusterer
{
	friend class UpdatePlanesPerspectiveCameraTask;

public:
	Clusterer()
	{}

	~Clusterer();

	void init(const GenericMemoryPoolAllocator<U8>& alloc, U clusterCountX,
		U clusterCountY, U clusterCountZ);

	/// Prepare for visibility tests.
	void prepare(ThreadPool& threadpool, const FrustumComponent& frc);

	void initTestResults(const GenericMemoryPoolAllocator<U8>& alloc,
		ClustererTestResult& rez) const;

	/// Bin collision shape.
	void bin(const CollisionShape& cs, const Aabb& csBox,
		ClustererTestResult& rez) const;

	F32 getDivisor() const
	{
		return m_calcNearOpt;
	}

	U getClusterCount() const
	{
		return m_counts[0] * m_counts[1] * m_counts[2];
	}

public:
	GenericMemoryPoolAllocator<U8> m_alloc;

	Array<U8, 3> m_counts;

	/// Tile planes.
	DArray<Plane> m_allPlanes; ///< Do one allocation.
	SArray<Plane> m_planesY; ///< Local space.
	SArray<Plane> m_planesX; ///< Local space.
	SArray<Plane> m_planesYW;
	SArray<Plane> m_planesXW;
	Plane* m_nearPlane; ///< In world space
	Plane* m_farPlane; ///< In world space

	/// Used to check if the frustum is changed and we need to update the
	/// planes.
	const SceneNode* m_node = nullptr;

	const FrustumComponent* m_frc = nullptr; ///< Cache it.

	/// Timestamp for the same reason as m_frc.
	Timestamp m_planesLSpaceTimestamp = 0;

	F32 m_near = 0.0;
	F32 m_far = 0.0;
	F32 m_calcNearOpt = 0.0;

	F32 calcNear(U k) const;

	U calcZ(F32 zVspace) const;

	void binGeneric(const CollisionShape& cs, U xBegin, U xEnd, U yBegin,
		U yEnd, U zBegin, U zEnd, ClustererTestResult& rez) const;

	/// Special fast path for binning spheres.
	void binSphere(const Sphere& s, const Aabb& aabb,
		ClustererTestResult& rez) const;

	void computeSplitRange(const CollisionShape& cs, U& zBegin, U& zEnd) const;

	void update(U32 threadId, PtrSize threadsCount, Bool frustumChanged);

	/// Calculate and set a top looking plane.
	void calcPlaneY(U i, const Vec4& projParams);

	/// Calculate and set a right looking plane.
	void calcPlaneX(U j, const Vec4& projParams);
};
/// @}

} // end namespace anki


