// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_AABB_H
#define ANKI_COLLISION_AABB_H

#include "anki/collision/ConvexShape.h"
#include "anki/math/Vec3.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Axis align bounding box collision shape
class Aabb: public ConvexShape
{
public:
	using Base = ConvexShape;

	/// @name Constructors
	/// @{
	Aabb()
		: Base(Type::AABB)
	{}

	Aabb(const Vec4& min, const Vec4& max)
		: Base(Type::AABB), m_min(min), m_max(max)
	{
		ANKI_ASSERT(m_min.xyz() < m_max.xyz());
	}

	Aabb(const Aabb& b)
		: Base(Type::AABB)
	{
		operator=(b);
	}
	/// @}

	/// @name Accessors
	/// @{
	const Vec4& getMin() const
	{
		return m_min;
	}
	Vec4& getMin()
	{
		return m_min;
	}
	void setMin(const Vec4& x)
	{
		m_min = x;
	}

	const Vec4& getMax() const
	{
		return m_max;
	}
	Vec4& getMax()
	{
		return m_max;
	}
	void setMax(const Vec4& x)
	{
		m_max = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Aabb& operator=(const Aabb& b)
	{
		Base::operator=(b);
		m_min = b.m_min;
		m_max = b.m_max;
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
	void computeAabb(Aabb& b) const
	{
		b = *this;
	}

	/// Implements CompoundShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const;

	/// It uses a nice trick to avoid unwanted calculations 
	Aabb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not
	/// very accurate
	Aabb getCompoundShape(const Aabb& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(
		const void* buff, U count, PtrSize stride, PtrSize buffSize);

private:
	/// @name Data
	/// @{
	Vec4 m_min;
	Vec4 m_max;
	/// @}
};
/// @}

} // end namespace anki

#endif
