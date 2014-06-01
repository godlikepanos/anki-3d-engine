#ifndef ANKI_COLLISION_SPHERE_H
#define ANKI_COLLISION_SPHERE_H

#include "anki/collision/CollisionShape.h"
#include "anki/Math.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Sphere collision shape
class Sphere: public CollisionShape
{
public:
	/// @name Constructors
	/// @{

	/// Default constructor
	Sphere()
		: CollisionShape(Type::SPHERE)
	{}

	/// Copy constructor
	Sphere(const Sphere& b)
		:	CollisionShape(Type::SPHERE), 
			center(b.center), 
			radius(b.radius)
	{}

	/// Constructor
	Sphere(const Vec3& center_, F32 radius_)
		:	CollisionShape(Type::SPHERE), 
			center(center_), 
			radius(radius_)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getCenter() const
	{
		return center;
	}
	Vec3& getCenter()
	{
		return center;
	}
	void setCenter(const Vec3& x)
	{
		center = x;
	}

	F32 getRadius() const
	{
		return radius;
	}
	F32& getRadius()
	{
		return radius;
	}
	void setRadius(const F32 x)
	{
		radius = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Sphere& operator=(const Sphere& b)
	{
		center = b.center;
		radius = b.radius;
		return *this;
	}
	/// @}

	/// Check for collision
	template<typename T>
	Bool collide(const T& x) const
	{
		return detail::collide(*this, x);
	}

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
	F32 testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& b) const;

	Sphere getTransformed(const Transform& transform) const;

	/// Get the sphere that includes this sphere and the given. See a
	/// drawing in the docs dir for more info about the algorithm
	Sphere getCompoundShape(const Sphere& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(
		const void* buff, U count, PtrSize stride, PtrSize buffSize);

private:
	Vec3 center;
	F32 radius;
};
/// @}

} // end namespace anki

#endif
