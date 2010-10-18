#ifndef SPHERE_H
#define SPHERE_H

#include "CollisionShape.h"
#include "Properties.h"
#include "Math.h"


class Plane;


/// Sphere collision shape
class Sphere: public CollisionShape
{
	PROPERTY_RW(Vec3, center, setCenter, getCenter)
	PROPERTY_RW(float, radius, setRadius, getRadius)

	public:
		/// Default constructor
		Sphere(): CollisionShape(CST_SPHERE) {}

		/// Copy constructor
		Sphere(const Sphere& other);

		/// Constructor
		Sphere(const Vec3& center_, float radius_);

		/// @see set
		Sphere(const float* pointer, size_t stride, int count);

		Sphere getTransformed(const Vec3& translate, const Mat3& rotate, float scale) const;

		/// @see CollisionShape::testPlane
		float testPlane(const Plane& plane) const;

	private:
		/// Set from Vec3 array
		/// @param pointer The start of the array
		/// @param stride The space between the elements
		/// @param count The number of 3D vectors
		void set(const float* pointer, size_t stride, int count);
};


inline Sphere::Sphere(const Sphere& b):
	CollisionShape(CST_SPHERE),
	center(b.center),
	radius(b.radius)
{}


inline Sphere::Sphere(const Vec3& center_, float radius_):
	CollisionShape(CST_SPHERE),
	center(center_),
	radius(radius_)
{}


inline Sphere::Sphere(const float* pointer, size_t stride, int count):
	CollisionShape(CST_SPHERE)
{
	set(pointer, stride, count);
}


#endif
