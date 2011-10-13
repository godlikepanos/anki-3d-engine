#ifndef ANKI_COLLISION_COLLISION_ALGORITHMS_MATRIX_H
#define ANKI_COLLISION_COLLISION_ALGORITHMS_MATRIX_H

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
class CollisionAlgorithmsMatrix
{
	public:
		typedef PerspectiveCameraShape Pcs;
		typedef LineSegment Ls;

		// 1st line (LS)
		virtual bool collide(const Ls& a, const Ls& b) const = 0;
		virtual bool collide(const Ls& a, const Obb& b) const = 0;
		virtual bool collide(const Ls& a, const Pcs& b) const = 0;
		virtual bool collide(const Ls& a, const Plane& b) const = 0;
		virtual bool collide(const Ls& a, const Ray& b) const = 0;
		virtual bool collide(const Ls& a, const Sphere& b) const = 0;

		// 2nd line (OBB)
		bool collide(const Obb& a, const Ls& b) const
		{
			return collide(b, a);
		}
		virtual bool collide(const Obb& a, const Obb& b) const = 0;
		virtual bool collide(const Obb& a, const Pcs& b) const = 0;
		virtual bool collide(const Obb& a, const Plane& b) const = 0;
		virtual bool collide(const Obb& a, const Ray& b) const = 0;
		virtual bool collide(const Obb& a, const Sphere& b) const = 0;

		// 3rd line (PCS)
		bool collide(const Pcs& a, const Ls& b) const
		{
			return collide(b, a);
		}
		bool collide(const Pcs& a, const Obb& b) const
		{
			return collide(b, a);
		}
		virtual bool collide(const Pcs& a, const Pcs& b) const = 0;
		virtual bool collide(const Pcs& a, const Plane& b) const = 0;
		virtual bool collide(const Pcs& a, const Ray& b) const = 0;
		virtual bool collide(const Pcs& a, const Sphere& b) const = 0;

		// 4th line (P)
		bool collide(const Plane& a, const Ls& b) const
		{
			return collide(b, a);
		}
		bool collide(const Plane& a, const Obb& b) const
		{
			return collide(b, a);
		}
		bool collide(const Plane& a,const Pcs& b) const
		{
			return collide(b, a);
		}
		virtual bool collide(const Plane& a, const Plane& b) const = 0;
		virtual bool collide(const Plane& a, const Ray& b) const = 0;
		virtual bool collide(const Plane& a, const Sphere& b) const = 0;

		// 5th line (R)
		bool collide(const Ray& a, const Ls& b) const
		{
			return collide(b, a);
		}
		bool collide(const Ray& a, const Obb& b) const
		{
			return collide(b, a);
		}
		bool collide(const Ray& a, const Pcs& b) const
		{
			return collide(b, a);
		}
		bool collide(const Ray& a, const Plane& b) const
		{
			return collide(b, a);
		}
		virtual bool collide(const Ray& a, const Ray& b) const = 0;
		virtual bool collide(const Ray& a, const Sphere& b) const = 0;

		// 6th line (S)
		bool collide(const Sphere& a, const Ls& b) const
		{
			return collide(b, a);
		}
		bool collide(const Sphere& a, const Obb& b) const
		{
			return collide(b, a);
		}
		bool collide(const Sphere& a, const Pcs& b) const
		{
			return collide(b, a);
		}
		bool collide(const Sphere& a, const Plane& b) const
		{
			return collide(b, a);
		}
		bool collide(const Sphere& a, const Ray& b) const
		{
			return collide(b, a);
		}
		virtual bool collide(const Sphere& a, const Sphere& b) const = 0;
};


/// The default implementation of collision algorithms
class DefaultCollisionAlgorithmsMatrix: public CollisionAlgorithmsMatrix
{
	public:
		// 1st line (LS)
		virtual bool collide(const Ls& a, const Ls& b) const;
		virtual bool collide(const Ls& a, const Obb& b) const;
		virtual bool collide(const Ls& a, const Pcs& b) const;
		virtual bool collide(const Ls& a, const Plane& b) const;
		virtual bool collide(const Ls& a, const Ray& b) const;
		virtual bool collide(const Ls& a, const Sphere& b) const;

		// 2nd line (OBB)
		virtual bool collide(const Obb& a, const Obb& b) const;
		virtual bool collide(const Obb& a, const Pcs& b) const;
		virtual bool collide(const Obb& a, const Plane& b) const;
		virtual bool collide(const Obb& a, const Ray& b) const;
		virtual bool collide(const Obb& a, const Sphere& b) const;

		// 3rd line (PCS)
		virtual bool collide(const Pcs& a, const Pcs& b) const;
		virtual bool collide(const Pcs& a, const Plane& b) const;
		virtual bool collide(const Pcs& a, const Ray& b) const;
		virtual bool collide(const Pcs& a, const Sphere& b) const;

		// 4th line (P)
		virtual bool collide(const Plane& a, const Plane& b) const;
		virtual bool collide(const Plane& a, const Ray& b) const;
		virtual bool collide(const Plane& a, const Sphere& b) const;

		// 5th line (R)
		virtual bool collide(const Ray& a, const Ray& b) const;
		virtual bool collide(const Ray& a, const Sphere& b) const;

		// 6th line (S)
		virtual bool collide(const Sphere& a, const Sphere& b) const;
};


} // end namespace


#endif
