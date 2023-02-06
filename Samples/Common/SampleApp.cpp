// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/Common/SampleApp.h>
#include <AnKi/Scene/Events/AnimationEvent.h>

using namespace anki;

class ButtonsUiNode : public SceneNode
{
public:
	FontPtr m_font;
	Renderer* m_renderer = nullptr;
	ConfigSet* m_config = nullptr;

	Bool m_vrs = true;
	Bool m_lsVis = false;
	Bool m_giVis = false;
	Bool m_reflVis = false;

	ButtonsUiNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
		SpatialComponent* spatialc = newComponent<SpatialComponent>();
		spatialc->setAlwaysVisible(true);

		UiComponent* uic = newComponent<UiComponent>();
		uic->init(
			[](CanvasPtr& canvas, void* ud) {
				static_cast<ButtonsUiNode*>(ud)->draw(canvas);
			},
			this);

		ANKI_CHECK_AND_IGNORE(getSceneGraph().getUiManager().newInstance(m_font, "EngineAssets/UbuntuMonoRegular.ttf",
																		 Array<U32, 1>{46}));
	}

	void draw(CanvasPtr& canvas)
	{
		if(!m_renderer->getGrManager().getDeviceCapabilities().m_vrs)
		{
			return;
		}

		const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.3f;

		ImGui::Begin("VrsButtons", nullptr,
					 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove
						 | ImGuiWindowFlags_AlwaysAutoResize);

		canvas->pushFont(m_font, 46);

		ImGui::SetWindowPos(Vec2(canvas->getWidth() - ImGui::GetWindowWidth() - 195.0f, 55.0f));
		ImGui::SetWindowSize(Vec2(230.0f, 450.0f));

		if(m_config->getRVrs())
		{
			//ImGui::Text("VRS visualization: ");
			//ImGui::SameLine();

			if(ImGui::Checkbox("Light Shading", &m_lsVis))
			{
				m_giVis = m_reflVis = false;
			}

			ImGui::SameLine();
			ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Spacing();
			ImGui::SameLine();

			if(ImGui::Checkbox("Global Illumination", &m_giVis))
			{
				m_lsVis = m_reflVis = false;
			}

			ImGui::SameLine();
			ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Spacing();
			ImGui::SameLine();

			if(ImGui::Checkbox("Reflections", &m_reflVis))
			{
				m_lsVis = m_giVis = false;
			}

			ImGui::SameLine();
			ImGui::Spacing();
            ImGui::SameLine();
            ImGui::Spacing();
			ImGui::SameLine();
		}

		if(ImGui::Checkbox("VRS", &m_vrs))
		{
		}

		m_config->setRVrs(m_vrs);

		if(m_config->getRVrs() && m_lsVis)
		{
			m_renderer->setCurrentDebugRenderTarget("VrsSri");
		}
		else if(m_config->getRVrs() && m_giVis)
		{
			m_renderer->setCurrentDebugRenderTarget("IndirectDiffuseVrsSri");
		}
		else if(m_config->getRVrs() && m_reflVis)
		{
			m_renderer->setCurrentDebugRenderTarget("VrsSriDownscaled");
		}
		else
		{
			m_renderer->setCurrentDebugRenderTarget("");
		}

		canvas->popFont();
		ImGui::End();

		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;
	}
};

Error SampleApp::init(int argc, char** argv, CString sampleName)
{
	HeapMemoryPool pool(allocAligned, nullptr);

	// Init the super class
	m_config.init(allocAligned, nullptr);
	m_config.setWindowFullscreen(true);

#if !ANKI_OS_ANDROID
	StringRaii mainDataPath(&pool, ANKI_SOURCE_DIRECTORY);
	StringRaii assetsDataPath(&pool);
	assetsDataPath.sprintf("%s/Samples/%s", ANKI_SOURCE_DIRECTORY, sampleName.cstr());

	if(!directoryExists(assetsDataPath))
	{
		ANKI_LOGE("Cannot find directory \"%s\". Have you moved the clone of the repository? "
				  "I'll continue but expect issues",
				  assetsDataPath.cstr());
	}
	else
	{
		m_config.setRsrcDataPaths(StringRaii(&pool).sprintf("%s:%s", mainDataPath.cstr(), assetsDataPath.cstr()));
	}
#endif

	ANKI_CHECK(m_config.setFromCommandLineArguments(argc - 1, argv + 1));
	ANKI_CHECK(App::init(&m_config, allocAligned, nullptr));

	ANKI_CHECK(sampleExtraInit());

	ButtonsUiNode* node;
	getSceneGraph().newSceneNode("VrsButtons", node);
	node->m_renderer = &getMainRenderer().getOffscreenRenderer();
	node->m_config = &m_config;
	node->m_vrs = m_config.getRVrs();

	return Error::kNone;
}

Error SampleApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	constexpr F32 ROTATE_ANGLE = toRad(2.5f);
	constexpr F32 MOUSE_SENSITIVITY = 5.0f;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();
	Input& in = getInput();

	if(in.getKey(KeyCode::kEscape))
	{
		quit = true;
		return Error::kNone;
	}

	if(in.getKey(KeyCode::kBackquote) == 1)
	{
		setDisplayDeveloperConsole(!getDisplayDeveloperConsole());
	}

	if(in.getKey(KeyCode::kY) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "GBufferNormals") ? "" : "GBufferNormals");
	}

	if(in.getKey(KeyCode::kU) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "IndirectDiffuse") ? "" : "IndirectDiffuse");
	}

	if(in.getKey(KeyCode::kI) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "SSR") ? "" : "SSR");
	}

	if(in.getKey(KeyCode::kO) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "ResolvedShadows") ? "" : "ResolvedShadows");
	}

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

	if(in.getKey(KeyCode::kL) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "LightShading") ? "" : "LightShading");
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
		m_config.setRVrs(!m_config.getRVrs());
	}

	static Vec2 mousePosOn1stClick = in.getMousePosition();
	if(in.getMouseButton(MouseButton::kRight) == 1)
	{
		// Re-init mouse pos
		mousePosOn1stClick = in.getMousePosition();
	}

	if(in.getMouseButton(MouseButton::kRight) || in.hasTouchDevice())
	{
		in.hideCursor(true);

		// move the camera
		static MoveComponent* mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();

		if(in.getKey(KeyCode::k1) == 1)
		{
			mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();
		}

		if(in.getKey(KeyCode::kF1) == 1)
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
		if(in.getKey(KeyCode::kF2) == 1)
		{
			// renderer.getDbg().flipFlags(DbgFlag::SPATIAL_COMPONENT);
		}

		if(in.getKey(KeyCode::kUp))
		{
			mover->rotateLocalX(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::kDown))
		{
			mover->rotateLocalX(-ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::kLeft))
		{
			mover->rotateLocalY(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::kRight))
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

		if(in.getKey(KeyCode::kA))
		{
			mover->moveLocalX(-moveDistance);
		}

		if(in.getKey(KeyCode::kD))
		{
			mover->moveLocalX(moveDistance);
		}

		if(in.getKey(KeyCode::kQ))
		{
			mover->moveLocalY(-moveDistance);
		}

		if(in.getKey(KeyCode::kE))
		{
			mover->moveLocalY(moveDistance);
		}

		if(in.getKey(KeyCode::kW))
		{
			mover->moveLocalZ(-moveDistance);
		}

		if(in.getKey(KeyCode::kS))
		{
			mover->moveLocalZ(moveDistance);
		}

		if(in.getKey(KeyCode::kF12) == 1 && ANKI_ENABLE_TRACE)
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

		static TouchPointer rotateCameraTouch = TouchPointer::kCount;
		static Vec2 rotateEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(rotateCameraTouch == TouchPointer::kCount && in.getTouchPointer(touch) == 1
			   && in.getTouchPointerNdcPosition(touch).x() > 0.1f && in.getTouchPointerNdcPosition(touch).y() < 0.6f)
			{
				rotateCameraTouch = touch;
				rotateEventInitialPos = in.getTouchPointerNdcPosition(touch) * getWindow().getAspectRatio();
				break;
			}
		}

		if(rotateCameraTouch != TouchPointer::kCount && in.getTouchPointer(rotateCameraTouch) == 0)
		{
			rotateCameraTouch = TouchPointer::kCount;
		}

		if(rotateCameraTouch != TouchPointer::kCount && in.getTouchPointer(rotateCameraTouch) > 1)
		{
			Vec2 velocity =
				in.getTouchPointerNdcPosition(rotateCameraTouch) * getWindow().getAspectRatio() - rotateEventInitialPos;
			velocity *= 0.1f;

			Euler angles(mover->getLocalRotation().getRotationPart());
			angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.z() = 0.0f;
			mover->setLocalRotation(Mat3x4(Vec3(0.0f), angles));
		}

		static TouchPointer moveCameraTouch = TouchPointer::kCount;
		static Vec2 moveEventInitialPos = Vec2(0.0f);
		for(TouchPointer touch : EnumIterable<TouchPointer>())
		{
			if(moveCameraTouch == TouchPointer::kCount && in.getTouchPointer(touch) == 1
			   && in.getTouchPointerNdcPosition(touch).x() < -0.1f && in.getTouchPointerNdcPosition(touch).y() < 0.6f)
			{
				moveCameraTouch = touch;
				moveEventInitialPos = in.getTouchPointerNdcPosition(touch) * getWindow().getAspectRatio();
				break;
			}
		}

		if(moveCameraTouch != TouchPointer::kCount && in.getTouchPointer(moveCameraTouch) == 0)
		{
			moveCameraTouch = TouchPointer::kCount;
		}

		if(moveCameraTouch != TouchPointer::kCount && in.getTouchPointer(moveCameraTouch) > 0)
		{
			Vec2 velocity =
				in.getTouchPointerNdcPosition(moveCameraTouch) * getWindow().getAspectRatio() - moveEventInitialPos;
			velocity *= 2.0f;

			mover->moveLocalX(moveDistance * velocity.x());
			mover->moveLocalZ(moveDistance * -velocity.y());
		}

		// Camera fly mode
		if(moveCameraTouch != TouchPointer::kCount || rotateCameraTouch != TouchPointer::kCount)
		{
			m_timesOfLastTouchEvent = kIdleTime;

			if(m_cameraAnimationEvent)
			{
				m_cameraAnimationEvent->setMarkedForDeletion();
				m_cameraAnimationEvent = nullptr;
			}
		}

		m_timesOfLastTouchEvent -= elapsedTime;

		if(m_timesOfLastTouchEvent < 0.0 && m_cameraAnimationEvent == nullptr)
		{
			scene.getEventManager().newEvent(m_cameraAnimationEvent,
											 "Assets/Camera.001Action.003_67159cf18109479.ankianim", "Camera.001",
											 &scene.getActiveCameraNode());
		}
	}
	else
	{
		in.hideCursor(false);
	}

	return Error::kNone;
}
