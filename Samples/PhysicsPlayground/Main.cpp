// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
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

	// Create the player
	if(0)
	{
		SceneNode& cam = SceneGraph::getSingleton().getActiveCameraNode();
		cam.setLocalTransform(Transform(Vec4(0.0, 2.0, 0.0, 0.0), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f)));

		SceneNode& arm = SceneGraph::getSingleton().findSceneNode("arm");
		arm.setLocalTransform(Transform(Vec3(0.065f, -0.13f, -0.4f), Mat3(Euler(0.0f, kPi, 0.0f)), Vec3(1.0f, 1.0f, 1.0f)));
		arm.setParent(&cam);

		SceneNode* player = SceneGraph::getSingleton().newSceneNode<SceneNode>("player");
		PlayerControllerComponent* playerc = player->newComponent<PlayerControllerComponent>();
		playerc->moveToPosition(Vec3(0.0f, 10.5f, 0.0f));

		player->addChild(&cam);
	}

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
		bodyc->teleportTo(Vec3(-0.0f, 5.0f, -3.0f), Mat3::getIdentity());

		SceneNode* joint = SceneGraph::getSingleton().newSceneNode<SceneNode>("hinge");
		JointComponent* jointc = joint->newComponent<JointComponent>();
		jointc->setType(JointType::kHinge);
		joint->setLocalOrigin(Vec3(-0.0f, 4.8f, -3.0f));
		base->addChild(joint);

		SceneNode* monkey = SceneGraph::getSingleton().newSceneNode<SceneNode>("monkey_p2p");
		monkey->newComponent<MeshComponent>()->setMeshFilename("Assets/Suzanne_e3526e1428c0763c.ankimesh");
		monkey->newComponent<MaterialComponent>()->setMaterialFilename("Assets/dynamic_f238b379a41079ff.ankimtl");

		const Aabb aabb = monkey->getFirstComponentOfType<MeshComponent>().getMeshResource().getBoundingShape();
		const F32 height = aabb.getMax().y() - aabb.getMin().y();

		bodyc = monkey->newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromMeshComponent);
		bodyc->teleportTo(Vec3(-0.0f, 4.8f - height / 2.0f, -3.0f), Mat3::getIdentity());
		bodyc->setMass(2.0f);

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
		bodyc->teleportTo(trf.getOrigin().xyz(), trf.getRotation().getRotationPart());

		trf.setOrigin(trf.getOrigin() - Vec4(0.0f, 0.5f, 0.0f, 0.0f));

		SceneNode* prevNode = base;

		for(U32 i = 0; i < linkCount; ++i)
		{
			SceneNode* joint = SceneGraph::getSingleton().newSceneNode<SceneNode>(String().sprintf("joint_chain%u", i));
			JointComponent* jointc = joint->newComponent<JointComponent>();
			jointc->setType(JointType::kPoint);
			joint->setLocalOrigin(trf.getOrigin().xyz());
			joint->setParent(prevNode);

			SceneNode* monkey = SceneGraph::getSingleton().newSceneNode<SceneNode>(String().sprintf("monkey_chain%u", i).toCString());
			const MeshComponent& meshc = monkey->newComponent<MeshComponent>()->setMeshFilename("Assets/Suzanne_e3526e1428c0763c.ankimesh");
			monkey->newComponent<MaterialComponent>()->setMaterialFilename("Assets/dynamic_f238b379a41079ff.ankimtl");
			const Aabb aabb = meshc.getMeshResource().getBoundingShape();
			const F32 height = aabb.getMax().y() - aabb.getMin().y();

			trf.setOrigin(trf.getOrigin() - Vec4(0.0f, height / 2.0f + 0.1f, 0.0f, 0.0f));

			BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
			bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromMeshComponent);
			bodyc->teleportTo(trf.getOrigin().xyz(), trf.getRotation().getRotationPart());
			bodyc->setMass(1.0f);
			joint->addChild(monkey);

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

	Input::getSingleton().lockMouseWindowCenter(true);
	Input::getSingleton().hideCursor(true);
	Input::getSingleton().moveMouseNdc(Vec2(0.0f));

	return Error::kNone;
}

Error MyApp::userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime)
{
	// ANKI_CHECK(SampleApp::userMainLoop(quit));
	Renderer& renderer = Renderer::getSingleton();
	Input& in = Input::getSingleton();

	if(in.getKey(KeyCode::kGrave) == 1)
	{
		toggleDeveloperConsole();
	}

	if(in.getKey(KeyCode::kEscape) > 0)
	{
		quit = true;
	}

	if(in.getKey(KeyCode::kY) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "IndirectDiffuseClipmapsTest") ? "" : "IndirectDiffuseClipmapsTest");
		// g_shadowMappingPcssCVar = !g_shadowMappingPcssCVar;
	}

	if(in.getKey(KeyCode::kU) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "Reflections") ? "" : "Reflections");
	}

	if(in.getKey(KeyCode::kI) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "Ssao") ? "" : "Ssao");
	}

	if(in.getKey(KeyCode::kO) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "HistoryLen") ? "" : "HistoryLen");
	}

	if(Input::getSingleton().getKey(KeyCode::kP) == 1)
	{
		static U32 idx = 3;
		++idx;
		idx %= 4;
		if(idx == 0)
		{
			renderer.setCurrentDebugRenderTarget("IndirectDiffuseVrsSri");
		}
		else if(idx == 1)
		{
			renderer.setCurrentDebugRenderTarget("VrsSriDownscaled");
		}
		else if(idx == 2)
		{
			renderer.setCurrentDebugRenderTarget("VrsSri");
		}
		else
		{
			renderer.setCurrentDebugRenderTarget("");
		}
	}

	if(Input::getSingleton().getKey(KeyCode::kL) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "Bloom") ? "" : "Bloom");
	}

	if(Input::getSingleton().getKey(KeyCode::kJ) == 1)
	{
		g_cvarGrVrs = !g_cvarGrVrs;
	}

	if(Input::getSingleton().getKey(KeyCode::kF1) == 1)
	{
		DbgOption options = renderer.getDbg().getOptions();

		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			options &= ~DbgOption::kBoundingBoxes;
		}
		else if(mode == 1)
		{
			options |= DbgOption::kBoundingBoxes;
			options |= DbgOption::kDepthTest;
			options &= ~DbgOption::kDitheredDepthTest;
		}
		else
		{
			options |= DbgOption::kBoundingBoxes;
			options &= ~DbgOption::kDepthTest;
			options |= DbgOption::kDitheredDepthTest;
		}

		renderer.getDbg().setOptions(options);
	}

	if(Input::getSingleton().getKey(KeyCode::kF2) == 1)
	{
		DbgOption options = renderer.getDbg().getOptions();
		options ^= DbgOption::kPhysics;
		renderer.getDbg().setOptions(options);
	}

	if(0)
	{
		SceneNode& node = SceneGraph::getSingleton().findSceneNode("trigger");
		TriggerComponent& comp = node.getFirstComponentOfType<TriggerComponent>();

		for(U32 i = 0; i < comp.getSceneNodesEnter().getSize(); ++i)
		{
			// ANKI_LOGI("Touching %s", comp.getContactSceneNodes()[i]->getName().cstr());
		}
	}

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
