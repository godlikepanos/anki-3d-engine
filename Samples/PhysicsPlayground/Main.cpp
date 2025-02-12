// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include <Samples/Common/SampleApp.h>

using namespace anki;

static Error createDestructionEvent(SceneNode* node)
{
	CString script = R"(
function update(event, prevTime, crntTime)
	-- Do nothing
	return 1
end

function onKilled(event, prevTime, crntTime)
	logi(string.format("Will kill %s", event:getAssociatedSceneNodes():getAt(0):getName()))
	event:getAssociatedSceneNodes():getAt(0):setMarkedForDeletion()
	return 1
end
	)";
	ScriptEvent* event;
	ANKI_CHECK(SceneGraph::getSingleton().getEventManager().newEvent(event, -1, 10.0, script));
	event->addAssociatedSceneNode(node);

	return Error::kNone;
}

static Error createFogVolumeFadeEvent(SceneNode* node)
{
	CString script = R"(
density = 15
radius = 3.5

function update(event, prevTime, crntTime)
	node = event:getAssociatedSceneNodes():getAt(0)
	-- logi(string.format("Will fade fog for %s", node:getName()))
	fogComponent = node:getFirstFogDensityComponent()

	dt = crntTime - prevTime
	density = density - 4.0 * dt
	radius = radius + 0.5 * dt

	pos = node:getLocalOrigin()
	pos:setY(pos:getY() - 1.1 * dt)
	node:setLocalOrigin(pos)

	if density <= 0.0 or radius <= 0.0 then
		event:getAssociatedSceneNodes():getAt(0):setMarkedForDeletion()
	else
		fogComponent:setSphereVolumeRadius(radius)
		fogComponent:setDensity(density)
	end

	return 1
end

function onKilled(event, prevTime, crntTime)
	return 1
end
	)";
	ScriptEvent* event;
	ANKI_CHECK(SceneGraph::getSingleton().getEventManager().newEvent(event, -1, 10.0, script));
	event->addAssociatedSceneNode(node);

	return Error::kNone;
}

class MyApp : public SampleApp
{
public:
	Error sampleExtraInit() override;
	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

Error MyApp::sampleExtraInit()
{
	ScriptResourcePtr script;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("Assets/Scene.lua", script));
	ANKI_CHECK(ScriptManager::getSingleton().evalString(script->getSource()));

	// Create the player
	if(1)
	{
		SceneNode& cam = SceneGraph::getSingleton().getActiveCameraNode();
		cam.setLocalTransform(Transform(Vec4(0.0, 2.0, 5.0, 0.0), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f)));

		SceneNode* player;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode("player", player));
		PlayerControllerComponent* playerc = player->newComponent<PlayerControllerComponent>();
		playerc->moveToPosition(Vec3(0.0f, 10.5f, 0.0f));

		player->addChild(&cam);
	}

	// Create a body component with hinge joint
	if(1)
	{
		SceneNode* base;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode("hingeBase", base));
		BodyComponent* bodyc = base->newComponent<BodyComponent>();
		bodyc->setBoxExtend(Vec3(0.1f));
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kAabb);
		bodyc->teleportTo(Transform(Vec4(-0.0f, 5.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f)));

		SceneNode* joint;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode("hinge", joint));
		JointComponent* jointc = joint->newComponent<JointComponent>();
		jointc->setType(JointType::kHinge);
		joint->setLocalOrigin(Vec4(-0.0f, 4.8f, -3.0f, 0.0f));
		base->addChild(joint);

		SceneNode* monkey;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode("monkey_p2p", monkey));
		ModelComponent* modelc = monkey->newComponent<ModelComponent>();
		modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
		const Aabb aabb = modelc->getModelResource()->getBoundingVolume();
		const F32 height = aabb.getMax().y() - aabb.getMin().y();

		bodyc = monkey->newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromModelComponent);
		bodyc->teleportTo(Transform(Vec4(-0.0f, 4.8f - height / 2.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f)));
		bodyc->setMass(2.0f);

		joint->addChild(monkey);
	}

	// Create a chain
	if(1)
	{
		const U linkCount = 5;

		Transform trf(Vec4(-4.3f, 12.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), Vec4(1.0, 1.0, 1.0, 0.0));

		SceneNode* base;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode("p2pBase", base));
		BodyComponent* bodyc = base->newComponent<BodyComponent>();
		bodyc->setBoxExtend(Vec3(0.1f));
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kAabb);
		bodyc->teleportTo(trf);

		trf.setOrigin(trf.getOrigin() - Vec4(0.0f, 0.5f, 0.0f, 0.0f));

		SceneNode* prevNode = base;

		for(U32 i = 0; i < linkCount; ++i)
		{
			SceneNode* joint;
			ANKI_CHECK(SceneGraph::getSingleton().newSceneNode(String().sprintf("joint_chain%u", i), joint));
			JointComponent* jointc = joint->newComponent<JointComponent>();
			jointc->setType(JointType::kPoint);
			joint->setLocalOrigin(trf.getOrigin());
			joint->setParent(prevNode);

			SceneNode* monkey;
			ANKI_CHECK(SceneGraph::getSingleton().newSceneNode(String().sprintf("monkey_chain%u", i).toCString(), monkey));
			ModelComponent* modelc = monkey->newComponent<ModelComponent>();
			modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
			const Aabb aabb = modelc->getModelResource()->getBoundingVolume();
			const F32 height = aabb.getMax().y() - aabb.getMin().y();

			trf.setOrigin(trf.getOrigin() - Vec4(0.0f, height / 2.0f + 0.1f, 0.0f, 0.0f));

			BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
			bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromModelComponent);
			bodyc->teleportTo(trf);
			bodyc->setMass(1.0f);
			joint->addChild(monkey);

			trf.setOrigin(trf.getOrigin() - Vec4(0.0f, height / 2.0f + 0.1f, 0.0f, 0.0f));

			prevNode = monkey;
		}
	}

	// Trigger
	if(1)
	{
		SceneNode* node;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode("trigger", node));
		TriggerComponent* triggerc = node->newComponent<TriggerComponent>();
		triggerc->setSphereVolumeRadius(1.8f);
		node->setLocalTransform(Transform(Vec4(4.0f, 0.5f, 0.0f, 0.0f), Mat3x4::getIdentity(), Vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	}

	Input::getSingleton().lockCursor(true);
	Input::getSingleton().hideCursor(true);
	Input::getSingleton().moveCursor(Vec2(0.0f));

	return Error::kNone;
}

Error MyApp::userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime)
{
	// ANKI_CHECK(SampleApp::userMainLoop(quit));
	Renderer& renderer = Renderer::getSingleton();

	if(Input::getSingleton().getKey(KeyCode::kEscape))
	{
		quit = true;
	}

	if(Input::getSingleton().getKey(KeyCode::kH) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "RtShadows") ? "" : "RtShadows");
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
		g_vrsCVar = !g_vrsCVar;
	}

	if(Input::getSingleton().getKey(KeyCode::kF1) == 1)
	{
		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			g_dbgSceneCVar = false;
		}
		else if(mode == 1)
		{
			g_dbgSceneCVar = true;
			renderer.getDbg().setDepthTestEnabled(true);
			renderer.getDbg().setDitheredDepthTestEnabled(false);
		}
		else
		{
			g_dbgSceneCVar = true;
			renderer.getDbg().setDepthTestEnabled(false);
			renderer.getDbg().setDitheredDepthTestEnabled(true);
		}
	}

	if(Input::getSingleton().getKey(KeyCode::kF2) == 1)
	{
		g_dbgPhysicsCVar = !g_dbgPhysicsCVar;
		renderer.getDbg().setDepthTestEnabled(true);
		renderer.getDbg().setDitheredDepthTestEnabled(false);
	}

	// Move player
	{
		SceneNode& player = SceneGraph::getSingleton().findSceneNode("player");
		PlayerControllerComponent& playerc = player.getFirstComponentOfType<PlayerControllerComponent>();

		if(Input::getSingleton().getKey(KeyCode::kR))
		{
			player.getFirstComponentOfType<PlayerControllerComponent>().moveToPosition(Vec3(0.0f, 2.0f, 0.0f));
		}

		constexpr F32 ang = toRad(7.0f);

		F32 y = Input::getSingleton().getMousePosition().y();
		F32 x = Input::getSingleton().getMousePosition().x();
		if(y != 0.0 || x != 0.0)
		{
			// Set rotation
			Mat3x4 rot(Vec3(0.0f), Euler(ang * y * 11.25f, ang * x * -20.0f, 0.0f));

			rot = player.getLocalRotation().combineTransformations(rot);

			Vec3 newz = rot.getColumn(2).getNormalized();
			Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
			Vec3 newy = newz.cross(newx);
			rot.setColumns(newx, newy, newz, Vec3(0.0));
			rot.reorthogonalize();

			// Update move
			player.setLocalRotation(rot);
		}

		const F32 speed = 8.5;
		Vec3 moveVec(0.0);
		if(Input::getSingleton().getKey(KeyCode::kW))
		{
			moveVec.z() += 1.0f;
		}

		if(Input::getSingleton().getKey(KeyCode::kA))
		{
			moveVec.x() += 1.0f;
		}

		if(Input::getSingleton().getKey(KeyCode::kS))
		{
			moveVec.z() -= 1.0f;
		}

		if(Input::getSingleton().getKey(KeyCode::kD))
		{
			moveVec.x() -= 1.0f;
		}

		F32 jumpSpeed = 0.0f;
		if(Input::getSingleton().getKey(KeyCode::kSpace))
		{
			jumpSpeed += 8.0f;
		}

		static Bool crouch = false;
		Bool crouchChanged = false;
		if(Input::getSingleton().getKey(KeyCode::kC))
		{
			crouch = !crouch;
			crouchChanged = true;
		}

		if(moveVec != 0.0f || jumpSpeed != 0.0f || crouchChanged)
		{
			Vec3 dir;
			if(moveVec != 0.0f)
			{
				dir = -(player.getLocalRotation() * moveVec.xyz0());
				dir.y() = 0.0f;
				dir.normalize();
			}

			playerc.setVelocity(speed, jumpSpeed, dir, crouch);
		}
	}

	if(Input::getSingleton().getMouseButton(MouseButton::kLeft) == 1)
	{
		ANKI_LOGI("Firing a monkey");

		static U32 instance = 0;

		Transform camTrf = SceneGraph::getSingleton().getActiveCameraNode().getWorldTransform();

		SceneNode* monkey;
		ANKI_CHECK(SceneGraph::getSingleton().newSceneNode(String().sprintf("FireMonkey%u", instance++).toCString(), monkey));
		ModelComponent* modelc = monkey->newComponent<ModelComponent>();
		modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
		// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(camTrf);

		BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromModelComponent);
		bodyc->teleportTo(camTrf);
		bodyc->setMass(1.0f);

		bodyc->applyForce(camTrf.getRotation().getZAxis().xyz() * -1500.0f, Vec3(0.0f, 0.0f, 0.0f));

		// Create the destruction event
		ANKI_CHECK(createDestructionEvent(monkey));
	}

	if(Input::getSingleton().getMouseButton(MouseButton::kRight) == 1)
	{
		Transform camTrf = SceneGraph::getSingleton().getActiveCameraNode().getWorldTransform();
		Vec3 from = camTrf.getOrigin().xyz();
		Vec3 to = from + -camTrf.getRotation().getZAxis() * 100.0f;

		v2::RayHitResult result;
		const Bool hit = v2::PhysicsWorld::getSingleton().castRayClosestHit(from, to, v2::PhysicsLayerBit::kStatic, result);

		if(hit)
		{
			// Create rotation
			const Vec3& zAxis = result.m_normal;
			Vec3 yAxis = Vec3(0, 1, 0.5);
			Vec3 xAxis = yAxis.cross(zAxis).getNormalized();
			yAxis = zAxis.cross(xAxis);

			Mat3x4 rot = Mat3x4::getIdentity();
			rot.setXAxis(xAxis);
			rot.setYAxis(yAxis);
			rot.setZAxis(zAxis);

			Transform trf(result.m_hitPosition.xyz0(), rot, Vec4(1.0f, 1.0f, 1.0f, 0.0f));

			// Create an obj
			static U32 id = 0;
			SceneNode* monkey;
			ANKI_CHECK(SceneGraph::getSingleton().newSceneNode(String().sprintf("decal%u", id++).toCString(), monkey));
			ModelComponent* modelc = monkey->newComponent<ModelComponent>();
			modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
			monkey->setLocalTransform(trf);

			ANKI_CHECK(createDestructionEvent(monkey));

#if 1
			// Create some particles
			ParticleEmitterComponent* partc = monkey->newComponent<ParticleEmitterComponent>();
			partc->loadParticleEmitterResource("Assets/Smoke.ankipart");
#endif

			// Create some fog volumes
			for(U i = 0; i < 1; ++i)
			{
				static int id = 0;
				String name;
				name.sprintf("fog%u", id++);

				SceneNode* fogNode;
				ANKI_CHECK(SceneGraph::getSingleton().newSceneNode(name.toCString(), fogNode));
				FogDensityComponent* fogComp = fogNode->newComponent<FogDensityComponent>();
				fogComp->setSphereVolumeRadius(2.1f);
				fogComp->setDensity(15.0f);

				fogNode->setLocalTransform(trf);

				ANKI_CHECK(createDestructionEvent(fogNode));
				ANKI_CHECK(createFogVolumeFadeEvent(fogNode));
			}
		}
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
	Error err = Error::kNone;

	MyApp* app = new MyApp;
	err = app->init(argc, argv, "PhysicsPlayground");
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
		ANKI_LOGI("Bye!!");
	}

	delete app;

	return 0;
}
