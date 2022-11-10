// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Math.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Interface for movable scene nodes.
class MoveComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MoveComponent)

public:
	MoveComponent(SceneNode* node);

	~MoveComponent();

	/// Get the parent's world transform.
	void setIgnoreLocalTransform(Bool ignore)
	{
		m_ignoreLocalTransform = ignore;
	}

	/// Ignore parent nodes's transform.
	void setIgnoreParentTransform(Bool ignore)
	{
		m_ignoreParentTransform = ignore;
	}

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

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(F32 angleRad)
	{
		m_ltrf.getRotation().rotateXAxis(angleRad);
		markForUpdate();
	}
	void rotateLocalY(F32 angleRad)
	{
		m_ltrf.getRotation().rotateYAxis(angleRad);
		markForUpdate();
	}
	void rotateLocalZ(F32 angleRad)
	{
		m_ltrf.getRotation().rotateZAxis(angleRad);
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

	SceneNode* m_node = nullptr;
	SegregatedListsGpuAllocatorToken m_gpuSceneTransforms;

	Bool m_markedForUpdate : 1;
	Bool m_ignoreLocalTransform : 1;
	Bool m_ignoreParentTransform : 1;

	void markForUpdate()
	{
		m_markedForUpdate = true;
	}

	/// Called every frame. It updates the @a m_wtrf if @a shouldUpdateWTrf is true. Then it moves to the children.
	Bool updateWorldTransform(SceneNode& node);

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	void onDestroy(SceneNode& node);
};
/// @}

} // end namespace anki
