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
			[](Canvas& canvas, void* ud) {
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
		ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 1, m_argv + 1));
		g_cvarWindowBorderless = true;
		g_cvarWindowFullscreen = false;

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
		Input& input = Input::getSingleton();
		if(input.getKey(KeyCode::kEscape) || m_editorUiNode->m_editorUi.m_quit)
		{
			quit = true;
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
