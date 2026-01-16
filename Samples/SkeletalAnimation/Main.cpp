// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/Common/SampleApp.h>

using namespace anki;

class MyApp : public SampleApp
{
public:
	using SampleApp::SampleApp;

	AnimationResourcePtr m_floatAnim;
	AnimationResourcePtr m_waveAnim;

	Error userPostInit() override
	{
		ScriptResourcePtr script;
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/Scene.lua", script));
		ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

		ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/float.001_ccb9eb33e30c8fa4.ankianim", m_floatAnim));
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/wave_6cf284ed471bff3b.ankianim", m_waveAnim));

		AnimationPlayInfo animInfo;
		animInfo.m_startTime = 2.0;
		animInfo.m_repeatTimes = -1.0;
		SceneGraph::getSingleton().findSceneNode("droid.001").getFirstComponentOfType<SkinComponent>().playAnimation(0, m_floatAnim, animInfo);

		g_cvarRenderBloomThreshold = 5.0f;
		return Error::kNone;
	}

	Error userMainLoop(Bool& quit, Second elapsedTime) override
	{
		if(Input::getSingleton().getKey(KeyCode::kZ) == 1)
		{
			AnimationPlayInfo animInfo;
			animInfo.m_startTime = 0.5;
			animInfo.m_repeatTimes = 3.0;
			animInfo.m_blendInTime = 0.5;
			animInfo.m_blendOutTime = 0.35;
			SceneGraph::getSingleton().findSceneNode("droid.001").getFirstComponentOfType<SkinComponent>().playAnimation(1, m_waveAnim, animInfo);
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
