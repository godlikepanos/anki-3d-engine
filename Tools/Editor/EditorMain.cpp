// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

ANKI_CVAR(StringCVar, Editor, Scene, "", "Load this scene at startup")

class EditorUiNode : public SceneNode
{
public:
	EditorUi m_editorUi;

	EditorUiNode(CString name)
		: SceneNode(name)
	{
		UiComponent* uic = newComponent<UiComponent>();
		uic->init(
			[](UiCanvas& canvas, void* ud) {
				static_cast<EditorUiNode*>(ud)->m_editorUi.draw(canvas);
			},
			this);
	}
};

class MyApp : public App
{
public:
	U32 m_argc = 0;
	Char** m_argv = nullptr;

	EditorUiNode* m_editorUiNode = nullptr;

	String m_sceneLuaFname;

	MyApp(U32 argc, Char** argv)
		: App("AnKiEditor")
		, m_argc(argc)
		, m_argv(argv)
	{
	}

	Error userPreInit() override
	{
		g_cvarWindowFullscreen = false;
		g_cvarWindowMaximized = true;
		g_cvarWindowBorderless = true;
		ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 1, m_argv + 1));

		if(CString(g_cvarEditorScene) != "")
		{
			ANKI_CHECK(walkDirectoryTree(g_cvarEditorScene, [this](WalkDirectoryArgs& args) {
				if(!args.m_isDirectory && args.m_path.find("Scene.lua") != CString::kNpos)
				{
					m_sceneLuaFname = args.m_path;
					args.m_stopSearch = true;
				}

				return Error::kNone;
			}));

			if(m_sceneLuaFname)
			{
				String dataPaths = CString(g_cvarRsrcDataPaths);
				dataPaths += ":";
				dataPaths += CString(g_cvarEditorScene);
				g_cvarRsrcDataPaths = dataPaths;
			}
			else
			{
				ANKI_LOGE("Failed to find a Scene.lua");
			}
		}

		return Error::kNone;
	}

	Error userPostInit() override
	{
		m_editorUiNode = SceneGraph::getSingleton().newSceneNode<EditorUiNode>("MainUi");

		if(m_sceneLuaFname)
		{
			ANKI_LOGI("Will load: %s", m_sceneLuaFname.cstr());

			ScriptResourcePtr script;
			ANKI_CHECK(ResourceManager::getSingleton().loadResource(m_sceneLuaFname, script));
			ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));
		}

		return Error::kNone;
	}

	Error userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime) override
	{
		Input& in = Input::getSingleton();
		SceneGraph& scene = SceneGraph::getSingleton();
		if(m_editorUiNode->m_editorUi.m_quit)
		{
			quit = true;
		}

		static Vec2 mousePosOn1stClick = in.getMousePositionNdc();
		if(in.getMouseButton(MouseButton::kRight) == 1)
		{
			// Re-init mouse pos
			mousePosOn1stClick = in.getMousePositionNdc();
		}

		if(in.getMouseButton(MouseButton::kRight) > 0)
		{
			in.hideCursor(true);

			// move the camera
			static SceneNode* mover = &scene.getActiveCameraNode();

			constexpr F32 kRotateAngle = toRad(2.5f);
			constexpr F32 kMouseSensitivity = 5.0f;

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

			const F32 moveDistance = 0.1f;
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
				angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * kMouseSensitivity;
				angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
				angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * kMouseSensitivity;
				angles.z() = 0.0f;
				mover->setLocalRotation(Mat3(angles));
			}
		}
		else
		{
			in.hideCursor(false);
		}

		return Error::kNone;
	}
};

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	MyApp* app = new MyApp(argc, argv);
	const Error err = app->mainLoop();
	delete app;

	if(err)
	{
		ANKI_LOGE("Error reported. Bye!!");
	}
	else
	{
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
