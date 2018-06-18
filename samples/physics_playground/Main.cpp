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
	Error sampleExtraInit() override
	{
		ScriptResourcePtr script;
		ANKI_CHECK(getResourceManager().loadResource("assets/scene.lua", script));
		ANKI_CHECK(getScriptManager().evalString(script->getSource()));
		return Error::NONE;
	}

	Error userMainLoop(Bool& quit) override;
};

Error MyApp::userMainLoop(Bool& quit)
{
	ANKI_CHECK(SampleApp::userMainLoop(quit));

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
