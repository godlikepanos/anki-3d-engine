// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_PLANE_H
#define ANKI_COLLISION_PLANE_H

#include "anki/collision/CollisionShape.h"
#include "anki/Math.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Plane collision shape
class Plane: public CollisionShape
{
public:
	using Base = CollisionShape;

	/// @name Constructors
	/// @{

	/// Default constructor
	Plane()
		: CollisionShape(Type::PLANE)
	{}

	/// Copy constructor
	Plane(const Plane& b)
		: CollisionShape(Type::PLANE)
	{
		operator=(b);
	}

	/// Constructor
	Plane(const Vec4& normal, F32 offset);

	/// @see setFrom3Points
	Plane(const Vec3& p0, const Vec3& p1, const Vec3& p2)
		: CollisionShape(Type::PLANE)
	{
		setFrom3Points(p0, p1, p2);
	}

	/// @see setFromPlaneEquation
	Plane(F32 a, F32 b, F32 c, F32 d)
		: CollisionShape(Type::PLANE)
	{
		setFromPlaneEquation(a, b, c, d);
	}
	/// @}

	/// @name Operators
	/// @{
	Plane& operator=(const Plane& b)
	{
		Base::operator=(b);
		m_normal = b.m_normal;
		m_offset = b.m_offset;
		return *this;
	}
	/// @}

	/// @name Accessors
	/// @{
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
	/// @}

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

	/// It gives the distance between a point and a plane. if returns >0
	/// then the point lies in front of the plane, if <0 then it is behind
	/// and if =0 then it is co-planar
	F32 test(const Vec4& point) const
	{
		ANKI_ASSERT(isZero<F32>(point.w()));
		return m_normal.dot(point) - m_offset;
	}

	/// Get the distance from a point to this plane
	F32 getDistance(const Vec4& point) const
	{
		return fabs(test(point));
	}

	/// Returns the perpedicular point of a given point in this plane.
	/// Plane's normal and returned-point are perpedicular
	Vec4 getClosestPoint(const Vec4& point) const
	{
		return point - m_normal * test(point);
	}

	/// Test a CollisionShape
	template<typename T>
	F32 testShape(const T& x) const
	{
		return x.testPlane(*this, x);
	}

private:
	/// @name Data
	/// @{
	Vec4 m_normal;
	F32 m_offset;
	/// @}

	/// Set the plane from 3 points
	void setFrom3Points(const Vec3& p0, const Vec3& p1, const Vec3& p2);

	/// Set from plane equation is ax+by+cz+d
	void setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d);
};
/// @}

} // end namespace anki

#endif
