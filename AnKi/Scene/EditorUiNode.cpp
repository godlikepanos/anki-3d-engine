// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/EditorUiNode.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

void EditorUiNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	getFirstComponentOfType<UiComponent>().setEnabled(g_cvarCoreShowEditor);

	if(!g_cvarCoreShowEditor)
	{
		return;
	}

	Input& in = Input::getSingleton();

	if(m_editorUi.m_quit)
	{
		in.addEvent(InputEvent::kWindowClosed);
	}

	static Vec2 mousePosOn1stClick = in.getMousePositionNdc();
	if(in.getMouseButton(MouseButton::kRight) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = in.getMousePositionNdc();
	}

	if(in.getMouseButton(MouseButton::kRight) > 0 && !m_editorUi.m_mouseOverAnyWindow)
	{
		in.hideCursor(true);

		// move the camera
		SceneNode& mover = SceneGraph::getSingleton().getActiveCameraNode();

		constexpr F32 kRotateAngle = toRad(2.5f);
		constexpr F32 kMouseSensitivity = 5.0f;

		if(in.getKey(KeyCode::kUp) > 0)
		{
			mover.rotateLocalX(kRotateAngle);
		}

		if(in.getKey(KeyCode::kDown) > 0)
		{
			mover.rotateLocalX(-kRotateAngle);
		}

		if(in.getKey(KeyCode::kLeft) > 0)
		{
			mover.rotateLocalY(kRotateAngle);
		}

		if(in.getKey(KeyCode::kRight) > 0)
		{
			mover.rotateLocalY(-kRotateAngle);
		}

		F32 moveDistance = 0.1f;
		if(in.getKey(KeyCode::kLeftShift) > 0)
		{
			moveDistance *= 4.0f;
		}

		if(in.getKey(KeyCode::kA) > 0)
		{
			mover.moveLocalX(-moveDistance);
		}

		if(in.getKey(KeyCode::kD) > 0)
		{
			mover.moveLocalX(moveDistance);
		}

		if(in.getKey(KeyCode::kQ) > 0)
		{
			mover.moveLocalY(-moveDistance);
		}

		if(in.getKey(KeyCode::kE) > 0)
		{
			mover.moveLocalY(moveDistance);
		}

		if(in.getKey(KeyCode::kW) > 0)
		{
			mover.moveLocalZ(-moveDistance);
		}

		if(in.getKey(KeyCode::kS) > 0)
		{
			mover.moveLocalZ(moveDistance);
		}

		const Vec2 velocity = in.getMousePositionNdc() - mousePosOn1stClick;
		in.moveMouseNdc(mousePosOn1stClick);
		if(velocity != Vec2(0.0))
		{
			const Second dt = crntTime - prevUpdateTime;
			Euler angles(mover.getLocalRotation().getRotationPart());
			angles.x += velocity.y * toRad(360.0f) * F32(dt) * kMouseSensitivity;
			angles.x = clamp(angles.x, toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y += -velocity.x * toRad(360.0f) * F32(dt) * kMouseSensitivity;
			angles.z = 0.0f;
			mover.setLocalRotation(Mat3(angles));
		}
	}
	else
	{
		in.hideCursor(false);
	}
}

} // end namespace anki
