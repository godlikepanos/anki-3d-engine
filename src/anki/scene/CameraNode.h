// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/Math.h>
#include <anki/collision/Frustum.h>

namespace anki
{

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class CameraNode : public SceneNode
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

	CameraNode(SceneGraph* scene, Type type, CString name);

	virtual ~CameraNode();

	ANKI_USE_RESULT Error init(Frustum* frustum);

	Type getCameraType() const
	{
		return m_type;
	}

	void lookAtPoint(const Vec3& point);

private:
	class MoveFeedbackComponent;
	class FrustumFeedbackComponent;

	Type m_type;

	/// Called when moved.
	void onMoveComponentUpdate(MoveComponent& move);

	/// Called when something changed in the frustum.
	void onFrustumComponentUpdate(FrustumComponent& fr);
};

/// Perspective camera
class PerspectiveCameraNode : public CameraNode
{
public:
	PerspectiveCameraNode(SceneGraph* scene, CString name);

	~PerspectiveCameraNode();

	ANKI_USE_RESULT Error init()
	{
		return CameraNode::init(&m_frustum);
	}

	void setAll(F32 fovX, F32 fovY, F32 near, F32 far);

private:
	PerspectiveFrustum m_frustum;
};

/// Orthographic camera
class OrthographicCameraNode : public CameraNode
{
public:
	OrthographicCameraNode(SceneGraph* scene, CString name);

	~OrthographicCameraNode();

	ANKI_USE_RESULT Error init()
	{
		return CameraNode::init(&m_frustum);
	}

private:
	OrthographicFrustum m_frustum;
};
/// @}

} // end namespace anki
