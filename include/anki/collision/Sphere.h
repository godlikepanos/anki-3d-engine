// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_SPHERE_H
#define ANKI_COLLISION_SPHERE_H

#include "anki/collision/ConvexShape.h"
#include "anki/Math.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Sphere collision shape
class Sphere: public ConvexShape
{
public:
	using Base = ConvexShape;

	/// @name Constructors
	/// @{

	/// Default constructor
	Sphere()
		: Base(Type::SPHERE)
	{}

	/// Copy constructor
	Sphere(const Sphere& b)
		: Base(Type::SPHERE)
	{
		operator=(b);
	}

	/// Constructor
	Sphere(const Vec4& center, F32 radius)
		:	Base(Type::SPHERE), 
			m_center(center), 
			m_radius(radius)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Vec4& getCenter() const
	{
		return m_center;
	}

	Vec4& getCenter()
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

	F32& getRadius()
	{
		return m_radius;
	}

	void setRadius(const F32 x)
	{
		m_radius = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Sphere& operator=(const Sphere& b)
	{
		Base::operator=(b);
		m_center = b.m_center;
		m_radius = b.m_radius;
		return *this;
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

	Sphere getTransformed(const Transform& transform) const;

	/// Get the sphere that includes this sphere and the given. See a
	/// drawing in the docs dir for more info about the algorithm
	Sphere getCompoundShape(const Sphere& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(
		const void* buff, U count, PtrSize stride, PtrSize buffSize);

	/// Implements CompoundShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const;

private:
	Vec4 m_center;
	F32 m_radius;
};
/// @}

} // end namespace anki

#endif
