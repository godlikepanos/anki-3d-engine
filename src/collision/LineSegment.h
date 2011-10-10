#ifndef LINE_SEGMENT_H
#define LINE_SEGMENT_H

#include "CollisionShape.h"
#include "math/Vec3.h"


/// @addtogroup Collision
/// @{

/// Line segment. Line from a point to a point
class LineSegment: public CollisionShape
{
	public:
		/// @name Constructors
		/// @{
		LineSegment()
		:	CollisionShape(CST_LINE_SEG)
		{}
		LineSegment(const Vec3& origin, const Vec3& direction);
		LineSegment(const LineSegment& b);
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
/// @}


inline LineSegment::LineSegment(const Vec3& origin_, const Vec3& direction)
:	CollisionShape(CST_LINE_SEG),
	origin(origin_),
	dir(direction)
{}


inline LineSegment::LineSegment(const LineSegment& b)
:	CollisionShape(CST_LINE_SEG),
	origin(b.origin),
	dir(b.dir)
{}


#endif
