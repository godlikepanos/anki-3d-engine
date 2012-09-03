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

	F32 getNear() const
	{
		return near;
	}
	void setNear(const F32 x)
	{
		near = x;
		recalculate();
	}

	F32 getFar() const
	{
		return far;
	}
	void setFar(const F32 x)
	{
		far = x;
		recalculate();
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

	/// Check if a collision shape @a b is inside the frustum
	bool insideFrustum(const CollisionShape& b) const;

	/// Calculate the projection matrix
	virtual Mat4 calculateProjectionMatrix() const = 0;

	/// Its like transform() but but with a difference. It doesn't transform
	/// the @a trf, it just replaces it
	virtual void setTransform(const Transform& trf) = 0;

public: // XXX
	/// @name Viewing variables
	/// @{
	F32 near = 0.0;
	F32 far = 0.0;
	/// @}

	/// Used to check against the frustum
	std::array<Plane, FP_COUNT> planes;

	Transform trf = Transform::getIdentity(); ///< Retain the transformation

	/// Called when a viewing variable changes. It recalculates the planes and
	/// the other variables
	virtual void recalculate() = 0;

	/// Copy
	Frustum& operator=(const Frustum& b);

	void transformPlanes()
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
	PerspectiveFrustum(F32 fovX_, F32 fovY_, F32 near_, F32 far_)
		: Frustum(FT_PERSPECTIVE)
	{
		setAll(fovX_, fovY_, near_, far_);
	}
	/// @}

	/// @name Accessors
	/// @{
	F32 getFovX() const
	{
		return fovX;
	}
	void setFovX(F32 ang)
	{
		fovX = ang;
		recalculate();
	}

	F32 getFovY() const
	{
		return fovY;
	}
	void setFovY(F32 ang)
	{
		fovY = ang;
		recalculate();
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(F32 fovX_, F32 fovY_, F32 near_, F32 far_)
	{
		fovX = fovX_;
		fovY = fovY_,
		near = near_;
		far = far_;
		recalculate();
	}
	/// @}

	/// Copy
	PerspectiveFrustum& operator=(const PerspectiveFrustum& b);

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const;

	/// Calculate and get transformed
	PerspectiveFrustum getTransformed(const Transform& trf) const
	{
		PerspectiveFrustum o = *this;
		o.transform(trf);
		return o;
	}

	/// Re-implements Frustum::transform
	void transform(const Transform& trf);

	/// Implements Frustum::setTransform
	void setTransform(const Transform& trf);

	/// Implements Frustum::calculateProjectionMatrix
	Mat4 calculateProjectionMatrix() const;

	/// Implements CollisionShape::toAabb
	void toAabb(Aabb& aabb) const;

private:
	/// @name Viewing variables
	/// @{
	F32 fovX = 0.0;
	F32 fovY = 0.0;
	/// @}

	/// @name Shape
	/// @{
	Vec3 eye; ///< The eye point
	std::array<Vec3, 4> dirs; ///< Directions
	/// @}

	/// Implements CollisionShape::recalculate. Recalculate:
	/// @li planes
	/// @li eye
	/// @li dirs
	void recalculate();

	/// Transform the @a eye and @a dirs using @a Frustrum::trf
	void transformShape();
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
	OrthographicFrustum(F32 left_, F32 right_, F32 near_,
		F32 far_, F32 top_, F32 bottom_)
		: Frustum(FT_ORTHOGRAPHIC)
	{
		setAll(left_, right_, near_, far_, top_, bottom_);
	}
	/// @}

	/// @name Accessors
	/// @{
	F32 getLeft() const
	{
		return left;
	}
	void setLeft(F32 f)
	{
		left = f;
		recalculate();
	}

	F32 getRight() const
	{
		return right;
	}
	void setRight(F32 f)
	{
		right = f;
		recalculate();
	}

	F32 getTop() const
	{
		return top;
	}
	void setTop(F32 f)
	{
		top = f;
		recalculate();
	}

	F32 getBottom() const
	{
		return bottom;
	}
	void setBottom(F32 f)
	{
		bottom = f;
		recalculate();
	}

	/// Set all
	void setAll(F32 left_, F32 right_, F32 near_,
		F32 far_, F32 top_, F32 bottom_)
	{
		left = left_;
		right = right_;
		near = near_;
		far = far_;
		top = top_;
		bottom = bottom_;
		recalculate();
	}

	/// Needed for debug drawing
	const Obb& getObb() const
	{
		return obb;
	}
	/// @}

	/// Copy
	OrthographicFrustum& operator=(const OrthographicFrustum& b);

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const
	{
		return obb.testPlane(p);
	}

	/// Override Frustum::transform
	void transform(const Transform& trf);

	/// Implements Frustum::setTransform
	void setTransform(const Transform& trf);

	/// Implements CollisionShape::toAabb
	void toAabb(Aabb& aabb) const
	{
		obb.toAabb(aabb);
	}

	/// Implements Frustum::calculateProjectionMatrix
	Mat4 calculateProjectionMatrix() const;

	/// Calculate and get transformed
	OrthographicFrustum getTransformed(const Transform& trf) const
	{
		OrthographicFrustum o = *this;
		o.transform(trf);
		return o;
	}

private:
	/// @name Viewing variables
	/// @{
	F32 left = 0.0, right = 0.0, top = 0.0, bottom = 0.0;
	/// @}

	/// @name Shape
	/// @{
	Obb obb; ///< Including shape
	/// @}

	/// Implements CollisionShape::recalculate. Recalculate @a planes and
	/// @a obb
	void recalculate();

	/// Transform the @a obb using @a Frustrum::trf
	void transformShape()
	{
		obb.transform(trf);
	}
};
/// @}

} // end namespace

#endif
