// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class MyApp : public App
{
public:
	U32 m_argc = 0;
	Char** m_argv = nullptr;

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
		g_cvarRsrcTrackFileUpdates = true;
		ANKI_CHECK(CVarSet::getSingleton().setFromCommandLineArguments(m_argc - 1, m_argv + 1));

		return Error::kNone;
	}

	Error userPostInit() override
	{
		SceneGraph::getSingleton().setCheckForResourceUpdates(true);
		SceneNode& editorNode = SceneGraph::getSingleton().getEditorUiNode();
		editorNode.getFirstComponentOfType<UiComponent>().setEnabled(true);

		Renderer::getSingleton().getDbg().enableOptions(DbgOption::kObjectPicking | DbgOption::kIcons);

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
