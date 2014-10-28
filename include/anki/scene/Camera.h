// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_CAMERA_H
#define ANKI_SCENE_CAMERA_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/FrustumComponent.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class Camera: public SceneNode, public MoveComponent, public FrustumComponent,
	public SpatialComponent
{
public:
	/// @note Don't EVER change the order
	enum class Type: U8
	{
		PERSPECTIVE,
		ORTHOGRAPHIC,
		COUNT
	};

	Camera(SceneGraph* scene, Type type, Frustum* frustum);

	virtual ~Camera();

	ANKI_USE_RESULT Error create(const CString& name);

	Type getCameraType() const
	{
		return m_type;
	}

	/// Needed by the renderer
	F32 getNear() const
	{
		return getFrustum().getNear();
	}

	/// Needed by the renderer
	F32 getFar() const
	{
		return getFrustum().getFar();
	}

	/// @name MoveComponent virtuals
	/// @{
	ANKI_USE_RESULT Error onMoveComponentUpdate(
		SceneNode& node, F32 prevTime, F32 crntTime) override;
	/// @}

	/// @name SpatialComponent virtuals
	/// @{
	Vec4 getSpatialOrigin()
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

	void lookAtPoint(const Vec3& point);

protected:
	/// Called when something changes in the frustum
	void frustumUpdate();

private:
	Type m_type;
};

/// Perspective camera
class PerspectiveCamera: public Camera
{
public:
	PerspectiveCamera(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name)
	{
		return Camera::create(name);
	}

	F32 getFovX() const
	{
		return m_frustum.getFovX();
	}
	void setFovX(F32 x)
	{
		m_frustum.setFovX(x);
		frustumUpdate();
	}

	F32 getFovY() const
	{
		return m_frustum.getFovY();
	}
	void setFovY(F32 x)
	{
		m_frustum.setFovY(x);
		frustumUpdate();
	}

	void setAll(F32 fovX_, F32 fovY_, F32 near_, F32 far_)
	{
		m_frustum.setAll(fovX_, fovY_, near_, far_);
		frustumUpdate();
	}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return m_frustum;
	}
	/// @}

private:
	PerspectiveFrustum m_frustum;
};

/// Orthographic camera
class OrthographicCamera: public Camera
{
public:
	OrthographicCamera(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name)
	{
		return Camera::create(name);
	}

	F32 getNear() const
	{
		return m_frustum.getNear();
	}

	F32 getFar() const
	{
		return m_frustum.getFar();
	}

	F32 getLeft() const
	{
		return m_frustum.getLeft();
	}

	F32 getRight() const
	{
		return m_frustum.getRight();
	}

	F32 getBottom() const
	{
		return m_frustum.getBottom();
	}

	F32 getTop() const
	{
		return m_frustum.getTop();
	}

	void setAll(F32 left, F32 right, F32 near, F32 far, F32 top, F32 bottom)
	{
		m_frustum.setAll(left, right, near, far, top, bottom);
		frustumUpdate();
	}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return m_frustum;
	}
	/// @}

private:
	OrthographicFrustum m_frustum;
};
/// @}

} // end namespace anki

#endif
