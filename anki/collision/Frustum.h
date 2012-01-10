#ifndef ANKI_COLLISION_FRUSTUM_H
#define ANKI_COLLISION_FRUSTUM_H

#include "anki/collision/CollisionShape.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Obb.h"
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
	/// Frustum type
	enum FrustumType
	{
		FT_PERSPECTIVE,
		FT_ORTHOGRAPHIC
	};

	/// The 6 planes
	enum FrustrumPlane
	{
		FP_NEAR,
		FP_FAR,
		FP_LEFT,
		FP_RIGHT,
		FP_TOP,
		FP_BOTTOM,
		FP_COUNT
	};

	/// @name Constructors
	/// @{
	Frustum()
		: CollisionShape(CST_FRUSTUM), type(FT_ORTHOGRAPHIC)
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
		setPerspective(fovX, fovY, zNear, zFar, trf);
	}

	Frustum(float left, float right, float near,
		float far, float top, float buttom, const Transform& trf)
		: CollisionShape(CST_FRUSTUM)
	{
		setOrthographic(left, right, near, far, top, buttom, trf);
	}
	/// @}

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

	/// Set all for perspective camera
	void setPerspective(float fovX, float fovY, float zNear,
		float zFar, const Transform& trf);

	/// Set all for orthographic camera
	void setOrthographic(float left, float right, float near,
		float far, float top, float buttom, const Transform& trf)
	{
		type = FT_ORTHOGRAPHIC;
		/// XXX
	}

	/// Check if a collision shape is inside the frustum
	bool insideFrustum(const CollisionShape& b) const;

private:
	FrustumType type;

	boost::array<Plane, 6> planes;

	/// @name Including shape for perspective cameras
	/// @{
	Vec3 eye; ///< The eye point
	boost::array<Vec3, 4> dirs; ///< Directions
	/// @}

	/// @name Including shape for orthographic cameras
	/// @{
	Obb obb;
	/// @}
};
/// @}


} // end namespace


#endif
