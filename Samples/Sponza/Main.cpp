// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include <Samples/Common/SampleApp.h>

using namespace anki;

class MyApp : public SampleApp
{
public:
	using SampleApp::SampleApp;

	Error sampleExtraInit() final
	{
		ScriptResourcePtr script;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/Scene.lua", script));
		ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

		return Error::kNone;
	}
};

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	Error err = Error::kNone;

	MyApp* app = new MyApp(allocAligned, nullptr);
	err = app->init(argc, argv, "Sponza");
	if(!err)
	{
		err = app->mainLoop();
	}

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
