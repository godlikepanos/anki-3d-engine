// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class MyApp : public App
{
public:
	MyApp(U32 argc, Char** argv)
		: App("Sandbox", argc, argv)
	{
	}

	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

Error MyApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	quit = false;

	SceneGraph& scene = SceneGraph::getSingleton();
	Input& in = Input::getSingleton();

	if(scene.isPaused())
	{
		// Editor running
		return Error::kNone;
	}

	// move the camera
	static SceneNode* mover = &scene.getActiveCameraNode();

#if ANKI_TRACING_ENABLED
	if(in.getKey(KeyCode::kF11) == 1)
	{
		g_cvarCoreTracingEnabled = !g_cvarCoreTracingEnabled;
	}
#endif

	static Vec2 mousePosOn1stClick = in.getMousePositionNdc();
	if(in.getMouseButton(MouseButton::kRight) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = in.getMousePositionNdc();
	}

	if(in.getMouseButton(MouseButton::kRight) > 0 || in.hasTouchDevice())
	{
		constexpr F32 kRotateAngle = toRad(2.5f);
		constexpr F32 kMouseSensitivity = 5.0f;

		in.hideMouseCursor(true);

		if(in.getKey(KeyCode::kUp) > 0)
		{
			mover->rotateLocalX(kRotateAngle);
		}

		if(in.getKey(KeyCode::kDown) > 0)
		{
			mover->rotateLocalX(-kRotateAngle);
		}

		if(in.getKey(KeyCode::kLeft) > 0)
		{
			mover->rotateLocalY(kRotateAngle);
		}

		if(in.getKey(KeyCode::kRight) > 0)
		{
			mover->rotateLocalY(-kRotateAngle);
		}

		static F32 moveDistance = 0.1f;
		if(in.getMouseButton(MouseButton::kScrollUp) == 1)
		{
			moveDistance += 0.1f;
			moveDistance = min(moveDistance, 10.0f);
		}

		if(in.getMouseButton(MouseButton::kScrollDown) == 1)
		{
			moveDistance -= 0.1f;
			moveDistance = max(moveDistance, 0.1f);
		}

		if(in.getKey(KeyCode::kA) > 0)
		{
			mover->moveLocalX(-moveDistance);
		}

		if(in.getKey(KeyCode::kD) > 0)
		{
			mover->moveLocalX(moveDistance);
		}

		if(in.getKey(KeyCode::kQ) > 0)
		{
			mover->moveLocalY(-moveDistance);
		}

		if(in.getKey(KeyCode::kE) > 0)
		{
			mover->moveLocalY(moveDistance);
		}

		if(in.getKey(KeyCode::kW) > 0)
		{
			mover->moveLocalZ(-moveDistance);
		}

		if(in.getKey(KeyCode::kS) > 0)
		{
			mover->moveLocalZ(moveDistance);
		}

		const Vec2 velocity = in.getMousePositionNdc() - mousePosOn1stClick;
		in.moveMouseNdc(mousePosOn1stClick);
		if(velocity != Vec2(0.0))
		{
			Euler angles(mover->getLocalRotation().getRotationPart());
			angles.x += velocity.y * toRad(360.0f) * F32(elapsedTime) * kMouseSensitivity;
			angles.x = clamp(angles.x, toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y += -velocity.x * toRad(360.0f) * F32(elapsedTime) * kMouseSensitivity;
			angles.z = 0.0f;
			mover->setLocalRotation(Mat3(angles));
		}

		static TouchPointer rotateCameraTouch = TouchPointer::kCount;
		static Vec2 rotateEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(rotateCameraTouch == TouchPointer::kCount && in.getTouchPointer(touch) == 1 && in.getTouchPointerNdcPosition(touch).x > 0.1f)
			{
				rotateCameraTouch = touch;
				rotateEventInitialPos = in.getTouchPointerNdcPosition(touch) * NativeWindow::getSingleton().getAspectRatio();
				break;
			}
		}

		if(rotateCameraTouch != TouchPointer::kCount && in.getTouchPointer(rotateCameraTouch) == 0)
		{
			rotateCameraTouch = TouchPointer::kCount;
		}

		if(rotateCameraTouch != TouchPointer::kCount && in.getTouchPointer(rotateCameraTouch) > 1)
		{
			Vec2 velocity = in.getTouchPointerNdcPosition(rotateCameraTouch) * NativeWindow::getSingleton().getAspectRatio() - rotateEventInitialPos;
			velocity *= 0.3f;

			Euler angles(mover->getLocalRotation().getRotationPart());
			angles.x += velocity.y * toRad(360.0f) * F32(elapsedTime) * kMouseSensitivity;
			angles.x = clamp(angles.x, toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y += -velocity.x * toRad(360.0f) * F32(elapsedTime) * kMouseSensitivity;
			angles.z = 0.0f;
			mover->setLocalRotation(Mat3(angles));
		}

		static TouchPointer moveCameraTouch = TouchPointer::kCount;
		static Vec2 moveEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(moveCameraTouch == TouchPointer::kCount && in.getTouchPointer(touch) == 1 && in.getTouchPointerNdcPosition(touch).x < -0.1f)
			{
				moveCameraTouch = touch;
				moveEventInitialPos = in.getTouchPointerNdcPosition(touch) * NativeWindow::getSingleton().getAspectRatio();
				break;
			}
		}

		if(moveCameraTouch != TouchPointer::kCount && in.getTouchPointer(moveCameraTouch) == 0)
		{
			moveCameraTouch = TouchPointer::kCount;
		}

		if(moveCameraTouch != TouchPointer::kCount && in.getTouchPointer(moveCameraTouch) > 0)
		{
			Vec2 velocity = in.getTouchPointerNdcPosition(moveCameraTouch) * NativeWindow::getSingleton().getAspectRatio() - moveEventInitialPos;
			velocity *= 3.0f;

			mover->moveLocalX(moveDistance * velocity.x);
			mover->moveLocalZ(moveDistance * -velocity.y);
		}
	}
	else
	{
		in.hideMouseCursor(false);
	}

	if(in.getEvent(InputEvent::kWindowClosed))
	{
		quit = true;
	}

	return Error::kNone;
}

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	MyApp* app = new MyApp(argc, argv);
	const Error err = app->mainLoop();
	delete app;

	if(err)
	{
		ANKI_LOGE("Error reported. See previous messages");
	}
	else
	{
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
