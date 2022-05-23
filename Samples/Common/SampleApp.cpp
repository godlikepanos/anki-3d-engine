// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/Common/SampleApp.h>

using namespace anki;

Error SampleApp::init(int argc, char** argv, CString sampleName)
{
	HeapAllocator<U32> alloc(allocAligned, nullptr);

	// Init the super class
	m_config.init(allocAligned, nullptr);
	m_config.setWindowFullscreen(true);

#if !ANKI_OS_ANDROID
	StringAuto mainDataPath(alloc, ANKI_SOURCE_DIRECTORY);
	StringAuto assetsDataPath(alloc);
	assetsDataPath.sprintf("%s/Samples/%s", ANKI_SOURCE_DIRECTORY, sampleName.cstr());

	if(!directoryExists(assetsDataPath))
	{
		ANKI_LOGE("Cannot find directory \"%s\". Have you moved the clone of the repository? "
				  "I'll continue but expect issues",
				  assetsDataPath.cstr());
	}
	else
	{
		m_config.setRsrcDataPaths(StringAuto(alloc).sprintf("%s:%s", mainDataPath.cstr(), assetsDataPath.cstr()));
	}
#endif

	ANKI_CHECK(m_config.setFromCommandLineArguments(argc - 1, argv + 1));
	ANKI_CHECK(App::init(&m_config, argv[0], allocAligned, nullptr));

	ANKI_CHECK(sampleExtraInit());

	return Error::NONE;
}

Error SampleApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	constexpr F32 ROTATE_ANGLE = toRad(2.5f);
	constexpr F32 MOUSE_SENSITIVITY = 5.0f;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();
	Input& in = getInput();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return Error::NONE;
	}

	if(in.getKey(KeyCode::BACKQUOTE) == 1)
	{
		setDisplayDeveloperConsole(!getDisplayDeveloperConsole());
	}

	if(in.getKey(KeyCode::Y) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "GBufferNormals") ? "" : "GBufferNormals");
	}

	if(in.getKey(KeyCode::U) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "IndirectDiffuse") ? "" : "IndirectDiffuse");
	}

	if(in.getKey(KeyCode::I) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "SSR") ? "" : "SSR");
	}

	if(in.getKey(KeyCode::O) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "SM_resolve") ? ""
																									  : "SM_resolve");
	}

	if(in.getKey(KeyCode::P) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "VRS") ? "" : "VRS");
	}

	if(in.getKey(KeyCode::L) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "MotionVectorsHistoryLength")
												 ? ""
												 : "MotionVectorsHistoryLength");
	}

	if(in.getKey(KeyCode::H) == 1)
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

	if(in.getKey(KeyCode::J) == 1)
	{
		m_config.setRVrs(!m_config.getRVrs());
	}

	static Vec2 mousePosOn1stClick = in.getMousePosition();
	if(in.getMouseButton(MouseButton::RIGHT) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = in.getMousePosition();
	}

	if(in.getMouseButton(MouseButton::RIGHT) || in.hasTouchDevice())
	{
		in.hideCursor(true);

		// move the camera
		static MoveComponent* mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();

		if(in.getKey(KeyCode::_1) == 1)
		{
			mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();
		}

		if(in.getKey(KeyCode::F1) == 1)
		{
			static U mode = 0;
			mode = (mode + 1) % 3;
			if(mode == 0)
			{
				getConfig().setRDbgEnabled(false);
			}
			else if(mode == 1)
			{
				getConfig().setRDbgEnabled(true);
				renderer.getDbg().setDepthTestEnabled(true);
				renderer.getDbg().setDitheredDepthTestEnabled(false);
			}
			else
			{
				getConfig().setRDbgEnabled(true);
				renderer.getDbg().setDepthTestEnabled(false);
				renderer.getDbg().setDitheredDepthTestEnabled(true);
			}
		}
		if(in.getKey(KeyCode::F2) == 1)
		{
			// renderer.getDbg().flipFlags(DbgFlag::SPATIAL_COMPONENT);
		}

		if(in.getKey(KeyCode::UP))
		{
			mover->rotateLocalX(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::DOWN))
		{
			mover->rotateLocalX(-ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::LEFT))
		{
			mover->rotateLocalY(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::RIGHT))
		{
			mover->rotateLocalY(-ROTATE_ANGLE);
		}

		static F32 moveDistance = 0.1f;
		if(in.getMouseButton(MouseButton::SCROLL_UP) == 1)
		{
			moveDistance += 0.1f;
			moveDistance = min(moveDistance, 10.0f);
		}

		if(in.getMouseButton(MouseButton::SCROLL_DOWN) == 1)
		{
			moveDistance -= 0.1f;
			moveDistance = max(moveDistance, 0.1f);
		}

		if(in.getKey(KeyCode::A))
		{
			mover->moveLocalX(-moveDistance);
		}

		if(in.getKey(KeyCode::D))
		{
			mover->moveLocalX(moveDistance);
		}

		if(in.getKey(KeyCode::Q))
		{
			mover->moveLocalY(-moveDistance);
		}

		if(in.getKey(KeyCode::E))
		{
			mover->moveLocalY(moveDistance);
		}

		if(in.getKey(KeyCode::W))
		{
			mover->moveLocalZ(-moveDistance);
		}

		if(in.getKey(KeyCode::S))
		{
			mover->moveLocalZ(moveDistance);
		}

		if(in.getKey(KeyCode::F12) == 1 && ANKI_ENABLE_TRACE)
		{
			TracerSingleton::get().setEnabled(!TracerSingleton::get().getEnabled());
		}

		const Vec2 velocity = in.getMousePosition() - mousePosOn1stClick;
		in.moveCursor(mousePosOn1stClick);
		if(velocity != Vec2(0.0))
		{
			Euler angles(mover->getLocalRotation().getRotationPart());
			angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.z() = 0.0f;
			mover->setLocalRotation(Mat3x4(Vec3(0.0f), angles));
		}

		static TouchPointer rotateCameraTouch = TouchPointer::COUNT;
		static Vec2 rotateEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(rotateCameraTouch == TouchPointer::COUNT && in.getTouchPointer(touch) == 1
			   && in.getTouchPointerNdcPosition(touch).x() > 0.1f)
			{
				rotateCameraTouch = touch;
				rotateEventInitialPos = in.getTouchPointerNdcPosition(touch) * getWindow().getAspectRatio();
				break;
			}
		}

		if(rotateCameraTouch != TouchPointer::COUNT && in.getTouchPointer(rotateCameraTouch) == 0)
		{
			rotateCameraTouch = TouchPointer::COUNT;
		}

		if(rotateCameraTouch != TouchPointer::COUNT && in.getTouchPointer(rotateCameraTouch) > 1)
		{
			Vec2 velocity =
				in.getTouchPointerNdcPosition(rotateCameraTouch) * getWindow().getAspectRatio() - rotateEventInitialPos;
			velocity *= 0.3f;

			Euler angles(mover->getLocalRotation().getRotationPart());
			angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.z() = 0.0f;
			mover->setLocalRotation(Mat3x4(Vec3(0.0f), angles));
		}

		static TouchPointer moveCameraTouch = TouchPointer::COUNT;
		static Vec2 moveEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(moveCameraTouch == TouchPointer::COUNT && in.getTouchPointer(touch) == 1
			   && in.getTouchPointerNdcPosition(touch).x() < -0.1f)
			{
				moveCameraTouch = touch;
				moveEventInitialPos = in.getTouchPointerNdcPosition(touch) * getWindow().getAspectRatio();
				break;
			}
		}

		if(moveCameraTouch != TouchPointer::COUNT && in.getTouchPointer(moveCameraTouch) == 0)
		{
			moveCameraTouch = TouchPointer::COUNT;
		}

		if(moveCameraTouch != TouchPointer::COUNT && in.getTouchPointer(moveCameraTouch) > 0)
		{
			Vec2 velocity =
				in.getTouchPointerNdcPosition(moveCameraTouch) * getWindow().getAspectRatio() - moveEventInitialPos;
			velocity *= 2.0f;

			mover->moveLocalX(moveDistance * velocity.x());
			mover->moveLocalZ(moveDistance * -velocity.y());
		}
	}
	else
	{
		in.hideCursor(false);
	}

	return Error::NONE;
}
