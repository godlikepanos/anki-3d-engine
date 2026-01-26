// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class MyApp : public App
{
public:
	String m_sceneLuaFname;

	MyApp(U32 argc, Char** argv)
		: App("AnKiEditor", argc, argv)
	{
	}

	Error userPreInit() override
	{
		g_cvarWindowFullscreen = false;
		g_cvarWindowMaximized = true;
		g_cvarWindowBorderless = true;
		g_cvarCoreShowEditor = true;

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
