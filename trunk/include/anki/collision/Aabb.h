// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_AABB_H
#define ANKI_COLLISION_AABB_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Vec3.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Axis align bounding box collision shape
class Aabb: public CollisionShape
{
public:
	/// @name Constructors
	/// @{
	Aabb()
		: CollisionShape(Type::AABB)
	{}

	Aabb(const Vec3& min, const Vec3& max)
		: CollisionShape(Type::AABB), m_min(min), m_max(max)
	{
		ANKI_ASSERT(m_min < m_max);
	}

	Aabb(const Aabb& b)
		:	CollisionShape(Type::AABB), 
			m_min(b.m_min), 
			m_max(b.m_max)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getMin() const
	{
		return m_min;
	}
	Vec3& getMin()
	{
		return m_min;
	}
	void setMin(const Vec3& x)
	{
		m_min = x;
	}

	const Vec3& getMax() const
	{
		return m_max;
	}
	Vec3& getMax()
	{
		return m_max;
	}
	void setMax(const Vec3& x)
	{
		m_max = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Aabb& operator=(const Aabb& b)
	{
		m_min = b.m_min;
		m_max = b.m_max;
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
	void computeAabb(Aabb& b) const
	{
		b = *this;
	}

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
	Vec3 m_min;
	Vec3 m_max;
	/// @}
};
/// @}

} // end namespace anki

#endif
