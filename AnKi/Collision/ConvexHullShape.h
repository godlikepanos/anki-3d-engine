// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Collision/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup collision
/// @{

/// Convex hull collision shape
class ConvexHullShape
{
public:
	static constexpr CollisionShapeType kClassType = CollisionShapeType::kConvexHull;

	/// Will not initialize any memory, nothing.
	ConvexHullShape()
	{
	}

	ConvexHullShape(const ConvexHullShape& other)
	{
		operator=(other);
	}

	/// In initializes the storage that holds the point cloud using a predefined buffer. The convex hull is not the
	/// owner of the storage.
	/// @param points The base of the storage buffer. Size should be @a pointCount * sizeof(Vec4)
	/// @param pointCount The number of points
	ConvexHullShape(const Vec4* points, U32 pointCount)
		: m_trf(Transform::getIdentity())
		, m_invTrf(Transform::getIdentity())
		, m_points(points)
		, m_pointCount(pointCount)
		, m_trfIdentity(true)
	{
		check();
	}

	ConvexHullShape& operator=(const ConvexHullShape& b)
	{
		b.check();
		m_trf = b.m_trf;
		m_invTrf = b.m_invTrf;
		m_points = b.m_points;
		m_pointCount = b.m_pointCount;
		m_trfIdentity = b.m_trfIdentity;
		return *this;
	}

	/// Get points in local space.
	ConstWeakArray<Vec4> getPoints() const
	{
		check();
		return ConstWeakArray<Vec4>(m_points, m_pointCount);
	}

	/// Get current transform.
	const Transform& getTransform() const
	{
		check();
		return m_trf;
	}

	const Transform& getInvertTransform() const
	{
		check();
		return m_invTrf;
	}

	Bool isTransformIdentity() const
	{
		return m_trfIdentity;
	}

	void setTransform(const Transform& trf);

	/// Get a transformed.
	[[nodiscard]] ConvexHullShape getTransformed(const Transform& trf) const;

	/// Compute the GJK support.
	[[nodiscard]] Vec4 computeSupport(const Vec4& dir) const;

private:
	Transform m_trf;
	Transform m_invTrf;

	const Vec4* m_points
#if ANKI_ENABLE_ASSERTIONS
		= nullptr
#endif
		;

	U32 m_pointCount
#if ANKI_ENABLE_ASSERTIONS
		= 0
#endif
		;

	Bool m_trfIdentity; ///< Optimization.

	void check() const
	{
		ANKI_ASSERT(m_points && m_pointCount > 0);
	}
};
/// @}

} // end namespace anki
