// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/ConvexShape.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Sphere collision shape
class Sphere : public ConvexShape
{
public:
	using Base = ConvexShape;

	/// Default constructor
	Sphere()
		: Base(CollisionShapeType::SPHERE)
	{
	}

	/// Copy constructor
	Sphere(const Sphere& b)
		: Base(CollisionShapeType::SPHERE)
	{
		operator=(b);
	}

	/// Constructor
	Sphere(const Vec4& center, F32 radius)
		: Base(CollisionShapeType::SPHERE)
		, m_center(center)
		, m_radius(radius)
		, m_radiusSq(radius * radius)
	{
	}

	const Vec4& getCenter() const
	{
		return m_center;
	}

	void setCenter(const Vec4& x)
	{
		m_center = x;
	}

	F32 getRadius() const
	{
		return m_radius;
	}

	F32 getRadiusSquared() const
	{
		ANKI_ASSERT(m_radiusSq == m_radius * m_radius);
		return m_radiusSq;
	}

	void setRadius(const F32 x)
	{
		m_radius = x;
		m_radiusSq = x * x;
	}

	Sphere& operator=(const Sphere& b)
	{
		Base::operator=(b);
		m_center = b.m_center;
		m_radius = b.m_radius;
		m_radiusSq = b.m_radiusSq;
		return *this;
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

	Sphere getTransformed(const Transform& transform) const;

	/// Get the sphere that includes this sphere and the given. See a drawing in the docs dir for more info about the
	/// algorithm
	Sphere getCompoundShape(const Sphere& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(const void* buff, U count, PtrSize stride, PtrSize buffSize);

	/// Implements CompoundShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const;

	/// Intersect a ray with the sphere. It will not return the points that are not part of the ray.
	Bool intersectsRay(
		const Vec4& rayDir, const Vec4& rayOrigin, Array<Vec4, 2>& intersectionPoints, U& intersectionPointCount) const;

private:
	Vec4 m_center;
	F32 m_radius;
	F32 m_radiusSq;
};
/// @}

} // end namespace anki
