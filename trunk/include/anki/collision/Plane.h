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
	/// @name Constructors
	/// @{

	/// Default constructor
	Plane()
		: CollisionShape(Type::PLANE)
	{}

	/// Copy constructor
	Plane(const Plane& b);

	/// Constructor
	Plane(const Vec3& normal_, F32 offset_);

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

	/// @name Accessors
	/// @{
	const Vec3& getNormal() const
	{
		return normal;
	}
	Vec3& getNormal()
	{
		return normal;
	}
	void setNormal(const Vec3& x)
	{
		normal = x;
	}

	F32 getOffset() const
	{
		return offset;
	}
	F32& getOffset()
	{
		return offset;
	}
	void setOffset(const F32 x)
	{
		offset = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Plane& operator=(const Plane& b)
	{
		normal = b.normal;
		offset = b.offset;
		return *this;
	}
	/// @}

	/// Check for collision
	template<typename T>
	Bool collide(const T& x) const
	{
		return detail::collide(*this, x);
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

	/// It gives the distance between a point and a plane. if returns >0
	/// then the point lies in front of the plane, if <0 then it is behind
	/// and if =0 then it is co-planar
	F32 test(const Vec3& point) const
	{
		return normal.dot(point) - offset;
	}

	/// Get the distance from a point to this plane
	F32 getDistance(const Vec3& point) const
	{
		return fabs(test(point));
	}

	/// Returns the perpedicular point of a given point in this plane.
	/// Plane's normal and returned-point are perpedicular
	Vec3 getClosestPoint(const Vec3& point) const
	{
		return point - normal * test(point);
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
	Vec3 normal;
	F32 offset;
	/// @}

	/// Set the plane from 3 points
	void setFrom3Points(const Vec3& p0, const Vec3& p1, const Vec3& p2);

	/// Set from plane equation is ax+by+cz+d
	void setFromPlaneEquation(F32 a, F32 b, F32 c, F32 d);
};
/// @}

} // end namespace

#endif
