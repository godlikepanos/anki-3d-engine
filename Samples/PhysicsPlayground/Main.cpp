// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/Common/SampleApp.h>
#include <Samples/PhysicsPlayground/FpsCharacterNode.h>

using namespace anki;

class MyApp : public SampleApp
{
public:
	using SampleApp::SampleApp;

	Error sampleExtraInit() override;
	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

Error MyApp::sampleExtraInit()
{
	ScriptResourcePtr script;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/Scene.lua", script));
	ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

	{
		FpsCharacter* c = SceneGraph::getSingleton().newSceneNode<FpsCharacter>("FpsCharacter");
		SceneGraph::getSingleton().setActiveCameraNode(c->m_cameraNode);
	}

	// Create a body component with hinge joint
	if(1)
	{
		SceneNode* base = SceneGraph::getSingleton().newSceneNode<SceneNode>("hingeBase");
		BodyComponent* bodyc = base->newComponent<BodyComponent>();
		bodyc->setBoxExtend(Vec3(0.1f));
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kAabb);
		base->setLocalOrigin(Vec3(-0.0f, 5.0f, -3.0f));

		SceneNode* joint = SceneGraph::getSingleton().newSceneNode<SceneNode>("hinge");
		JointComponent* jointc = joint->newComponent<JointComponent>();
		jointc->setJointType(JointComponentyType::kHinge);
		joint->setLocalOrigin(Vec3(-0.0f, 4.8f, -3.0f));
		base->addChild(joint);

		SceneNode* monkey = SceneGraph::getSingleton().newSceneNode<SceneNode>("monkey_p2p");
		monkey->newComponent<MeshComponent>()->setMeshFilename("Assets/Suzanne_e3526e1428c0763c.ankimesh");
		monkey->newComponent<MaterialComponent>()->setMaterialFilename("Assets/dynamic_f238b379a41079ff.ankimtl");

		const Aabb aabb = monkey->getFirstComponentOfType<MeshComponent>().getMeshResource().getBoundingShape();
		const F32 height = aabb.getMax().y - aabb.getMin().y;

		bodyc = monkey->newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromMeshComponent);
		bodyc->setMass(2.0f);
		monkey->setLocalOrigin(Vec3(-0.0f, 4.8f - height / 2.0f, -3.0f));

		joint->addChild(monkey);
	}

	// Create a chain
	if(1)
	{
		const U linkCount = 5;

		Transform trf(Vec4(-4.3f, 12.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), Vec4(1.0, 1.0, 1.0, 0.0));

		SceneNode* base = SceneGraph::getSingleton().newSceneNode<SceneNode>("p2pBase");
		BodyComponent* bodyc = base->newComponent<BodyComponent>();
		bodyc->setBoxExtend(Vec3(0.1f));
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kAabb);
		base->setLocalOrigin(trf.getOrigin().xyz);

		trf.setOrigin(trf.getOrigin() - Vec4(0.0f, 0.5f, 0.0f, 0.0f));

		SceneNode* prevNode = base;

		for(U32 i = 0; i < linkCount; ++i)
		{
			SceneNode* joint = SceneGraph::getSingleton().newSceneNode<SceneNode>(String().sprintf("joint_chain%u", i));
			JointComponent* jointc = joint->newComponent<JointComponent>();
			jointc->setJointType(JointComponentyType::kPoint);
			joint->setLocalOrigin(trf.getOrigin().xyz);
			joint->setParent(prevNode);

			SceneNode* monkey = SceneGraph::getSingleton().newSceneNode<SceneNode>(String().sprintf("monkey_chain%u", i).toCString());
			const MeshComponent& meshc = monkey->newComponent<MeshComponent>()->setMeshFilename("Assets/Suzanne_e3526e1428c0763c.ankimesh");
			monkey->newComponent<MaterialComponent>()->setMaterialFilename("Assets/dynamic_f238b379a41079ff.ankimtl");
			const Aabb aabb = meshc.getMeshResource().getBoundingShape();
			const F32 height = aabb.getMax().y - aabb.getMin().y;

			trf.setOrigin(trf.getOrigin() - Vec4(0.0f, height / 2.0f + 0.1f, 0.0f, 0.0f));

			BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
			bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromMeshComponent);
			bodyc->setMass(1.0f);
			joint->addChild(monkey);
			monkey->setLocalOrigin(trf.getOrigin().xyz);
			monkey->setLocalRotation(trf.getRotation().getRotationPart());

			trf.setOrigin(trf.getOrigin() - Vec4(0.0f, height / 2.0f + 0.1f, 0.0f, 0.0f));

			prevNode = monkey;
		}
	}

	// Trigger
	if(1)
	{
		SceneNode* node = SceneGraph::getSingleton().newSceneNode<SceneNode>("trigger");
		TriggerComponent* triggerc = node->newComponent<TriggerComponent>();
		triggerc->setType(TriggerComponentShapeType::kSphere);
		node->setLocalScale(Vec3(1.8f, 1.8f, 1.8f));
		node->setLocalOrigin(Vec3(4.0f, 0.5f, 0.0f));
	}

	Input::getSingleton().moveMouseNdc(Vec2(0.0f));
	ANKI_CHECK(Input::getSingleton().handleEvents());

	return Error::kNone;
}

Error MyApp::userMainLoop([[maybe_unused]] Bool& quit, [[maybe_unused]] Second elapsedTime)
{
	return Error::kNone;
}

ANKI_MAIN_FUNCTION(myMain)
int myMain(int argc, char* argv[])
{
	MyApp* app = new MyApp(argc, argv, "PhysicsPlayground");
	Error err = app->mainLoop();
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
