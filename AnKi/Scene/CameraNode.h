// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Collision/Common.h>

namespace anki {

/// @addtogroup Scene
/// @{

/// Camera SceneNode interface class
class CameraNode : public SceneNode
{
public:
	CameraNode(SceneGraph* scene, CString name);

	~CameraNode();

protected:
	void initCommon(FrustumType frustumType);

private:
	class MoveFeedbackComponent;

	/// Called when moved.
	void onMoveComponentUpdate(MoveComponent& move);
};

/// Perspective camera
class PerspectiveCameraNode : public CameraNode
{
public:
	PerspectiveCameraNode(SceneGraph* scene, CString name);

	~PerspectiveCameraNode();
};

/// Orthographic camera
class OrthographicCameraNode : public CameraNode
{
public:
	OrthographicCameraNode(SceneGraph* scene, CString name);

	~OrthographicCameraNode();
};
/// @}

} // end namespace anki
