// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class Camera : public SceneNode
{
	friend class CameraMoveFeedbackComponent;
	friend class CameraFrustumFeedbackComponent;

public:
	/// @note Don't EVER change the order
	enum class Type : U8
	{
		PERSPECTIVE,
		ORTHOGRAPHIC,
		COUNT
	};

	Camera(SceneGraph* scene, Type type, CString name);

	virtual ~Camera();

	ANKI_USE_RESULT Error init(Frustum* frustum);

	Type getCameraType() const
	{
		return m_type;
	}

	void lookAtPoint(const Vec3& point);

private:
	Type m_type;

	/// Called when moved.
	void onMoveComponentUpdate(MoveComponent& move);

	/// Called when something changed in the frustum.
	void onFrustumComponentUpdate(FrustumComponent& fr);
};

/// Perspective camera
class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(SceneGraph* scene, CString name);

	~PerspectiveCamera();

	ANKI_USE_RESULT Error init()
	{
		return Camera::init(&m_frustum);
	}

	void setAll(F32 fovX, F32 fovY, F32 near, F32 far);

private:
	PerspectiveFrustum m_frustum;
};

/// Orthographic camera
class OrthographicCamera : public Camera
{
public:
	OrthographicCamera(SceneGraph* scene, CString name);

	~OrthographicCamera();

	ANKI_USE_RESULT Error init()
	{
		return Camera::init(&m_frustum);
	}

private:
	OrthographicFrustum m_frustum;
};
/// @}

} // end namespace anki
