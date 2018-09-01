// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/CompoundShape.h>
#include <anki/collision/Plane.h>
#include <anki/collision/Obb.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/Math.h>
#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup Collision
/// @{

/// Frustum type
enum class FrustumType : U8
{
	PERSPECTIVE,
	ORTHOGRAPHIC
};

/// Frustum collision shape. This shape consists from 6 planes. The planes are being used to find shapes that are
/// inside the frustum
class Frustum : public CompoundShape
{
public:
	Frustum(FrustumType type)
		: m_type(type)
	{
	}

	virtual ~Frustum()
	{
	}

	FrustumType getType() const
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
		update();
	}

	F32 getFar() const
	{
		return m_far;
	}

	void setFar(const F32 x)
	{
		m_far = x;
		update();
	}

	const Transform& getTransform() const
	{
		return m_trf;
	}

	/// Override CompoundShape::accept
	void accept(MutableVisitor& v) override;

	/// Override CompoundShape::accept
	void accept(ConstVisitor& v) const override;

	/// Override CompoundShape::testPlane
	F32 testPlane(const Plane& p) const override;

	/// Override CompoundShape::testPlane computeAabb
	void computeAabb(Aabb&) const override;

	/// Override CompoundShape::transform
	void transform(const Transform& trf) override;

	/// Convenient method to reset the transformation
	void resetTransform(const Transform& trf);

	/// Check if a collision shape @a b is inside the frustum
	Bool insideFrustum(const CollisionShape& b) const;

	/// Calculate the projection matrix
	virtual Mat4 calculateProjectionMatrix() const = 0;

	const Array<Plane, U(FrustumPlaneType::COUNT)>& getPlanesWorldSpace() const
	{
		return m_planesW;
	}

protected:
	/// @name Viewing variables
	/// @{
	F32 m_near = 0.0;
	F32 m_far = 0.0;
	/// @}

	/// Used to check against the frustum
	Array<Plane, U(FrustumPlaneType::COUNT)> m_planesL;
	Array<Plane, U(FrustumPlaneType::COUNT)> m_planesW;

	/// Keep the transformation.
	Transform m_trf = Transform::getIdentity();

	/// Called when a viewing variable changes. It recalculates the planes and the other variables.
	virtual void recalculate() = 0;

	/// Called when there is a change in the transformation.
	virtual void onTransform() = 0;

	/// Update if dirty
	void update();

	/// Copy
	Frustum& operator=(const Frustum& b);

private:
	FrustumType m_type;
};

/// Frustum shape for perspective cameras
class PerspectiveFrustum : public Frustum
{
public:
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

	/// Get FOV on X axis.
	F32 getFovX() const
	{
		return m_fovX;
	}
	void setFovX(F32 ang)
	{
		m_fovX = ang;
		update();
	}

	/// Get FOV on Y axis.
	F32 getFovY() const
	{
		return m_fovY;
	}
	void setFovY(F32 ang)
	{
		m_fovY = ang;
		update();
	}

	/// Set all the parameters and recalculate the planes and shape
	void setAll(F32 fovX, F32 fovY, F32 near, F32 far)
	{
		ANKI_ASSERT(near > 0.0);
		ANKI_ASSERT(far > near);
		ANKI_ASSERT(fovX <= PI);
		ANKI_ASSERT(fovY <= PI);
		m_fovX = fovX;
		m_fovY = fovY;
		m_near = near;
		m_far = far;
		update();
	}

	const Array<Vec4, 5>& getPoints() const
	{
		return m_pointsW;
	}

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
	Mat4 calculateProjectionMatrix() const override;

private:
	/// @name Viewing variables
	/// @{
	F32 m_fovX = 0.0;
	F32 m_fovY = 0.0;
	/// @}

	/// @name Shape
	/// @{
	Array<Vec4, 5> m_pointsW;
	Array<Vec4, 4> m_pointsL; ///< Don't need the eye point.
	ConvexHullShape m_hull;
	/// @}

	/// Implements Frustum::recalculate. Recalculates:
	/// @li planes
	/// @li line segments
	void recalculate() override;

	void onTransform() override;
};

/// Frustum shape for orthographic cameras
class OrthographicFrustum : public Frustum
{
public:
	/// Default
	OrthographicFrustum();

	/// Copy
	OrthographicFrustum(const OrthographicFrustum& b)
		: OrthographicFrustum()
	{
		*this = b;
	}

	/// Set all
	OrthographicFrustum(F32 left, F32 right, F32 near, F32 far, F32 top, F32 bottom)
		: OrthographicFrustum()
	{
		setAll(left, right, near, far, top, bottom);
	}

	F32 getLeft() const
	{
		return m_left;
	}

	void setLeft(F32 f)
	{
		m_left = f;
		update();
	}

	F32 getRight() const
	{
		return m_right;
	}

	void setRight(F32 f)
	{
		m_right = f;
		update();
	}

	F32 getTop() const
	{
		return m_top;
	}

	void setTop(F32 f)
	{
		m_top = f;
		update();
	}

	F32 getBottom() const
	{
		return m_bottom;
	}

	void setBottom(F32 f)
	{
		m_bottom = f;
		update();
	}

	/// Set all
	void setAll(F32 left, F32 right, F32 near, F32 far, F32 top, F32 bottom)
	{
		ANKI_ASSERT(left < right && far > near && bottom < top);
		m_left = left;
		m_right = right;
		m_near = near;
		m_far = far;
		m_top = top;
		m_bottom = bottom;
		update();
	}

	/// Needed for debug drawing
	const Obb& getObb() const
	{
		return m_obbW;
	}

	/// Copy
	OrthographicFrustum& operator=(const OrthographicFrustum& b);

	/// Implements Frustum::calculateProjectionMatrix
	Mat4 calculateProjectionMatrix() const override;

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
	Obb m_obbL, m_obbW; ///< Including shape
	/// @}

	/// Implements Frustum::recalculate. Recalculate @a m_planesL and @a m_obb
	void recalculate() override;

	void onTransform() override;
};
/// @}

} // end namespace anki
