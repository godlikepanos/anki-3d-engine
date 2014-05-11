#ifndef ANKI_COLLISION_OBB_H
#define ANKI_COLLISION_OBB_H

#include "anki/collision/CollisionShape.h"
#include "anki/Math.h"
#include "anki/util/Array.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Object oriented bounding box
class Obb: public CollisionShape
{
public:
	/// @name Constructors
	/// @{
	Obb();

	Obb(const Obb& b);

	Obb(const Vec3& center, const Mat3& rotation, const Vec3& extends);
	/// @}

	/// @name Accessors
	/// @{
	const Vec3& getCenter() const
	{
		return m_center;
	}
	Vec3& getCenter()
	{
		return m_center;
	}
	void setCenter(const Vec3& x)
	{
		m_center = x;
	}

	const Mat3& getRotation() const
	{
		return m_rotation;
	}
	Mat3& getRotation()
	{
		return m_rotation;
	}
	void setRotation(const Mat3& x)
	{
		m_rotation = x;
	}

	const Vec3& getExtend() const
	{
		return m_extends;
	}
	Vec3& getExtend()
	{
		return m_extends;
	}
	void setExtend(const Vec3& x)
	{
		m_extends = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Obb& operator=(const Obb& b)
	{
		m_center = b.m_center;
		m_rotation = b.m_rotation;
		m_extends = b.m_extends;
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
	void computeAabb(Aabb& aabb) const;

	Obb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not
	/// very accurate
	Obb getCompoundShape(const Obb& b) const;

	/// Calculate from a set of points
	template<typename Container>
	void set(const Container& container);

	/// Get extreme points in 3D space
	void getExtremePoints(Array<Vec3, 8>& points) const;

public:
	/// @name Data
	/// @{
	Vec3 m_center;
	Mat3 m_rotation;
	/// With identity rotation this points to max (front, right, top in
	/// our case)
	Vec3 m_extends;
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

	// set the locals
	m_center = (max + min) / 2.0;
	m_rotation = Mat3::getIdentity();
	m_extends = max - m_center;
}

} // end namespace

#endif
