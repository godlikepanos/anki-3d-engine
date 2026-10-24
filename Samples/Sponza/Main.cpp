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

	Error userPreInit() final
	{
		ANKI_CHECK(SampleApp::userPreInit());
		g_cvarCoreStartupScene = "Assets/Scene.lua";
		return Error::kNone;
	}

	Error userMainLoop(Bool& quit, Second elapsedTime) final
	{
		ANKI_CHECK(SampleApp::userMainLoop(quit, elapsedTime));

		static Bool firstTime = true;
		if(firstTime)
		{
			firstTime = false;

			SceneNode& knight = SceneGraph::getSingleton().findSceneNode("MESH_kinght.001");
			SkinComponent& skinc = knight.getFirstComponentOfType<SkinComponent>();
			skinc.registerAnimation(0, "Assets/Armature_mixamo.com_Layer0_2ff0b9b4a30af3d0.ankianim");
			AnimationState& state = skinc.playAnimation(0, 0, 0.0, AnimationStart::kNow);
			state.m_repeatTimes = -1.0f;
			state.m_animationSpeedScale = 2.1f;
		}

		return Error::kNone;
	}
};

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	Error err = Error::kNone;

	MyApp* app = new MyApp(argc, argv, "Sponza");
	err = app->mainLoop();

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
