// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include <Samples/Common/SampleApp.h>
#include <Samples/PhysicsPlayground/FpsCharacter.h>

using namespace anki;

static Error createDestructionEvent(SceneNode* node)
{
	CString script = R"(
function update(event, prevTime, crntTime)
	-- Do nothing
end

function onKilled(event, prevTime, crntTime)
	logi(string.format("Will kill %s", event:getAssociatedSceneNodes():getAt(0):getName()))
	event:getAssociatedSceneNodes():getAt(0):markForDeletion()
end
	)";
	ScriptEvent* event = SceneGraph::getSingleton().getEventManager().newEvent<ScriptEvent>(-1.0f, 10.0f, script);
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
		node:markForDeletion()
	else
		node:setLocalScale(Vec3.new(radius))
		fogComponent:setDensity(density)
	end
end

function onKilled(event, prevTime, crntTime)
	-- Nothing
end
	)";
	ScriptEvent* event = SceneGraph::getSingleton().getEventManager().newEvent<ScriptEvent>(-1, 10.0, script);
	event->addAssociatedSceneNode(node);

	return Error::kNone;
}

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
	if(1)
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
		ModelComponent* modelc = monkey->newComponent<ModelComponent>();
		modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
		const Aabb aabb = modelc->getModelResource()->getBoundingVolume();
		const F32 height = aabb.getMax().y() - aabb.getMin().y();

		bodyc = monkey->newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromModelComponent);
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
			ModelComponent* modelc = monkey->newComponent<ModelComponent>();
			modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
			const Aabb aabb = modelc->getModelResource()->getBoundingVolume();
			const F32 height = aabb.getMax().y() - aabb.getMin().y();

			trf.setOrigin(trf.getOrigin() - Vec4(0.0f, height / 2.0f + 0.1f, 0.0f, 0.0f));

			BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
			bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromModelComponent);
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

	Input::getSingleton().lockCursor(true);
	Input::getSingleton().hideCursor(true);
	Input::getSingleton().moveCursor(Vec2(0.0f));

	return Error::kNone;
}

Error MyApp::userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime)
{
	// ANKI_CHECK(SampleApp::userMainLoop(quit));
	Renderer& renderer = Renderer::getSingleton();
	Input& in = Input::getSingleton();

	if(Input::getSingleton().getKey(KeyCode::kEscape))
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
			Mat3 rot(Euler(ang * y * 11.25f, ang * x * -20.0f, 0.0f));

			rot = player.getLocalRotation() * rot;

			Vec3 newz = rot.getColumn(2).normalize();
			Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
			Vec3 newy = newz.cross(newx);
			rot.setColumns(newx, newy, newz);
			rot = rot.reorthogonalize();

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
				dir = -(player.getLocalRotation() * moveVec);
				dir.y() = 0.0f;
				dir = dir.normalize();
			}

			F32 speed1 = speed;
			if(Input::getSingleton().getKey(KeyCode::kLeftShift))
			{
				speed1 *= 2.0f;
			}
			playerc.setVelocity(speed1, jumpSpeed, dir, crouch);
		}
	}

	if(Input::getSingleton().getMouseButton(MouseButton::kRight) == 1)
	{
		ANKI_LOGI("Firing a grenade");

		static U32 instance = 0;

		Transform camTrf = SceneGraph::getSingleton().getActiveCameraNode().getWorldTransform();
		const Vec3 newPos = camTrf.getOrigin().xyz() + camTrf.getRotation().getZAxis() * -3.0f;
		camTrf.setOrigin(newPos.xyz0());

		SceneNode* grenade = SceneGraph::getSingleton().newSceneNode<SceneNode>(String().sprintf("Grenade%u", instance++).toCString());
		grenade->setLocalScale(Vec3(2.8f));
		ModelComponent* modelc = grenade->newComponent<ModelComponent>();
		modelc->loadModelResource("Assets/MESH_grenade_MTL_grenade_85852a78645563d8.ankimdl");
		// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(camTrf);

		BodyComponent* bodyc = grenade->newComponent<BodyComponent>();
		bodyc->setCollisionShapeType(BodyComponentCollisionShapeType::kFromModelComponent);
		bodyc->teleportTo(camTrf.getOrigin().xyz(), camTrf.getRotation().getRotationPart());
		bodyc->setMass(1.0f);

		bodyc->applyForce(camTrf.getRotation().getZAxis().xyz() * -1200.0f, Vec3(0.0f, 0.0f, 0.0f));

		// Create the destruction event
		ANKI_CHECK(createDestructionEvent(grenade));
	}

	if(Input::getSingleton().getMouseButton(MouseButton::kLeft) == 1)
	{
		const Transform camTrf = SceneGraph::getSingleton().getActiveCameraNode().getWorldTransform();

		for(U32 i = 0; i < 8; ++i)
		{
			F32 spredAngle = toRad(getRandomRange(-2.0f, 2.0f));
			Mat3 randDirection(Axisang(spredAngle, Vec3(1.0f, 0.0f, 0.0f)));
			spredAngle = toRad(getRandomRange(-2.0f, 2.0f));
			randDirection = randDirection * Mat3(Axisang(spredAngle, Vec3(0.0f, 1.0f, 0.0f)));
			randDirection = camTrf.getRotation().getRotationPart() * randDirection;

			const Vec3 from = camTrf.getOrigin().xyz();
			const Vec3 to = from + -randDirection.getZAxis() * 100.0f;

			RayHitResult result;
			const Bool hit = PhysicsWorld::getSingleton().castRayClosestHit(from, to, PhysicsLayerBit::kStatic, result);

			if(hit)
			{
				// Create rotation
				const Vec3& zAxis = result.m_normal;
				Vec3 yAxis = Vec3(0, 1, 0.5);
				Vec3 xAxis = yAxis.cross(zAxis).normalize();
				yAxis = zAxis.cross(xAxis);

				Mat3x4 rot = Mat3x4::getIdentity();
				rot.setXAxis(xAxis);
				rot.setYAxis(yAxis);
				rot.setZAxis(zAxis);

				Transform trf(result.m_hitPosition.xyz0(), rot, Vec4(1.0f, 1.0f, 1.0f, 0.0f));

				// Create an obj
				static U32 id = 0;
				SceneNode* bulletDecal = SceneGraph::getSingleton().newSceneNode<SceneNode>(String().sprintf("decal%u", id++).toCString());
				bulletDecal->setLocalTransform(trf);
				bulletDecal->setLocalScale(Vec3(0.1f, 0.1f, 0.3f));
				DecalComponent* decalc = bulletDecal->newComponent<DecalComponent>();
				decalc->loadDiffuseImageResource("Assets/bullet_hole_decal.ankitex", 1.0f);

				ANKI_CHECK(createDestructionEvent(bulletDecal));

#if 0
			// Create some particles
			ParticleEmitterComponent* partc = monkey->newComponent<ParticleEmitterComponent>();
			partc->loadParticleEmitterResource("Assets/Smoke.ankipart");
#endif

				// Create some fog volumes
				if(i == 0)
				{
					static int id = 0;
					String name;
					name.sprintf("fog%u", id++);

					SceneNode* fogNode = SceneGraph::getSingleton().newSceneNode<SceneNode>(name.toCString());
					FogDensityComponent* fogComp = fogNode->newComponent<FogDensityComponent>();
					fogNode->setLocalScale(Vec3(2.1f));
					fogComp->setDensity(15.0f);

					fogNode->setLocalTransform(trf);

					ANKI_CHECK(createDestructionEvent(fogNode));
					ANKI_CHECK(createFogVolumeFadeEvent(fogNode));
				}
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
