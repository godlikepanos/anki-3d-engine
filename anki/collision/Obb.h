#ifndef ANKI_COLLISION_OBB_H
#define ANKI_COLLISION_OBB_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"
#include <array>


namespace anki {


/// @addtogroup Collision
/// @{

/// Object oriented bounding box
class Obb: public CollisionShape
{
public:
	/// @name Constructors
	/// @{
	Obb()
		: CollisionShape(CST_OBB)
	{}

	Obb(const Obb& b);

	Obb(const Vec3& center, const Mat3& rotation, const Vec3& extends);
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

	const Mat3& getRotation() const
	{
		return rotation;
	}
	Mat3& getRotation()
	{
		return rotation;
	}
	void setRotation(const Mat3& x)
	{
		rotation = x;
	}

	const Vec3& getExtend() const
	{
		return extends;
	}
	Vec3& getExtend()
	{
		return extends;
	}
	void setExtend(const Vec3& x)
	{
		extends = x;
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
	void getAabb(Aabb& aabb) const;

	Obb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not
	/// very accurate
	Obb getCompoundShape(const Obb& b) const;

	/// Calculate from a set of points
	template<typename Container>
	void set(const Container& container);

	/// Get extreme points in 3D space
	void getExtremePoints(std::array<Vec3, 8>& points) const;

public:
	/// @name Data
	/// @{
	Vec3 center;
	Mat3 rotation;
	/// With identity rotation this points to max (front, right, top in
	/// our case)
	Vec3 extends;
	/// @}
};
/// @}


//==============================================================================
template<typename Container>
void Obb::set(const Container& container)
{
	ANKI_ASSERT(container.size() >= 1);

	Vec3 min(container.front());
	Vec3 max(container.front());

	// for all the Vec3s calc the max and min
	typename Container::const_iterator it = container.begin() + 1;
	for(; it != container.end(); ++it)
	{
		const Vec3& v = *it;

		for(int j = 0; j < 3; j++)
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

	// set the locals
	center = (max + min) / 2.0;
	rotation = Mat3::getIdentity();
	extends = max - center;
}


} // end namespace


#endif
