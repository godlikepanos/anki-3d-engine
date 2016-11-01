// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/Math.h>
#include <anki/collision/Aabb.h>
#include <anki/core/Timestamp.h>

namespace anki
{

// Forward
class FrustumComponent;
class SceneNode;
class ThreadPool;

/// @addtogroup renderer
/// @{

/// The result of the cluster tests.
class ClustererTestResult
{
	friend class Clusterer;

public:
	ClustererTestResult()
	{
	}

	~ClustererTestResult()
	{
		m_clusterIds.destroy(m_alloc);
	}

	DynamicArray<Array<U8, 3>>::ConstIterator getClustersBegin() const
	{
		return m_clusterIds.getBegin();
	}

	DynamicArray<Array<U8, 3>>::ConstIterator getClustersEnd() const
	{
		return m_clusterIds.getBegin() + m_count;
	}

	U getClusterCount() const
	{
		return m_count;
	}

private:
	DynamicArray<Array<U8, 3>> m_clusterIds;
	U32 m_count = 0;
	GenericMemoryPoolAllocator<U8> m_alloc;

	void pushBack(U x, U y, U z)
	{
		ANKI_ASSERT(x <= 0xFF && y <= 0xFF && z <= 0xFF);
		m_clusterIds[m_count][0] = U8(x);
		m_clusterIds[m_count][1] = U8(y);
		m_clusterIds[m_count][2] = U8(z);
		++m_count;
	}
};

/// Collection of clusters for visibility tests.
class Clusterer
{
	friend class UpdatePlanesPerspectiveCameraTask;

public:
	Clusterer()
	{
	}

	~Clusterer();

	void init(const GenericMemoryPoolAllocator<U8>& alloc, U clusterCountX, U clusterCountY, U clusterCountZ);

	/// Prepare for visibility tests.
	void prepare(ThreadPool& threadpool, const FrustumComponent& frc);

	void initTestResults(const GenericMemoryPoolAllocator<U8>& alloc, ClustererTestResult& rez) const;

	/// Bin collision shape.
	void bin(const CollisionShape& cs, const Aabb& csBox, ClustererTestResult& rez) const;

	/// A value that will be used in shaders to calculate the cluster index.
	F32 getShaderMagicValue() const
	{
		return m_shaderMagicVal;
	}

	U getClusterCountX() const
	{
		return m_counts[0];
	}

	U getClusterCountY() const
	{
		return m_counts[1];
	}

	U getClusterCountZ() const
	{
		return m_counts[2];
	}

	U getClusterCount() const
	{
		return m_counts[0] * m_counts[1] * m_counts[2];
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;

	Array<U8, 3> m_counts;

	/// Tile planes.
	DynamicArray<Plane> m_allPlanes; ///< Do one allocation.
	WeakArray<Plane> m_planesY; ///< Local space.
	WeakArray<Plane> m_planesX; ///< Local space.
	WeakArray<Plane> m_planesYW;
	WeakArray<Plane> m_planesXW;
	Plane* m_nearPlane; ///< In world space
	Plane* m_farPlane; ///< In world space

	/// Used to check if the frustum is changed and we need to update the planes.
	const SceneNode* m_node = nullptr;

	const FrustumComponent* m_frc = nullptr; ///< Cache it.

	/// Timestamp for the same reason as m_frc.
	Timestamp m_planesLSpaceTimestamp = 0;

	F32 m_near = 0.0;
	F32 m_far = 0.0;
	F32 m_calcNearOpt = 0.0;
	F32 m_shaderMagicVal = 0.0;

	Array<Vec4, 8> m_disk; ///< To bin a sphere in tiles.

	void initDisk();

	F32 calcNear(U k) const;

	U calcZ(F32 zVspace) const;

	void binGeneric(
		const CollisionShape& cs, U xBegin, U xEnd, U yBegin, U yEnd, U zBegin, U zEnd, ClustererTestResult& rez) const;

	/// Special fast path for binning spheres.
	void binSphere(const Sphere& s, const Aabb& aabb, ClustererTestResult& rez) const;

	void computeSplitRange(const CollisionShape& cs, U& zBegin, U& zEnd) const;

	void update(U32 threadId, PtrSize threadsCount, Bool frustumChanged);

	/// Calculate and set a top looking plane.
	void calcPlaneY(U i, const Vec4& projParams);

	/// Calculate and set a right looking plane.
	void calcPlaneX(U j, const Vec4& projParams);

	/// Call this when a shape is visible by all tiles.
	void totallyInsideAllTiles(U zBegin, U zEnd, ClustererTestResult& rez) const;

	void createConverHull();
};
/// @}

} // end namespace anki
