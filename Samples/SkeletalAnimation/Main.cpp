// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/Common/SampleApp.h>

using namespace anki;

enum class DroidAnimation : U32
{
	kFloat,
	kWave
};

class MyApp : public SampleApp
{
public:
	using SampleApp::SampleApp;

	Error userPostInit() override
	{
		ScriptResourcePtr script;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/Scene.lua", script));
		ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

		SkinComponent& skinc = SceneGraph::getSingleton().findSceneNode("droid.001").getFirstComponentOfType<SkinComponent>();
		skinc.registerAnimation(DroidAnimation::kFloat, "Assets/float.001_ccb9eb33e30c8fa4.ankianim");
		skinc.registerAnimation(DroidAnimation::kWave, "Assets/wave_6cf284ed471bff3b.ankianim");

		skinc.playAnimation(0, DroidAnimation::kFloat, 0.0, AnimationStart::kNow).m_repeatTimes = -1.0f;

		g_cvarRenderBloomThreshold = 5.0f;
		return Error::kNone;
	}

	Error userMainLoop(Bool& quit, Second elapsedTime) override
	{
		if(Input::getSingleton().getKey(KeyCode::kZ) == 1)
		{
			SkinComponent& skinc = SceneGraph::getSingleton().findSceneNode("droid.001").getFirstComponentOfType<SkinComponent>();
			skinc.playAnimation(1, DroidAnimation::kWave, 0.5, AnimationStart::kNow).m_repeatTimes = 3.0f;
		}

		return SampleApp::userMainLoop(quit, elapsedTime);
	}
};

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	MyApp* app = new MyApp(argc, argv, "SkeletalAnimation");
	const Error err = app->mainLoop();
	delete app;

	if(err)
	{
		ANKI_LOGE("Error reported. Bye!");
	}
	else
	{
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
