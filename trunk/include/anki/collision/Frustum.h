#ifndef ANKI_COLLISION_FRUSTUM_H
#define ANKI_COLLISION_FRUSTUM_H

#include "anki/collision/CompoundShape.h"
#include "anki/collision/Plane.h"
#include "anki/collision/Obb.h"
#include "anki/collision/LineSegment.h"
#include "anki/Math.h"
#include "anki/util/Array.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Frustum collision shape. This shape consists from 6 planes. The planes are
/// being used to find shapes that are inside the frustum
class Frustum: public CompoundShape
{
public:
	/// Frustum type
	enum class Type: U8
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	/// The 6 frustum planes
	enum class PlaneType: U8
	{
		NEAR,
		FAR,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		COUNT ///< Number of planes
	};

	/// @name Constructors
	/// @{
	Frustum(Type type)
		: m_type(type)
	{}

	virtual ~Frustum()
	{}
	/// @}

	/// @name Accessors
	/// @{
	Type getType() const
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
		m_frustumDirty = true;
	}

	F32 getFar() const
	{
		return m_far;
	}
	void setFar(const F32 x)
	{
		m_far = x;
		m_frustumDirty = true;
	}

	const Transform& getTransform() const
	{
		return m_trf;
	}
	/// @}

	/// Override CompoundShape::transform
	void transform(const Transform& trf) override;

	/// Convenient method to reset the transformation
	void resetTransform(const Transform& trf);

	/// Check if a collision shape @a b is inside the frustum
	Bool insideFrustum(const CollisionShape& b);

	/// Calculate the projection matrix
	virtual Mat4 calculateProjectionMatrix() const = 0;

protected:
	/// @name Viewing variables
	/// @{
	F32 m_near = 0.0;
	F32 m_far = 0.0;
	/// @}

	/// Used to check against the frustum
	Array<Plane, (U)PlaneType::COUNT> m_planes;

	Transform m_trf = Transform::getIdentity(); ///< Keep the transformation

	/// It's true when the frustum changed
	Bool8 m_frustumDirty = true;

	/// Called when a viewing variable changes. It recalculates the planes and
	/// the other variables
	virtual void recalculate() = 0;

	/// Copy
	Frustum& operator=(const Frustum& b);

private:
	Type m_type;
};

/// Frustum shape for perspective cameras
class PerspectiveFrustum: public Frustum
{
public:
	/// @name Constructors
	/// @{

	/// Default
	PerspectiveFrustum();

	/// Copy
	PerspectiveFrustum(const PerspectiveFrustum& b)
		: PerspectiveFrustum()
	{
		*this = b;
	}

	/// Set all
	PerspectiveFrustum(F32 fovX, F32 fovY, F32 near, F32 far)
		: PerspectiveFrustum()
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
		m_frustumDirty = true;
	}

	F32 getFovY() const
	{
		return m_fovY;
	}
	void setFovY(F32 ang)
	{
		m_fovY = ang;
		m_frustumDirty = true;
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(F32 fovX, F32 fovY, F32 near, F32 far)
	{
		m_fovX = fovX;
		m_fovY = fovY,
		m_near = near;
		m_far = far;
		m_frustumDirty = true;
	}

	const Array<LineSegment, 4>& getLineSegments() const
	{
		return m_segments;
	}
	/// @}

	/// Copy
	PerspectiveFrustum& operator=(const PerspectiveFrustum& b);

	/// Calculate and get transformed
	PerspectiveFrustum getTransformed(const Transform& trf) const
	{
		PerspectiveFrustum o = *this;
		o.transform(trf);
		return o;
	}

	/// Implements Frustum::calculateProjectionMatrix
	Mat4 calculateProjectionMatrix() const;

private:
	/// @name Viewing variables
	/// @{
	F32 m_fovX = 0.0;
	F32 m_fovY = 0.0;
	/// @}

	/// @name Shape
	/// @{
	Array<LineSegment, 4> m_segments;
	/// @}

	/// Implements Frustum::recalculate. Recalculates:
	/// @li planes
	/// @li line segments
	void recalculate();
};

/// Frustum shape for orthographic cameras
class OrthographicFrustum: public Frustum
{
public:
	/// @name Constructors
	/// @{

	/// Default
	OrthographicFrustum();

	/// Copy
	OrthographicFrustum(const OrthographicFrustum& b)
		: OrthographicFrustum()
	{
		*this = b;
	}

	/// Set all
	OrthographicFrustum(F32 left, F32 right, F32 near,
		F32 far, F32 top, F32 bottom)
		: OrthographicFrustum()
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
		m_frustumDirty = true;
	}

	F32 getRight() const
	{
		return m_right;
	}
	void setRight(F32 f)
	{
		m_right = f;
		m_frustumDirty = true;
	}

	F32 getTop() const
	{
		return m_top;
	}
	void setTop(F32 f)
	{
		m_top = f;
		m_frustumDirty = true;
	}

	F32 getBottom() const
	{
		return m_bottom;
	}
	void setBottom(F32 f)
	{
		m_bottom = f;
		m_frustumDirty = true;
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
		m_frustumDirty = true;
	}

	/// Needed for debug drawing
	const Obb& getObb() const
	{
		return m_obb;
	}
	/// @}

	/// Copy
	OrthographicFrustum& operator=(const OrthographicFrustum& b);

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

	/// Implements Frustum::recalculate. Recalculate @a planes and
	/// @a obb
	void recalculate();
};
/// @}

} // end namespace anki

#endif
