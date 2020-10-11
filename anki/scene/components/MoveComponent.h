// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/components/SceneComponent.h>
#include <anki/util/BitMask.h>
#include <anki/util/Enum.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup scene
/// @{

enum class MoveComponentFlag : U8
{
	NONE = 0,

	/// Get the parent's world transform
	IGNORE_LOCAL_TRANSFORM = 1 << 1,

	/// Ignore parent's transform
	IGNORE_PARENT_TRANSFORM = 1 << 2,

	/// If dirty then is marked for update
	MARKED_FOR_UPDATE = 1 << 3,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MoveComponentFlag)

/// Interface for movable scene nodes
class MoveComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::MOVE;

	/// The one and only constructor
	/// @param flags The flags
	MoveComponent(MoveComponentFlag flags = MoveComponentFlag::NONE);

	~MoveComponent();

	const Transform& getLocalTransform() const
	{
		return m_ltrf;
	}

	void setLocalTransform(const Transform& x)
	{
		m_ltrf = x;
		markForUpdate();
	}

	void setLocalOrigin(const Vec4& x)
	{
		m_ltrf.setOrigin(x);
		markForUpdate();
	}

	const Vec4& getLocalOrigin() const
	{
		return m_ltrf.getOrigin();
	}

	void setLocalRotation(const Mat3x4& x)
	{
		m_ltrf.setRotation(x);
		markForUpdate();
	}

	const Mat3x4& getLocalRotation() const
	{
		return m_ltrf.getRotation();
	}

	void setLocalScale(F32 x)
	{
		m_ltrf.setScale(x);
		markForUpdate();
	}

	F32 getLocalScale() const
	{
		return m_ltrf.getScale();
	}

	const Transform& getWorldTransform() const
	{
		return m_wtrf;
	}

	const Transform& getPreviousWorldTransform() const
	{
		return m_prevWTrf;
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(F32 angDegrees)
	{
		m_ltrf.getRotation().rotateXAxis(angDegrees);
		markForUpdate();
	}
	void rotateLocalY(F32 angDegrees)
	{
		m_ltrf.getRotation().rotateYAxis(angDegrees);
		markForUpdate();
	}
	void rotateLocalZ(F32 angDegrees)
	{
		m_ltrf.getRotation().rotateZAxis(angDegrees);
		markForUpdate();
	}
	void moveLocalX(F32 distance)
	{
		Vec3 x_axis = m_ltrf.getRotation().getColumn(0);
		m_ltrf.getOrigin() += Vec4(x_axis, 0.0) * distance;
		markForUpdate();
	}
	void moveLocalY(F32 distance)
	{
		Vec3 y_axis = m_ltrf.getRotation().getColumn(1);
		m_ltrf.getOrigin() += Vec4(y_axis, 0.0) * distance;
		markForUpdate();
	}
	void moveLocalZ(F32 distance)
	{
		Vec3 z_axis = m_ltrf.getRotation().getColumn(2);
		m_ltrf.getOrigin() += Vec4(z_axis, 0.0) * distance;
		markForUpdate();
	}
	void scale(F32 s)
	{
		m_ltrf.getScale() *= s;
		markForUpdate();
	}

	void lookAtPoint(const Vec4& point)
	{
		m_ltrf.lookAt(point, Vec4(0.0f, 1.0f, 0.0f, 0.0f));
		markForUpdate();
	}
	/// @}

private:
	/// The transformation in local space
	Transform m_ltrf = Transform::getIdentity();

	/// The transformation in world space (local combined with parent's transformation)
	Transform m_wtrf = Transform::getIdentity();

	/// Keep the previous transformation for checking if it moved
	Transform m_prevWTrf = Transform::getIdentity();

	BitMask<MoveComponentFlag> m_flags;

	void markForUpdate()
	{
		m_flags.set(MoveComponentFlag::MARKED_FOR_UPDATE);
	}

	/// Called every frame. It updates the @a m_wtrf if @a shouldUpdateWTrf is true. Then it moves to the children.
	Bool updateWorldTransform(SceneNode& node);
};
/// @}

} // end namespace anki
