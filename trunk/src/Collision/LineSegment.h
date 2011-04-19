#ifndef LINE_SEGMENT_H
#define LINE_SEGMENT_H

#include "CollisionShape.h"
#include "Math.h"


class LineSegment: public CollisionShape
{
	public:
		/// @name Constructors
		/// @{
		LineSegment(): CollisionShape(CST_LINE_SEG) {}
		LineSegment(const Vec3& origin, const Vec3& direction);
		LineSegment(const LineSegment& b);
		/// @}

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec3, origin, getOrigin, setOrigin)
		GETTER_SETTER(Vec3, dir, getDirection, setDirection)
		/// @}

		LineSegment getTransformed(const Transform& transform) const;

		/// Implements CollisionShape::testPlane @see CollisionShape::testPlane
		float testPlane(const Plane& plane) const;

	private:
		/// @name Data
		/// @{
		Vec3 origin; ///< P0
		Vec3 dir; ///< P1 = origin+dir so dir = P1-origin
		/// @}
};


inline LineSegment::LineSegment(const Vec3& origin_, const Vec3& direction):
	CollisionShape(CST_LINE_SEG),
	origin(origin_),
	dir(direction)
{}


inline LineSegment::LineSegment(const LineSegment& b):
	CollisionShape(CST_LINE_SEG),
	origin(b.origin),
	dir(b.dir)
{}


#endif
