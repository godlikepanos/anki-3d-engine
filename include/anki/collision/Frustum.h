#ifndef ANKI_COLLISION_FRUSTUM_H
#define ANKI_COLLISION_FRUSTUM_H

#include "anki/collision/CollisionShape.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Obb.h"
#include "anki/math/Math.h"
#include <array>

namespace anki {

/// @addtogroup Collision
/// @{

/// Frustum collision shape. This shape consists from 6 planes. The planes are
/// being used to find shapes that are inside the frustum
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
		FP_COUNT ///< Number of planes
	};

	/// @name Constructors
	/// @{
	Frustum(FrustumType type_)
		: CollisionShape(CST_FRUSTUM), type(type_)
	{}

	virtual ~Frustum()
	{}
	/// @}

	/// @name Accessors
	/// @{
	FrustumType getFrustumType() const
	{
		return type;
	}

	float getNear() const
	{
		return zNear;
	}
	float& getNear()
	{
		return zNear;
	}
	void setNear(const float x)
	{
		zNear = x;
	}

	float getFar() const
	{
		return zFar;
	}
	float& getFar()
	{
		return zFar;
	}
	void setFar(const float x)
	{
		zFar = x;
	}
	/// @}

	/// Copy
	Frustum& operator=(const Frustum& b);

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

	/// Check if a collision shape @a b is inside the frustum
	bool insideFrustum(const CollisionShape& b) const;

	/// Calculate the projection matrix
	virtual Mat4 calculateProjectionMatrix() const = 0;

protected:
	/// @name Viewing variables
	/// @{
	float zNear;
	float zFar;
	/// @}

	Transform trf;

	/// Used to check against the frustum
	mutable std::array<Plane, FP_COUNT> planes;

	/// It recalculates the planes in local space
	virtual void recalculatePlanes() const = 0;

	/// Self explanatory
	void transformPlanes() const
	{
		for(Plane& p : planes)
		{
			p.transform(trf);
		}
	}

private:
	FrustumType type;
};

/// Frustum shape for perspective cameras
class PerspectiveFrustum: public Frustum
{
public:
	/// @name Constructors
	/// @{

	/// Default
	PerspectiveFrustum()
		: Frustum(FT_PERSPECTIVE)
	{}

	/// Copy
	PerspectiveFrustum(const PerspectiveFrustum& b)
		: Frustum(FT_PERSPECTIVE)
	{
		*this = b;
	}

	/// Set all
	PerspectiveFrustum(float fovX_, float fovY_, float zNear_, float zFar_)
		: Frustum(FT_PERSPECTIVE)
	{
		setAll(fovX_, fovY_, zNear_, zFar_);
	}
	/// @}

	/// @name Accessors
	/// @{
	float getFovX() const
	{
		return fovX;
	}
	float& getFovX()
	{
		return fovX;
	}
	void setFovX(float ang)
	{
		fovX = ang;
	}

	float getFovY() const
	{
		return fovY;
	}
	float& getFovY()
	{
		return fovY;
	}
	void setFovY(float ang)
	{
		fovY = ang;
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(float fovX_, float fovY_, float zNear_, float zFar_)
	{
		fovX = fovX_;
		fovY = fovY_,
		zNear = zNear_;
		zFar = zFar_;
	}
	/// @}

	/// Copy
	PerspectiveFrustum& operator=(const PerspectiveFrustum& b);

	/// Implements CollisionShape::testPlane
	float testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	virtual void transform(const Transform& trf);

	/// Calculate and get transformed
	PerspectiveFrustum getTransformed(const Transform& trf) const
	{
		PerspectiveFrustum o = *this;
		o.transform(trf);
		return o;
	}

	/// Implements Frustum::calculateProjectionMatrix
	Mat4 calculateProjectionMatrix() const;

	/// Implements CollisionShape::getAabb
	void getAabb(Aabb& aabb) const;

private:
	/// @name Shape
	/// @{
	mutable Vec3 eye; ///< The eye point
	mutable std::array<Vec3, 4> dirs; ///< Directions
	/// @}

	/// @name Viewing variables
	/// @{
	float fovX;
	float fovY;
	/// @}

	/// Implements CollisionShape::recalculatePlanes
	void recalculatePlanes() const;

	/// Recalculate @a eye and @a dirs
	void recalculateShape() const;

	/// Transform @a eye and @a dirs
	void transformShape() const;
};

/// Frustum shape for orthographic cameras
class OrthographicFrustum: public Frustum
{
public:
	/// @name Constructors
	/// @{

	/// Default
	OrthographicFrustum()
		: Frustum(FT_ORTHOGRAPHIC)
	{}

	/// Copy
	OrthographicFrustum(const OrthographicFrustum& b)
		: Frustum(FT_ORTHOGRAPHIC)
	{
		*this = b;
	}

	/// Set all
	OrthographicFrustum(float left_, float right_, float zNear_,
		float zFar_, float top_, float bottom_)
		: Frustum(FT_ORTHOGRAPHIC)
	{
		setAll(left_, right_, zNear_, zFar_, top_, bottom_);
	}
	/// @}

	/// @name Accessors
	/// @{
	float getLeft() const
	{
		return left;
	}
	float& getLeft()
	{
		return left;
	}
	void setLeft(float f)
	{
		left = f;
	}

	float getRight() const
	{
		return right;
	}
	float& getRight()
	{
		return right;
	}
	void setRight(float f)
	{
		right = f;
	}

	float getTop() const
	{
		return top;
	}
	float& getTop()
	{
		return top;
	}
	void setTop(float f)
	{
		top = f;
	}

	float getBottom() const
	{
		return bottom;
	}
	float& getBottom()
	{
		return bottom;
	}
	void setBottom(float f)
	{
		bottom = f;
	}

	/// Set all
	void setAll(float left_, float right_, float zNear_,
		float zFar_, float top_, float bottom_)
	{
		left = left_;
		right = right_;
		zNear = zNear_;
		zFar = zFar_;
		top = top_;
		bottom = bottom_;
	}
	/// @}

	/// Copy
	OrthographicFrustum& operator=(const OrthographicFrustum& b);

	/// Implements CollisionShape::testPlane
	float testPlane(const Plane& p) const;

	/// Override Frustum::transform
	void transform(const Transform& trf);

	/// Implements CollisionShape::getAabb
	void getAabb(Aabb& aabb) const;

	/// Implements Frustum::calculateProjectionMatrix
	Mat4 calculateProjectionMatrix() const;

private:
	/// @name Shape
	/// @{
	mutable Obb obb; ///< Including shape
	/// @}

	/// @name Viewing variables
	/// @{
	float left, right, top, bottom;
	/// @}

	/// Implements Frustum::recalculatePlanes
	void recalculatePlanes() const;

	void recalculateShape() const;

	void transformShape() const
	{
		obb.transform(trf);
	}
};
/// @}

} // end namespace

#endif
