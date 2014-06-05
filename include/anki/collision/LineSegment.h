// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_LINE_SEGMENT_H
#define ANKI_COLLISION_LINE_SEGMENT_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Vec3.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Line segment. Line from a point to a point. P0 = origin and
/// P1 = direction + origin
class LineSegment: public CollisionShape
{
public:
	/// @name Constructors
	/// @{
	LineSegment()
		: CollisionShape(Type::LINE_SEG)
	{}

	LineSegment(const Vec3& origin_, const Vec3& direction)
		:	CollisionShape(Type::LINE_SEG), 
			origin(origin_), 
			dir(direction)
	{}

	LineSegment(const LineSegment& b)
		:	CollisionShape(Type::LINE_SEG), 
			origin(b.origin), 
			dir(b.dir)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getOrigin() const
	{
		return origin;
	}
	Vec3& getOrigin()
	{
		return origin;
	}
	void setOrigin(const Vec3& x)
	{
		origin = x;
	}

	const Vec3& getDirection() const
	{
		return dir;
	}
	Vec3& getDirection()
	{
		return dir;
	}
	void setDirection(const Vec3& x)
	{
		dir = x;
	}
	/// @}

	/// @name Operators
	/// @{
	LineSegment& operator=(const LineSegment& b)
	{
		origin = b.origin;
		dir = b.dir;
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

	LineSegment getTransformed(const Transform& transform) const;

private:
	/// @name Data
	/// @{
	Vec3 origin; ///< P0
	Vec3 dir; ///< P1 = origin+dir so dir = P1-origin
	/// @}
};
/// @}

} // end namespace

#endif
