#ifndef ANKI_COLLISION_PLANE_TESTS_H
#define ANKI_COLLISION_PLANE_TESTS_H

#include "anki/collision/Forward.h"


namespace anki {


/// @addtogroup Collision
/// @{
///
/// Contains methods for testing the position between a collision shape and a
/// plane.
///
/// If the collision shape intersects with the plane then the method returns
/// 0.0, else it returns the distance. If the distance is < 0.0 then the
/// collision shape lies behind the plane and if > 0.0 then in front of it
class PlaneTests
{
	public:
		/// Generic method. It doesn't use visitor pattern for speed reasons
		static float test(const Plane& p, const CollisionShape& cs);

		static float test(const Plane& p, const LineSegment& ls);
		static float test(const Plane& p, const Obb& obb);
		static float test(const Plane& p, const PerspectiveCameraShape& pcs);
		static float test(const Plane& p, const Plane& p1);
		static float test(const Plane& p, const Ray& r);
		static float test(const Plane& p, const Sphere& s);
		static float test(const Plane& p, const Aabb& aabb);
};
/// @}


} // end namespace


#endif
