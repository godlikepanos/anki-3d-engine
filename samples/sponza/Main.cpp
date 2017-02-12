// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include "../common/Framework.h"

using namespace anki;

class MyApp : public SampleApp
{
public:
	Error sampleExtraInit()
	{
		ScriptResourcePtr script;
		ANKI_CHECK(getResourceManager().loadResource("assets/scene.lua", script));
		ANKI_CHECK(getScriptManager().evalString(script->getSource()));

		getMainRenderer().getOffscreenRenderer().getVolumetric().setFogParticleColor(Vec3(1.0, 0.9, 0.9) * 0.009);
		return ErrorCode::NONE;
	}
};

int main(int argc, char* argv[])
{
	Error err = ErrorCode::NONE;

	MyApp* app = new MyApp;
	err = app->init(argc, argv);
	if(!err)
	{
		err = app->mainLoop();
	}

	if(err)
	{
		ANKI_LOGE("Error reported. To run %s you have to navigate to the /path/to/anki/samples/sponza. "
				  "And then execute it",
			argv[0]);
	}
	else
	{
		delete app;
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
