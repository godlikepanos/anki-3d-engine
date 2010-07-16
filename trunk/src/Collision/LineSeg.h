#ifndef LINESEG_H
#define LINESEG_H

#include "Common.h"
#include "Math.h"


class Plane;
class Ray;
class Sphere;
class Aabb;
class Obb;


/**
 * Line segment
 */
class LineSeg: public CollisionShape
{
	PROPERTY_RW(Vec3, origin, setOrigin, getOrigin) ///< P0
	PROPERTY_RW(Vec3, dir, setDir, getDir) ///< P1 = origin+dir so dir = P1-origin

	public:
		// constructors & distructors
		LineSeg() {}
		LineSeg(const LineSeg& b);
		LineSeg(const Vec3& origin_, const Vec3& dir_);

		// std funcs
		LineSeg getTransformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void render();
		float planeTest(const Plane& plane) const;
		bool intersects(const Sphere& sphere) const;
		bool intersects(const Aabb& aabb) const;
		bool intersects(const Obb& obb) const;

		// other funcs
		float getLengthSquared() const;
		float getLength() const;
		/**
		 * @name Distance funcs
		 *
		 * 1) If tc<0 then outside the line segment and close to origin. If tc>1 again outside the line segment and closer
		 *    to dir. If >0 or <1 then inside the segment
		 * 2) When we talk about distances we calc the distance between the point|line|ray etc and the P0 OR P1. For example
		 *    the dist between a point and P0 or P1 depending of tc
		 */
		/**@{*/
		float getDistanceSquared(const LineSeg& seg, float& sc, float& tc) const;   ///< Dist with another segment
		float getDistanceSquared(const Ray& ray, float& sc, float& tc) const;       ///< Dist with a ray
		float getDistanceSquared(const Vec3& point, float& tc) const;                ///< Dist with a point.
		/**@}*/
		void calcClosestPoints(const LineSeg& seg, Vec3& point0, Vec3& point1) const; ///< Between this and another LineSeg
		void calcClosestPoints(const Ray& ray, Vec3& point0, Vec3& point1) const;     ///< Between this and a Ray
		Vec3 calcClosestPoints(const Vec3& point) const;                              ///< Between this and a point
};


inline LineSeg::LineSeg(const LineSeg& b):
	origin(b.origin),
	dir(b.dir)
{}


inline LineSeg::LineSeg(const Vec3& origin_, const Vec3& dir_):
	origin(origin_),
	dir(dir_)
{}


inline float LineSeg::getLengthSquared() const
{
	return dir.getLengthSquared();
}


inline float LineSeg::getLength() const
{
	return dir.getLength();
}

#endif
