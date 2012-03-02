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
		: CollisionShape(CST_RAY)
	{}

	/// Copy constructor
	Ray(const Ray& b)
		: CollisionShape(CST_RAY), origin(b.origin), dir(b.dir)
	{}


	/// Constructor
	Ray(const Vec3& origin_, const Vec3& direction_)
		: CollisionShape(CST_RAY), origin(origin_), dir(direction_)
	{}
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
	void accept(ConstVisitor& v) const
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	float testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::getAabb
	void getAabb(Aabb& b) const;

	Ray getTransformed(const Transform& transform) const;

private:
	Vec3 origin;
	Vec3 dir;
};
/// @}


} // end namespace


#endif
