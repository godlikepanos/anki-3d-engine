#ifndef ANKI_COLLISION_RAY_H
#define ANKI_COLLISION_RAY_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"


namespace anki {


/// @addtogroup Collision
/// @{

/// Ray collision shape. It has a starting point but the end extends to infinity
class Ray: public CollisionShape
{
	public:
		/// @name Constructors
		/// @{

		/// Default constructor
		Ray()
		:	CollisionShape(CST_RAY)
		{}

		/// Copy constructor
		Ray(const Ray& other);

		/// Constructor
		Ray(const Vec3& origin, const Vec3& direction);
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

		/// Implements CollisionShape::accept
		void accept(MutableVisitor& v)
		{
			v.visit(*this);
		}
		/// Implements CollisionShape::accept
		void accept(ConstVisitor& v)
		{
			v.visit(*this);
		}

		Ray getTransformed(const Transform& transform) const;

		/// Overrides CollisionShape::testPlane
		float testPlane(const Plane& p) const
		{
			return PlaneTests::test(p, *this);
		}

	private:
		Vec3 origin;
		Vec3 dir;
};
/// @}


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
