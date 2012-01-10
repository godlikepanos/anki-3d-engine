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

	Frustum(FrustumType type_)
		: CollisionShape(CST_FRUSTUM), type(type_)
	{}

	/// Copy
	Frustum& operator=(const Frustum& b);

	/// Implements CollisionShape::transform
	void transform(const Transform& trf);

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

	/// Check if a collision shape is inside the frustum
	bool insideFrustum(const CollisionShape& b) const;

protected:
	boost::array<Plane, FP_COUNT> planes;
	float zNear, zFar;

	/// Called when a viewing variable changes. It recalculates the planes and
	/// the other variables
	virtual void recalculate() = 0;

private:
	FrustumType type;
};


/// XXX
class PerspectiveFrustum: public Frustum
{
public:
	PerspectiveFrustum()
		: Frustum(FT_PERSPECTIVE)
	{}

	/// @name Accessors
	/// @{
	float getFovX() const
	{
		return fovX;
	}
	void setFovX(float ang)
	{
		fovX = ang;
		recalculate();
	}

	float getFovY() const
	{
		return fovY;
	}
	void setFovY(float ang)
	{
		fovY = ang;
		recalculate();
	}

	void setAll(float fovX_, float fovY_, float zNear_, float zFar_)
	{
		fovX = fovX_;
		fovY = fovY_,
		zNear = zNear_;
		zFar = zFar_;
		recalculate();
	}
	/// @}

	/// Copy
	PerspectiveFrustum& operator=(const PerspectiveFrustum& b);

	/// Implements CollisionShape::testPlane
	float testPlane(const Plane& p) const;

	/// Re-implements Frustum::transform
	void transform(const Transform& trf);

private:
	Vec3 eye; ///< The eye point
	boost::array<Vec3, 4> dirs; ///< Directions
	float fovX;
	float fovY;

	/// Implements CollisionShape::recalculate
	void recalculate();
};


/// XXX
class OrthographicFrustum: public Frustum
{
public:

	/// @name Accessors
	/// @{
	float getLeft() const
	{
		return left;
	}
	void setLeft(float f)
	{
		left = f;
		recalculate();
	}

	float getRight() const
	{
		return right;
	}
	void setRight(float f)
	{
		right = f;
		recalculate();
	}

	float getTop() const
	{
		return top;
	}
	void setTop(float f)
	{
		top = f;
		recalculate();
	}

	float getBottom() const
	{
		return bottom;
	}
	void setBottom(float f)
	{
		bottom = f;
		recalculate();
	}

	void setAll(float left_, float right_, float zNear_,
		float zFar_, float top_, float bottom_)
	{
		left = left_;
		right = right_;
		zNear = zNear_;
		zFar = zFar_;
		top = top_;
		bottom = bottom_;
		recalculate();
	}
	/// @}

private:
	Obb obb;
	float left, right, top, bottom;
};
/// @}


} // end namespace


#endif
