#ifndef ANKI_COLLISION_FRUSTUM_H
#define ANKI_COLLISION_FRUSTUM_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"
#include <boost/array.hpp>


namespace anki {


/// @addtogroup Collision
/// @{

/// Frustum collision shape
///
/// This shape consists from 6 planes. The planes are being used to find shapes
/// that are inside the frustum
class Frustum: public CollisionShape
{
public:
	Frustum()
		: CollisionShape(CST_FRUSTUM)
	{}

	Frustum(const Frustum& b)
		: CollisionShape(CST_FRUSTUM)
	{
		*this = b;
	}

	Frustum(float fovX, float fovY, float zNear,
		float zFar, const Transform& trf)
		: CollisionShape(CST_FRUSTUM)
	{
		// XXX
	}

	/// Copy
	Frustum& operator=(const Frustum& b);

	/// Implements CollisionShape::testPlane
	float testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	Frustum getTransformed(const Transform& trf) const;

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

	/// Set all
	void setAll(float fovX, float fovY, float zNear,
		float zFar, const Transform& trf);

private:
	boost::array<Plane, 6> planes;
	Vec3 eye; ///< The eye point
	boost::array<Vec3, 4> dirs; ///< Directions
};
/// @}


} // end namespace
