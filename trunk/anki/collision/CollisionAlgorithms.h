#ifndef ANKI_COLLISION_COLLISION_ALGORITHMS_H
#define ANKI_COLLISION_COLLISION_ALGORITHMS_H

#include "anki/collision/Forward.h"


namespace anki {


/// Provides the collision algorithms that detect collision between collision
/// shapes
///
/// +------+------+------+------+------+------+------+
/// |      | LS   | OBB  | PCC  | P    | R    | S    |
/// +------+------+------+------+------+------+------+
/// | LS   |      |      |      |      |      |      |
/// +------+------+------+------+------+------+------+
/// | OBB  |      |      |      |      |      |      |
/// +------+------+------+------+------+------+------+
/// | PCS  |      |      |      |      |      |      |
/// +------+------+------+------+------+------+------+
/// | P    |      |      |      |      |      |      |
/// +------+------+------+------+------+------+------+
/// | R    |      |      |      |      |      |      |
/// +------+------+------+------+------+------+------+
/// | S    |      |      |      |      |      |      |
/// +------+------+------+------+------+------+------+
class CollisionAlgorithms
{
	public:
		// 1st line
		virtual bool collide(const LineSegment& a, const LineSegment& b) = 0;
		virtual bool collide(const LineSegment& a, const Obb& b) = 0;
		virtual bool collide(const LineSegment& a,
			const PerspectiveCameraShape& b) = 0;
		virtual bool collide(const LineSegment& a, const Ray& b) = 0;
		virtual bool collide(const LineSegment& a, const Sphere& b) = 0;

		// 2nd line
		bool collide(const Obb& a, const LineSegment& b)
		{
			return collide(b, a);
		}
		virtual bool collide(const Obb& a, const Obb& b) = 0;
		virtual bool collide(const Obb& a, const PerspectiveCameraShape& b) = 0;
		virtual bool collide(const Obb& a, const Ray& b) = 0;
		virtual bool collide(const Obb& a, const Sphere& b) = 0;

		// 3rd line
		bool collide(const PerspectiveCameraShape& a,
			const LineSegment& b)
		{
			return collide(b, a);
		}
		bool collide(const PerspectiveCameraShape& a, const Obb& b)
		{
			return collide(b, a);
		}
		virtual bool collide(const PerspectiveCameraShape& a,
			const PerspectiveCameraShape& b) = 0;
		virtual bool collide(const PerspectiveCameraShape& a, const Ray& b) = 0;
		virtual bool collide(const PerspectiveCameraShape& a,
			const Sphere& b) = 0;
};


} // end namespace


#endif
