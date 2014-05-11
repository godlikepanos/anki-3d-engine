#ifndef ANKI_COLLISION_FRUSTUM_H
#define ANKI_COLLISION_FRUSTUM_H

#include "anki/collision/CollisionShape.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Obb.h"
#include "anki/Math.h"
#include "anki/util/Array.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Frustum type
enum class FrustumType: U8
{
	PERSPECTIVE,
	ORTHOGRAPHIC
};

/// The 6 frustum planes
enum class FrustumPlane: U8
{
	NEAR,
	FAR,
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	COUNT ///< Number of planes
};

/// Frustum collision shape. This shape consists from 6 planes. The planes are
/// being used to find shapes that are inside the frustum
class Frustum: public CollisionShape
{
public:
	/// @name Constructors
	/// @{
	Frustum(FrustumType type)
		: CollisionShape(Type::FRUSTUM), m_type(type)
	{}

	virtual ~Frustum()
	{}
	/// @}

	/// @name Accessors
	/// @{
	FrustumType getFrustumType() const
	{
		return m_type;
	}

	F32 getNear() const
	{
		return m_near;
	}
	void setNear(const F32 x)
	{
		m_near = x;
		recalculate();
	}

	F32 getFar() const
	{
		return m_far;
	}
	void setFar(const F32 x)
	{
		m_far = x;
		recalculate();
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

	/// Check if a collision shape @a b is inside the frustum
	Bool insideFrustum(const CollisionShape& b) const;

	/// Calculate the projection matrix
	virtual Mat4 calculateProjectionMatrix() const = 0;

	/// Its like transform() but but with a difference. It doesn't transform
	/// the @a trf, it just replaces it
	virtual void setTransform(const Transform& trf) = 0;

	/// Implements CollisionShape::toAbb
	void computeAabb(Aabb& aabb) const;

protected:
	/// @name Viewing variables
	/// @{
	F32 m_near = 0.0;
	F32 m_far = 0.0;
	/// @}

	/// Used to check against the frustum
	Array<Plane, (U)FrustumPlane::COUNT> m_planes;

	Transform m_trf = Transform::getIdentity(); ///< Retain the transformation

	/// Called when a viewing variable changes. It recalculates the planes and
	/// the other variables
	virtual void recalculate() = 0;

	/// Copy
	Frustum& operator=(const Frustum& b);

	void transformPlanes()
	{
		for(Plane& p : m_planes)
		{
			p.transform(m_trf);
		}
	}

private:
	FrustumType m_type;
};

/// Frustum shape for perspective cameras
class PerspectiveFrustum: public Frustum
{
public:
	/// @name Constructors
	/// @{

	/// Default
	PerspectiveFrustum()
		: Frustum(FrustumType::PERSPECTIVE)
	{}

	/// Copy
	PerspectiveFrustum(const PerspectiveFrustum& b)
		: Frustum(FrustumType::PERSPECTIVE)
	{
		*this = b;
	}

	/// Set all
	PerspectiveFrustum(F32 fovX, F32 fovY, F32 near, F32 far)
		: Frustum(FrustumType::PERSPECTIVE)
	{
		setAll(fovX, fovY, near, far);
	}
	/// @}

	/// @name Accessors
	/// @{
	F32 getFovX() const
	{
		return m_fovX;
	}
	void setFovX(F32 ang)
	{
		m_fovX = ang;
		recalculate();
	}

	F32 getFovY() const
	{
		return m_fovY;
	}
	void setFovY(F32 ang)
	{
		m_fovY = ang;
		recalculate();
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(F32 fovX, F32 fovY, F32 near, F32 far)
	{
		m_fovX = fovX;
		m_fovY = fovY,
		m_near = near;
		m_far = far;
		recalculate();
	}

	const Array<Vec3, 4>& getDirections() const
	{
		return m_dirs;
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

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& aabb) const;

private:
	/// @name Viewing variables
	/// @{
	F32 m_fovX = 0.0;
	F32 m_fovY = 0.0;
	/// @}

	/// @name Shape
	/// @{
	Vec3 m_eye; ///< The eye point
	Array<Vec3, 4> m_dirs; ///< Directions
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
		: Frustum(FrustumType::ORTHOGRAPHIC)
	{}

	/// Copy
	OrthographicFrustum(const OrthographicFrustum& b)
		: Frustum(FrustumType::ORTHOGRAPHIC)
	{
		*this = b;
	}

	/// Set all
	OrthographicFrustum(F32 left, F32 right, F32 near,
		F32 far, F32 top, F32 bottom)
		: Frustum(FrustumType::ORTHOGRAPHIC)
	{
		setAll(left, right, near, far, top, bottom);
	}
	/// @}

	/// @name Accessors
	/// @{
	F32 getLeft() const
	{
		return m_left;
	}
	void setLeft(F32 f)
	{
		m_left = f;
		recalculate();
	}

	F32 getRight() const
	{
		return m_right;
	}
	void setRight(F32 f)
	{
		m_right = f;
		recalculate();
	}

	F32 getTop() const
	{
		return m_top;
	}
	void setTop(F32 f)
	{
		m_top = f;
		recalculate();
	}

	F32 getBottom() const
	{
		return m_bottom;
	}
	void setBottom(F32 f)
	{
		m_bottom = f;
		recalculate();
	}

	/// Set all
	void setAll(F32 left, F32 right, F32 near,
		F32 far, F32 top, F32 bottom)
	{
		m_left = left;
		m_right = right;
		m_near = near;
		m_far = far;
		m_top = top;
		m_bottom = bottom;
		recalculate();
	}

	/// Needed for debug drawing
	const Obb& getObb() const
	{
		return m_obb;
	}
	/// @}

	/// Copy
	OrthographicFrustum& operator=(const OrthographicFrustum& b);

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const
	{
		return m_obb.testPlane(p);
	}

	/// Override Frustum::transform
	void transform(const Transform& trf);

	/// Implements Frustum::setTransform
	void setTransform(const Transform& trf);

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& aabb) const
	{
		m_obb.computeAabb(aabb);
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
	F32 m_left = 0.0, m_right = 0.0, m_top = 0.0, m_bottom = 0.0;
	/// @}

	/// @name Shape
	/// @{
	Obb m_obb; ///< Including shape
	/// @}

	/// Implements CollisionShape::recalculate. Recalculate @a planes and
	/// @a obb
	void recalculate();

	/// Transform the @a obb using @a Frustrum::trf
	void transformShape()
	{
		m_obb.transform(m_trf);
	}
};
/// @}

} // end namespace

#endif
