// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include "../common/Framework.h"

using namespace anki;

class RayCast : public PhysicsWorldRayCastCallback
{
public:
	RayCast(Vec3 from, Vec3 to, PhysicsMaterialBit mtl)
		: PhysicsWorldRayCastCallback(from, to, mtl)
	{
	}

	void processResult(PhysicsFilteredObject& obj, const Vec3& worldNormal, const Vec3& worldPosition)
	{
		SceneNode* node = static_cast<SceneNode*>(obj.getUserData());
		ANKI_ASSERT(node);
		ANKI_LOGI("Ray hits %s", node->getName().cstr());
	}
};

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
	if(1)
	{
		SceneNode& cam = getSceneGraph().getActiveCameraNode();
		cam.getComponent<MoveComponent>().setLocalTransform(
			Transform(Vec4(0.0, 0.0, 5.0, 0.0), Mat3x4::getIdentity(), 1.0));

		PlayerNode* player;
		ANKI_CHECK(getSceneGraph().newSceneNode("player", player, Vec4(0.0f, 2.5f, 0.0f, 0.0f)));
		PlayerControllerComponent& pcomp = player->getComponent<PlayerControllerComponent>();
		pcomp.getPhysicsPlayerController()->setMaterialMask(PhysicsMaterialBit::STATIC_GEOMETRY);

		player->addChild(&cam);
	}

	// Create a body component with joint
	{
		ModelNode* monkey;
		ANKI_CHECK(
			getSceneGraph().newSceneNode<ModelNode>("monkey_p2p", monkey, "assets/SuzanneMaterial-material.ankimdl"));

		BodyNode* body;
		ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>("bmonkey_p2p", body, "assets/Suzanne.ankicl"));
		body->getComponent<BodyComponent>().setTransform(
			Transform(Vec4(-0.0f, 4.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f));

		body->addChild(monkey);

		body->getComponent<JointComponent>().newHingeJoint(Vec3(0.2f, 1.0f, 0.0f), Vec3(1, 0, 0));
	}

	// Create a chain
	{
		const U LINKS = 5;

		BodyNode* prevBody = nullptr;
		for(U i = 0; i < LINKS; ++i)
		{
			ModelNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>(
				StringAuto(getAllocator()).sprintf("monkey_chain%u", i).toCString(),
				monkey,
				"assets/SuzanneMaterial-material.ankimdl"));

			Transform trf(Vec4(-4.3f, 12.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f);
			trf.getOrigin().y() -= i * 1.25f;
			// trf.getOrigin().x() -= i * 0.25f;

			// monkey->getComponent<MoveComponent>().setLocalTransform(trf);

			BodyNode* body;
			ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>(
				StringAuto(getAllocator()).sprintf("bmonkey_chain%u", i).toCString(), body, "assets/Suzanne.ankicl"));
			body->getComponent<BodyComponent>().setTransform(trf);

			// Create joint
			JointComponent& jointc = body->getComponent<JointComponent>();
			if(prevBody == nullptr)
			{
				jointc.newPoint2PointJoint(Vec3(0, 1, 0));
			}
			else
			{
				prevBody->addChild(body);
				jointc.newPoint2PointJoint2(Vec3(0, 1.0, 0), Vec3(0, -1.0, 0));
			}

			body->addChild(monkey);
			prevBody = body;
		}
	}

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

		// Create the destruction event
		CString script = R"(
function update(event, prevTime, crntTime)
	-- Do nothing
	return 1
end

function onKilled(event, prevTime, crntTime)
	logi("onKilled")
	event:getAssociatedSceneNodes():getAt(0):setMarkedForDeletion()
	return 1
end
		)";
		ScriptEvent* event;
		ANKI_CHECK(getSceneGraph().getEventManager().newEvent(event, -1, 10.0, script));
		event->addAssociatedSceneNode(body);
	}

	if(getInput().getMouseButton(MouseButton::RIGHT) == 1)
	{
		Transform camTrf = getSceneGraph().getActiveCameraNode().getComponent<MoveComponent>().getWorldTransform();
		Vec3 from = camTrf.getOrigin().xyz();
		Vec3 to = from + -camTrf.getRotation().getZAxis() * 10.0f;

		RayCast ray(from, to, PhysicsMaterialBit::ALL);

		getPhysicsWorld().rayCast(ray);
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
