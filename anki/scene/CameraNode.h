// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/collision/Common.h>

namespace anki
{

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class CameraNode : public SceneNode
{
public:
	CameraNode(SceneGraph* scene, CString name);

	~CameraNode();

protected:
	ANKI_USE_RESULT Error init(FrustumType frustumType);

private:
	class MoveFeedbackComponent;
	class FrustumFeedbackComponent;

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
		return CameraNode::init(FrustumType::PERSPECTIVE);
	}
};

/// Orthographic camera
class OrthographicCameraNode : public CameraNode
{
public:
	OrthographicCameraNode(SceneGraph* scene, CString name);

	~OrthographicCameraNode();

	ANKI_USE_RESULT Error init()
	{
		return CameraNode::init(FrustumType::ORTHOGRAPHIC);
	}
};
/// @}

} // end namespace anki
