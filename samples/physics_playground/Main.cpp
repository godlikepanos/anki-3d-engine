// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include "../common/Framework.h"

using namespace anki;

class MyApp : public SampleApp
{
public:
	Error sampleExtraInit() override;
	Error userMainLoop(Bool& quit) override;
};

Error MyApp::sampleExtraInit()
{
	ScriptResourcePtr script;
	ANKI_CHECK(getResourceManager().loadResource("assets/scene.lua", script));
	ANKI_CHECK(getScriptManager().evalString(script->getSource()));

	// Create the player
	SceneNode& cam = getSceneGraph().getActiveCameraNode();
	cam.getComponent<MoveComponent>().setLocalTransform(
		Transform(Vec4(0.0, 0.0, 5.0, 0.0), Mat3x4::getIdentity(), 1.0));

	PlayerNode* player;
	ANKI_CHECK(getSceneGraph().newSceneNode("player", player, Vec4(0.0f, 2.5f, 0.0f, 0.0f)));

	player->addChild(&cam);

	return Error::NONE;
}

Error MyApp::userMainLoop(Bool& quit)
{
	// ANKI_CHECK(SampleApp::userMainLoop(quit));

	if(getInput().getKey(KeyCode::ESCAPE))
	{
		quit = true;
	}

	if(getInput().getKey(KeyCode::F1) == 1)
	{
		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			getMainRenderer().getDbg().setEnabled(false);
		}
		else if(mode == 1)
		{
			getMainRenderer().getDbg().setEnabled(true);
			getMainRenderer().getDbg().setDepthTestEnabled(true);
			getMainRenderer().getDbg().setDitheredDepthTestEnabled(false);
		}
		else
		{
			getMainRenderer().getDbg().setEnabled(true);
			getMainRenderer().getDbg().setDepthTestEnabled(false);
			getMainRenderer().getDbg().setDitheredDepthTestEnabled(true);
		}
	}

	if(getInput().getKey(KeyCode::R))
	{
		SceneNode& player = getSceneGraph().findSceneNode("player");
		player.getComponent<PlayerControllerComponent>().moveToPosition(Vec4(0.0f, 2.0f, 0.0f, 0.0f));
	}

	if(getInput().getMouseButton(MouseButton::LEFT) == 1)
	{
		ANKI_LOGI("Firing a monkey");

		static U instance = 0;

		Transform camTrf = getSceneGraph().getActiveCameraNode().getComponent<MoveComponent>().getWorldTransform();

		ModelNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>(
			StringAuto(getAllocator()).sprintf("monkey%u", instance++).toCString(),
			monkey,
			"assets/SuzanneMaterial-material.ankimdl"));
		// monkey->getComponent<MoveComponent>().setLocalTransform(camTrf);

		BodyNode* body;
		ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>(
			StringAuto(getAllocator()).sprintf("bmonkey%u", instance++).toCString(), body, "assets/Suzanne.ankicl"));
		body->getComponent<BodyComponent>().setTransform(camTrf);

		PhysicsBodyPtr pbody = body->getComponent<BodyComponent>().getPhysicsBody();
		pbody->applyForce(camTrf.getRotation().getZAxis().xyz() * -1500.0f, Vec3(0.0f, 0.0f, 0.0f));

		body->addChild(monkey);
	}

	return Error::NONE;
}

int main(int argc, char* argv[])
{
	Error err = Error::NONE;

	MyApp* app = new MyApp;
	err = app->init(argc, argv, argv[0]);
	if(!err)
	{
		err = app->mainLoop();
	}

	if(err)
	{
		ANKI_LOGE("Error reported. Bye!");
	}
	else
	{
		delete app;
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
