#ifndef ANKI_COLLISION_AABB_H
#define ANKI_COLLISION_AABB_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Vec3.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Axis align bounding box collision shape
class Aabb: public CollisionShape
{
public:
	/// @name Constructors
	/// @{
	Aabb()
		: CollisionShape(CST_AABB)
	{}

	Aabb(const Vec3& min_, const Vec3& max_)
		: CollisionShape(CST_AABB), min(min_), max(max_)
	{
		ANKI_ASSERT(min < max);
	}

	Aabb(const Aabb& b)
		: CollisionShape(CST_AABB), min(b.min), max(b.max)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getMin() const
	{
		return min;
	}
	Vec3& getMin()
	{
		return min;
	}
	void setMin(const Vec3& x)
	{
		min = x;
	}

	const Vec3& getMax() const
	{
		return max;
	}
	Vec3& getMax()
	{
		return max;
	}
	void setMax(const Vec3& x)
	{
		max = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Aabb& operator=(const Aabb& b)
	{
		min = b.min;
		max = b.max;
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
	float testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::getAabb
	void getAabb(Aabb& b) const
	{
		b = *this;
	}

	/// It uses a nice trick to avoid unwanted calculations 
	Aabb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not
	/// very accurate
	Aabb getCompoundShape(const Aabb& b) const;

	/// Calculate from a set of points
	template<typename Container>
	void set(const Container& container);

private:
	/// @name Data
	/// @{
	Vec3 min;
	Vec3 max;
	/// @}
};
/// @}

//==============================================================================
template<typename Container>
void Aabb::set(const Container& container)
{
	ANKI_ASSERT(container.size() >= 1);

	min = container.front();
	max = min;

	// for all the Vec3s calc the max and min
	typename Container::const_iterator it = container.begin() + 1;
	for(; it != container.end(); ++it)
	{
		for(int j = 0; j < 3; j++)
		{
			if((*it)[j] > max[j])
			{
				max[j] = (*it)[j];
			}
			else if((*it)[j] < min[j])
			{
				min[j] = (*it)[j];
			}
		}
	}
}

} // end namespace

#endif
