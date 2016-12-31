// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class PerspectiveFrustum;

/// @addtogroup renderer
/// @{

class ClustererDebugDrawer
{
public:
	virtual ~ClustererDebugDrawer() = default;

	virtual void operator()(const Vec3& lineA, const Vec3& lineB, const Vec3& color) = 0;
};

class alignas(alignof(U32)) Cluster
{
	friend class Clusterer;
	friend class ClustererTestResult;

public:
	U x() const
	{
		return m_v.m_x;
	}

	U y() const
	{
		return m_v.m_y;
	}

	U z() const
	{
		return m_v.m_z;
	}

private:
	class S
	{
	public:
		U8 m_x;
		U8 m_y;
		U8 m_z;
		U8 m_pad_;
	};

	union
	{
		S m_v;
		U32 m_u32;
	};
};

static_assert(sizeof(Cluster) == sizeof(U32), "Wrong size");

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

	DynamicArray<Cluster>::ConstIterator getClustersBegin() const
	{
		return m_clusterIds.getBegin();
	}

	DynamicArray<Cluster>::ConstIterator getClustersEnd() const
	{
		return m_clusterIds.getBegin() + m_count;
	}

	U getClusterCount() const
	{
		return m_count;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	DynamicArray<Cluster> m_clusterIds;
	U32 m_count = 0;

	void pushBack(U x, U y, U z)
	{
		ANKI_ASSERT(x <= 0xFF && y <= 0xFF && z <= 0xFF);
#if 1
		m_clusterIds[m_count].m_u32 = (z << 16) | (y << 8) | x;
		ANKI_ASSERT(m_clusterIds[m_count].x() == x && m_clusterIds[m_count].y() == y && m_clusterIds[m_count].z() == z);
#else
		m_clusterIds[m_count].m_v.m_x = U8(x);
		m_clusterIds[m_count].m_v.m_y = U8(y);
		m_clusterIds[m_count].m_v.m_z = U8(z);
#endif
		++m_count;
	}
};

/// Info that will prepare the clusterer.
class ClustererPrepareInfo
{
public:
	Mat4 m_viewMat;
	Mat4 m_projMat; ///< Must be perspective projection.
	Transform m_camTrf;
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
	void prepare(ThreadPool& threadpool, const ClustererPrepareInfo& inf);

	void initTestResults(const GenericMemoryPoolAllocator<U8>& alloc, ClustererTestResult& rez) const;

	/// Bin collision shape.
	void bin(const CollisionShape& cs, const Aabb& csBox, ClustererTestResult& rez) const;

	/// Bin a frustum.
	void binPerspectiveFrustum(const PerspectiveFrustum& fr, const Aabb& csBox, ClustererTestResult& rez) const;

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

	/// Call this after prepare()
	void debugDraw(ClustererDebugDrawer& drawer) const;

	/// Call this with a result.
	void debugDrawResult(const ClustererTestResult& rez, ClustererDebugDrawer& drawer) const;

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

	/// Cluster boxes in view space.
	DynamicArray<Aabb> m_clusterBoxes;

	Mat4 m_viewMat = Mat4::getIdentity();
	Mat4 m_projMat = Mat4::getIdentity();
	Transform m_camTrf = Transform::getIdentity();
	Vec4 m_unprojParams = Vec4(0.0);
	F32 m_near = 0.0;
	F32 m_far = 0.0;
	F32 m_calcNearOpt = 0.0;
	F32 m_shaderMagicVal = 0.0;

	F32 calcNear(U k) const;

	U calcZ(F32 zVspace) const;

	void setClusterBoxes(const Vec4& projParams, U begin, U end);

	void binGeneric(const CollisionShape& cs, const Aabb& box, ClustererTestResult& rez) const;
	void binGenericRecursive(
		const CollisionShape& cs, U xBegin, U xEnd, U yBegin, U yEnd, U zBegin, U zEnd, ClustererTestResult& rez) const;

	/// Special fast path for binning spheres.
	void binSphere(const Sphere& s, const Aabb& aabb, ClustererTestResult& rez) const;

	/// Quick reduction.
	void quickReduction(const Aabb& aabb, const Mat4& mvp, U& xBegin, U& xEnd, U& yBegin, U& yEnd) const;

	/// Box based reduction.
	template<typename TFunc>
	void boxReduction(U xBegin, U xEnd, U yBegin, U yEnd, U zBegin, U zEnd, ClustererTestResult& rez, TFunc func) const;

	void computeSplitRange(const CollisionShape& cs, U& zBegin, U& zEnd) const;

	void update(U32 threadId, PtrSize threadsCount, Bool frustumChanged);

	/// Calculate and set a top looking plane.
	void calcPlaneY(U i, const Vec4& projParams);

	/// Calculate and set a right looking plane.
	void calcPlaneX(U j, const Vec4& projParams);

	/// Call this when a shape is visible by all tiles.
	void totallyInsideAllTiles(U zBegin, U zEnd, ClustererTestResult& rez) const;
};
/// @}

} // end namespace anki
