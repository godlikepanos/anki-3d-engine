// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include <AnKi/AnKi.h>

using namespace anki;

#define PLAYER 0
#define MOUSE 1

class MyApp : public App
{
public:
	Bool m_profile = false;
	U32 m_argc = 0;
	Char** m_argv = nullptr;

	MyApp(U32 argc, Char** argv)
		: App("Sandbox")
		, m_argc(argc)
		, m_argv(argv)
	{
	}

	Error userPreInit() override;
	Error userPostInit() override;
	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

Error MyApp::userPreInit()
{
#if !ANKI_OS_ANDROID
	if(m_argc < 2)
	{
		ANKI_LOGE("usage: %s relative/path/to/scene.lua [anki config options]", m_argv[0]);
		return Error::kUserData;
	}
#endif

	// Config
#if ANKI_OS_ANDROID
	ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 1, m_argv + 1));
#else
	ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 2, m_argv + 2));
#endif

	return Error::kNone;
}

Error MyApp::userPostInit()
{
	// Other init
	ResourceManager& resources = ResourceManager::getSingleton();

	if(getenv("PROFILE"))
	{
		m_profile = true;
		g_cvarCoreTargetFps = 240;
#if ANKI_TRACING_ENABLED
		g_cvarCoreTracingEnabled = true;
#endif
	}

	// Load scene
	ScriptResourcePtr script;
#if ANKI_OS_ANDROID
	ANKI_CHECK(resources.loadResource("Assets/Scene.lua", script));
#else
	ANKI_CHECK(resources.loadResource(m_argv[1], script));
#endif
	ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

	// ANKI_CHECK(renderer.getFinalComposite().loadColorGradingTexture(
	//	"textures/color_gradient_luts/forge_lut.ankitex"));

#if PLAYER
	SceneGraph& scene = getSceneGraph();
	SceneNode& cam = scene.getActiveCameraNode();

	PlayerNode* pnode;
	ANKI_CHECK(
		scene.newSceneNode<PlayerNode>("player", pnode, cam.getFirstComponentOfType<MoveComponent>().getLocalOrigin() - Vec4(0.0, 1.0, 0.0, 0.0)));

	cam.getFirstComponentOfType<MoveComponent>().setLocalTransform(Transform(Vec4(0.0, 0.0, 0.0, 0.0), Mat3x4::getIdentity(), 1.0));

	pnode->addChild(&cam);
#endif

	return Error::kNone;
}

Error MyApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	quit = false;

	SceneGraph& scene = SceneGraph::getSingleton();
	Input& in = Input::getSingleton();
	Renderer& renderer = Renderer::getSingleton();

	if(in.getKey(KeyCode::kEscape) > 0)
	{
		quit = true;
		return Error::kNone;
	}

	// move the camera
	static SceneNode* mover = &scene.getActiveCameraNode();

	if(in.getKey(KeyCode::k1) > 0)
	{
		mover = &scene.getActiveCameraNode();
	}
	if(in.getKey(KeyCode::k2) > 0)
	{
		mover = &scene.findSceneNode("Point.018_Orientation");
	}

	if(in.getKey(KeyCode::kL) == 1)
	{
		const Vec3 origin = mover->getWorldTransform().getOrigin().xyz;
		mover->setLocalOrigin(origin + Vec3(0, 15, 0));
	}

#if ANKI_TRACING_ENABLED
	if(in.getKey(KeyCode::kF11) == 1)
	{
		g_cvarCoreTracingEnabled = !g_cvarCoreTracingEnabled;
	}
#endif

#if !PLAYER
	static Vec2 mousePosOn1stClick = in.getMousePositionNdc();
	if(in.getMouseButton(MouseButton::kRight) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = in.getMousePositionNdc();
	}

	if(in.getMouseButton(MouseButton::kRight) > 0 || in.hasTouchDevice())
	{
		constexpr F32 ROTATE_ANGLE = toRad(2.5f);
		constexpr F32 MOUSE_SENSITIVITY = 5.0f;

		in.hideCursor(true);

		if(in.getKey(KeyCode::k1) == 1)
		{
			mover = &scene.getActiveCameraNode();
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
			angles.x += velocity.y * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x = clamp(angles.x, toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y += -velocity.x * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
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
			angles.x += velocity.y * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x = clamp(angles.x, toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y += -velocity.x * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
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
		in.hideCursor(false);
	}
#endif

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
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "IndirectDiffuse") ? "" : "IndirectDiffuse");
	}

	if(in.getKey(KeyCode::kO) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "RtMaterialFetchDbg") ? "" : "RtMaterialFetchDbg");
	}

	/*if(in.getKey(KeyCode::J) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "MotionVectorsHistoryLength")
												 ? ""
												 : "MotionVectorsHistoryLength");
	}*/

	if(in.getKey(KeyCode::kP) == 1)
	{
		static U32 idx = 3;
		++idx;
		idx %= 4;
		if(idx == 0)
		{
			renderer.setCurrentDebugRenderTarget("IndirectDiffuseVrsSri");
		}
		else if(idx == 1)
		{
			renderer.setCurrentDebugRenderTarget("VrsSriDownscaled");
		}
		else if(idx == 2)
		{
			renderer.setCurrentDebugRenderTarget("VrsSri");
		}
		else
		{
			renderer.setCurrentDebugRenderTarget("");
		}
	}

	if(in.getKey(KeyCode::kJ) == 1)
	{
		g_cvarGrVrs = !g_cvarGrVrs;
	}

	if(in.getEvent(InputEvent::kWindowClosed))
	{
		quit = true;
	}

	if(m_profile && GlobalFrameIndex::getSingleton().m_value == 1000)
	{
		quit = true;
		return Error::kNone;
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
