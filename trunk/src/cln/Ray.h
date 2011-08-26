#ifndef CLN_RAY_H
#define CLN_RAY_H

#include "CollisionShape.h"
#include "m/Math.h"
#include "util/Accessors.h"


namespace cln {


class Plane;


/// Ray collision shape. It has a starting point but the end extends to infinity
class Ray: public CollisionShape
{
	public:
		/// Default constructor
		Ray(): CollisionShape(CST_RAY) {}

		/// Copy constructor
		Ray(const Ray& other);

		/// Constructor
		Ray(const Vec3& origin, const Vec3& direction);

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec3, origin, getOrigin, setOrigin)
		GETTER_SETTER(Vec3, dir, getDirection, setDirection)
		/// @}

		Ray getTransformed(const Transform& transform) const;

		/// Implements CollisionShape::testPlane
		/// @see CollisionShape::testPlane
		float testPlane(const Plane& plane) const;

	private:
		Vec3 origin;
		Vec3 dir;
};


inline Ray::Ray(const Ray& other)
:	CollisionShape(CST_RAY),
	origin(other.origin),
	dir(other.dir)
{}


inline Ray::Ray(const Vec3& origin_, const Vec3& direction_)
:	CollisionShape(CST_RAY),
	origin(origin_),
	dir(direction_)
{}


} // end namespace


#endif
