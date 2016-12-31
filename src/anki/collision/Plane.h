// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/CollisionShape.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Plane collision shape
class Plane : public CollisionShape
{
public:
	using Base = CollisionShape;

	/// Default constructor
	Plane()
		: CollisionShape(CollisionShapeType::PLANE)
	{
	}

	/// Copy constructor
	Plane(const Plane& b)
		: CollisionShape(CollisionShapeType::PLANE)
	{
		operator=(b);
	}

	/// Constructor
	Plane(const Vec4& normal, F32 offset)
		: CollisionShape(CollisionShapeType::PLANE)
		, m_normal(normal)
		, m_offset(offset)
	{
	}

	/// @see setFrom3Points
	Plane(const Vec4& p0, const Vec4& p1, const Vec4& p2)
		: CollisionShape(CollisionShapeType::PLANE)
	{
		setFrom3Points(p0, p1, p2);
	}

	/// @see setFromPlaneEquation
	Plane(F32 a, F32 b, F32 c, F32 d)
		: CollisionShape(CollisionShapeType::PLANE)
	{
		setFromPlaneEquation(a, b, c, d);
	}

	Plane& operator=(const Plane& b)
	{
		Base::operator=(b);
		m_normal = b.m_normal;
		m_offset = b.m_offset;
		return *this;
	}

	const Vec4& getNormal() const
	{
		return m_normal;
	}
	Vec4& getNormal()
	{
		return m_normal;
	}
	void setNormal(const Vec4& x)
	{
		m_normal = x;
	}

	F32 getOffset() const
	{
		return m_offset;
	}
	F32& getOffset()
	{
		return m_offset;
	}
	void setOffset(const F32 x)
	{
		m_offset = x;
	}

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v)
	{
		v.visit(*this);
	}
	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& b) const;

	/// Return the transformed
	Plane getTransformed(const Transform& trf) const;

	/// It gives the distance between a point and a plane. It returns >0 if the point lies in front of the plane, <0
	/// if it's behind and ==0 when it's co-planar.
	F32 test(const Vec4& point) const
	{
		ANKI_ASSERT(isZero<F32>(point.w()));
		return m_normal.dot(point) - m_offset;
	}

	/// Get the distance from a point to this plane
	F32 getDistance(const Vec4& point) const
	{
		return absolute(test(point));
	}

	/// Returns the perpedicular point of a given point in this plane. Plane's normal and returned-point are
	/// perpedicular
	Vec4 getClosestPoint(const Vec4& point) const
	{
		return point - m_normal * test(point);
	}

	/// Find intersection with a vector.
	Bool intersectVector(const Vec4& p, Vec4& intersection) const;

	/// Find the intersection point of this plane and a ray. If the ray hits the back of the plane then there is no
	/// intersection.
	Bool intersectRay(const Vec4& rayOrigin, const Vec4& rayDir, Vec4& intersection) const;

	/// Test a CollisionShape
	template<typename T>
	F32 testShape(const T& x) const
	{
		return x.testPlane(*this, x);
	}

	/// Set the plane from 3 points
	void setFrom3Points(const Vec4& p0, const Vec4& p1, const Vec4& p2);

private:
	/// @name Data
	/// @{
	Vec4 m_normal;
	F32 m_offset;
	/// @}

	/// Set from plane equation is ax+by+cz+d
	void setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d);
};
/// @}

} // end namespace anki
