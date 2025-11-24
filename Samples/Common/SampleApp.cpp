// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/Common/SampleApp.h>

using namespace anki;

Error SampleApp::userPreInit()
{
	// Init the super class
	g_cvarWindowFullscreen = 1;

#if !ANKI_OS_ANDROID
	String assetsDataPath;
	assetsDataPath.sprintf("%s/Samples/%s", ANKI_SOURCE_DIRECTORY, getApplicationName().cstr());

	if(!directoryExists(assetsDataPath))
	{
		ANKI_LOGE("Cannot find directory \"%s\". Have you moved the clone of the repository? I'll continue but expect issues", assetsDataPath.cstr());
	}
	else
	{
		g_cvarRsrcDataPaths = String().sprintf("%s|.anki,lua", assetsDataPath.cstr());
	}
#endif

	ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 1, m_argv + 1));

	return Error::kNone;
}

Error SampleApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	constexpr F32 ROTATE_ANGLE = toRad(2.5f);
	constexpr F32 MOUSE_SENSITIVITY = 5.0f;
	quit = false;

	SceneGraph& scene = SceneGraph::getSingleton();
	Renderer& renderer = Renderer::getSingleton();
	Input& in = Input::getSingleton();

	if(in.getKey(KeyCode::kEscape) > 0)
	{
		quit = true;
		return Error::kNone;
	}

	if(in.getKey(KeyCode::kGrave) == 1)
	{
		toggleDeveloperConsole();
	}

	if(getDeveloperConsoleEnabled())
	{
		return Error::kNone;
	}

	if(in.getKey(KeyCode::kY) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "IndirectDiffuseClipmapsTest") ? "" : "IndirectDiffuseClipmapsTest");
		// g_shadowMappingPcssCVar = !g_shadowMappingPcssCVar;
	}

	if(in.getKey(KeyCode::kU) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "Reflections") ? "" : "Reflections");
	}

	if(in.getKey(KeyCode::kI) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "RtMaterialFetchDbg") ? "" : "RtMaterialFetchDbg");
	}

	if(in.getKey(KeyCode::kO) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "HistoryLen") ? "" : "HistoryLen");
	}

	static Bool timeOfDay = false;
	if(in.getKey(KeyCode::kP) == 1 || timeOfDay)
	{
		timeOfDay = true;
		static F32 time = 8.0f;
		scene.getDirectionalLight()->setDirectionFromTimeOfDay(6, 25, time);
		time += 0.2f * F32(elapsedTime);
		if(time > 19.0)
		{
			time = 8.0f;
		}
	}

	if(in.getKey(KeyCode::kL) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "MotionBlur") ? "" : "MotionBlur");
	}

	if(in.getKey(KeyCode::kH) == 1)
	{
		static U32 pressCount = 0;
		CString rtName;
		switch(pressCount)
		{
		case 0:
			rtName = "RtShadows";
			break;
		case 1:
			rtName = "RtShadows1";
			break;
		case 2:
			rtName = "RtShadows2";
			break;
		default:
			rtName = "";
		}
		renderer.setCurrentDebugRenderTarget(rtName);

		pressCount = (pressCount + 1) % 4;
	}

	if(in.getKey(KeyCode::kJ) == 1)
	{
		g_cvarGrVrs = !g_cvarGrVrs;
	}

	static Vec2 mousePosOn1stClick = in.getMousePositionNdc();
	if(in.getMouseButton(MouseButton::kRight) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = in.getMousePositionNdc();
	}

	if(in.getKey(KeyCode::kF1) == 1)
	{
		DbgOption options = renderer.getDbg().getOptions();

		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			options &= ~DbgOption::kBoundingBoxes;
		}
		else if(mode == 1)
		{
			options |= DbgOption::kBoundingBoxes;
			options |= DbgOption::kDepthTest;
		}
		else
		{
			options |= DbgOption::kBoundingBoxes;
			options &= ~DbgOption::kDepthTest;
		}

		renderer.getDbg().setOptions(options);
	}

	if(in.getKey(KeyCode::kF11) == 1 && ANKI_TRACING_ENABLED)
	{
		g_cvarCoreTracingEnabled = !g_cvarCoreTracingEnabled;
	}

	if(in.getMouseButton(MouseButton::kRight) > 0 || in.hasTouchDevice())
	{
		in.hideCursor(true);

		// move the camera
		static SceneNode* mover = &scene.getActiveCameraNode();

		if(in.getKey(KeyCode::k1) == 1)
		{
			mover = &scene.getActiveCameraNode();
		}

		if(in.getKey(KeyCode::k2) == 1)
		{
			mover = &scene.findSceneNode("Spot");
		}

		if(in.getKey(KeyCode::kUp) > 0)
		{
			mover->rotateLocalX(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::kDown) > 0)
		{
			mover->rotateLocalX(-ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::kLeft) > 0)
		{
			mover->rotateLocalY(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::kRight) > 0)
		{
			mover->rotateLocalY(-ROTATE_ANGLE);
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
			angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.z() = 0.0f;
			mover->setLocalRotation(Mat3(angles));
		}

		static TouchPointer rotateCameraTouch = TouchPointer::kCount;
		static Vec2 rotateEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(rotateCameraTouch == TouchPointer::kCount && in.getTouchPointer(touch) == 1 && in.getTouchPointerNdcPosition(touch).x() > 0.1f)
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
			angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.z() = 0.0f;
			mover->setLocalRotation(Mat3(angles));
		}

		static TouchPointer moveCameraTouch = TouchPointer::kCount;
		static Vec2 moveEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(moveCameraTouch == TouchPointer::kCount && in.getTouchPointer(touch) == 1 && in.getTouchPointerNdcPosition(touch).x() < -0.1f)
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
			velocity *= 2.0f;

			mover->moveLocalX(moveDistance * velocity.x());
			mover->moveLocalZ(moveDistance * -velocity.y());
		}
	}
	else
	{
		in.hideCursor(false);
	}

	return Error::kNone;
}
