#ifndef ANKI_COLLISION_SPHERE_H
#define ANKI_COLLISION_SPHERE_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Sphere collision shape
class Sphere: public CollisionShape
{
public:
	/// @name Constructors
	/// @{

	/// Default constructor
	Sphere()
		: CollisionShape(CST_SPHERE)
	{}

	/// Copy constructor
	Sphere(const Sphere& b)
		: CollisionShape(CST_SPHERE), center(b.center), radius(b.radius)
	{}

	/// Constructor
	Sphere(const Vec3& center_, F32 radius_)
		: CollisionShape(CST_SPHERE), center(center_), radius(radius_)
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

	/// Implements CollisionShape::toAabb
	void toAabb(Aabb& b) const;

	Sphere getTransformed(const Transform& transform) const;

	/// Get the sphere that includes this sphere and the given. See a
	/// drawing in the docs dir for more info about the algorithm
	Sphere getCompoundShape(const Sphere& b) const;

	/// Calculate from a set of points
	template<typename Container>
	void set(const Container& container);

private:
	Vec3 center;
	F32 radius;
};
/// @}

//==============================================================================
template<typename Container>
void Sphere::set(const Container& container)
{
	ANKI_ASSERT(container.size() >= 1);

	Vec3 min(container.front());
	Vec3 max(container.front());

	// for all the Vec3 calc the max and min
	typename Container::const_iterator it = container.begin() + 1;
	for(; it != container.end(); ++it)
	{
		const Vec3& v = *it;

		for(U j = 0; j < 3; j++)
		{
			if(v[j] > max[j])
			{
				max[j] = v[j];
			}
			else if(v[j] < min[j])
			{
				min[j] = v[j];
			}
		}
	}

	center = (min + max) * 0.5; // average

	// max distance between center and the vec3 arr
	F32 maxDist = (container.front() - center).getLengthSquared();

	typename Container::const_iterator it_ = container.begin() + 1;
	for(; it_ != container.end(); ++it_)
	{
		const Vec3& v = *it_;

		F32 dist = (v - center).getLengthSquared();
		if(dist > maxDist)
		{
			maxDist = dist;
		}
	}

	radius = Math::sqrt(maxDist);
}

} // end namespace

#endif
