#ifndef ANKI_COLLISION_COLLISION_ALGORITHMS_MATRIX_H
#define ANKI_COLLISION_COLLISION_ALGORITHMS_MATRIX_H

#include "anki/collision/Forward.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Provides the collision algorithms that detect collision between collision
/// shapes
///
/// @code
/// +------+------+------+------+------+------+------+------+
/// |      | LS   | OBB  | FRU  | P    | R    | S    | AABB |
/// +------+------+------+------+------+------+------+------+
/// | LS   | N/A  | OK   | N/I  | OK   | N/A  | OK   | OK   |
/// +------+------+------+------+------+------+------+------+
/// | OBB  |      | OK   | N/I  | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+------+
/// | FRU  |      |      | N/I  | N/I  | N/I  | N/I  | N/I  |
/// +------+------+------+------+------+------+------+------+
/// | P    |      |      |      | OK   | OK   | OK   | OK   |
/// +------+------+------+------+------+------+------+------+
/// | R    |      |      |      |      | N/A  | OK   | OK   |
/// +------+------+------+------+------+------+------+------+
/// | S    |      |      |      |      |      | OK   | OK   |
/// +------+------+------+------+------+------+------+------+
/// | AABB |      |      |      |      |      |      | OK   |
/// +------+------+------+------+------+------+------+------+
/// @endcode
class CollisionAlgorithmsMatrix
{
public:
	typedef LineSegment Ls;

	/// Generic collide function. It doesn't uses visitor pattern for
	/// speed reasons
	static Bool collide(const CollisionShape& a, const CollisionShape& b);

	// 1st line (LS)
	static Bool collide(const Ls& a, const Ls& b);
	static Bool collide(const Ls& a, const Obb& b);
	static Bool collide(const Ls& a, const Frustum& b);
	static Bool collide(const Ls& a, const Plane& b);
	static Bool collide(const Ls& a, const Ray& b);
	static Bool collide(const Ls& a, const Sphere& b);
	static Bool collide(const Ls& a, const Aabb& b);

	// 2nd line (OBB)
	static Bool collide(const Obb& a, const Ls& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Obb& a, const Obb& b);
	static Bool collide(const Obb& a, const Frustum& b);
	static Bool collide(const Obb& a, const Plane& b);
	static Bool collide(const Obb& a, const Ray& b);
	static Bool collide(const Obb& a, const Sphere& b);
	static Bool collide(const Obb& a, const Aabb& b);

	// 3rd line (FRU)
	static Bool collide(const Frustum& a, const Ls& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Frustum& a, const Obb& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Frustum& a, const Frustum& b);
	static Bool collide(const Frustum& a, const Plane& b);
	static Bool collide(const Frustum& a, const Ray& b);
	static Bool collide(const Frustum& a, const Sphere& b);
	static Bool collide(const Frustum& a, const Aabb& b);

	// 4th line (P)
	static Bool collide(const Plane& a, const Ls& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Plane& a, const Obb& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Plane& a,const Frustum& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Plane& a, const Plane& b);
	static Bool collide(const Plane& a, const Ray& b);
	static Bool collide(const Plane& a, const Sphere& b);
	static Bool collide(const Plane& a, const Aabb& b);

	// 5th line (R)
	static Bool collide(const Ray& a, const Ls& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Ray& a, const Obb& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Ray& a, const Frustum& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Ray& a, const Plane& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Ray& a, const Ray& b);
	static Bool collide(const Ray& a, const Sphere& b);
	static Bool collide(const Ray& a, const Aabb& b);

	// 6th line (S)
	static Bool collide(const Sphere& a, const Ls& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Sphere& a, const Obb& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Sphere& a, const Frustum& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Sphere& a, const Plane& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Sphere& a, const Ray& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Sphere& a, const Sphere& b);
	static Bool collide(const Sphere& a, const Aabb& b);

	// 7th line (AABB)
	static Bool collide(const Aabb& a, const Ls& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Aabb& a, const Obb& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Aabb& a, const Frustum& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Aabb& a, const Plane& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Aabb& a, const Ray& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Aabb& a, const Sphere& b)
	{
		return collide(b, a);
	}
	static Bool collide(const Aabb& a, const Aabb& b);

private:
	template<typename T>
	static Bool tcollide(const CollisionShape& a, const CollisionShape& b);
};
/// @}

} // end namespace anki

#endif
