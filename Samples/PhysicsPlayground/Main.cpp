// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	ANKI_CHECK(node->getSceneGraph().getEventManager().newEvent(event, -1, 10.0, script));
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
	ANKI_CHECK(node->getSceneGraph().getEventManager().newEvent(event, -1, 10.0, script));
	event->addAssociatedSceneNode(node);

	return Error::kNone;
}

class RayCast : public PhysicsWorldRayCastCallback
{
public:
	Vec3 m_hitPosition = Vec3(kMaxF32);
	Vec3 m_hitNormal;
	Bool m_hit = false;

	RayCast(Vec3 from, Vec3 to, PhysicsMaterialBit mtl)
		: PhysicsWorldRayCastCallback(from, to, mtl)
	{
	}

	void processResult([[maybe_unused]] PhysicsFilteredObject& obj, const Vec3& worldNormal, const Vec3& worldPosition)
	{
		if((m_from - m_to).dot(worldNormal) < 0.0f)
		{
			return;
		}

		if((worldPosition - m_from).getLengthSquared() > (m_hitPosition - m_from).getLengthSquared())
		{
			return;
		}

		m_hitPosition = worldPosition;
		m_hitNormal = worldNormal;
		m_hit = true;
	}
};

class MyApp : public SampleApp
{
public:
	Error sampleExtraInit() override;
	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

Error MyApp::sampleExtraInit()
{
	ScriptResourcePtr script;
	ANKI_CHECK(getResourceManager().loadResource("Assets/Scene.lua", script));
	ANKI_CHECK(getScriptManager().evalString(script->getSource()));

	// Create the player
	if(1)
	{
		SceneNode& cam = getSceneGraph().getActiveCameraNode();
		cam.setLocalTransform(Transform(Vec4(0.0, 2.0, 5.0, 0.0), Mat3x4::getIdentity(), 1.0));

		SceneNode* player;
		ANKI_CHECK(getSceneGraph().newSceneNode("player", player));
		PlayerControllerComponent* playerc = player->newComponent<PlayerControllerComponent>();
		playerc->moveToPosition(Vec3(0.0f, 2.5f, 0.0f));
		playerc->getPhysicsPlayerController().setMaterialMask(PhysicsMaterialBit::kStaticGeometry);

		player->addChild(&cam);
	}

	// Create a body component with joint
	{
		SceneNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode("monkey_p2p", monkey));
		ModelComponent* modelc = monkey->newComponent<ModelComponent>();
		modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");

		BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
		bodyc->loadMeshResource("Assets/Suzanne_e3526e1428c0763c.ankimesh");
		bodyc->teleportTo(Transform(Vec4(-0.0f, 4.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f));
		bodyc->setMass(2.0f);

		JointComponent* jointc = monkey->newComponent<JointComponent>();
		jointc->newHingeJoint(Vec3(0.2f, 1.0f, 0.0f), Vec3(1, 0, 0));
	}

	// Create a chain
	{
		const U LINKS = 5;

		SceneNode* prevBody = nullptr;
		for(U32 i = 0; i < LINKS; ++i)
		{
			SceneNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode(
				StringRaii(&getMemoryPool()).sprintf("monkey_chain%u", i).toCString(), monkey));
			monkey->newComponent<ModelComponent>()->loadModelResource(
				"Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");

			Transform trf(Vec4(-4.3f, 12.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f);
			trf.getOrigin().y() -= F32(i) * 1.25f;
			// trf.getOrigin().x() -= i * 0.25f;

			// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

			BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
			bodyc->setMeshFromModelComponent();
			bodyc->teleportTo(trf);
			bodyc->setMass(1.0f);

			// Create joint
			JointComponent* jointc = monkey->newComponent<JointComponent>();
			if(prevBody == nullptr)
			{
				jointc->newPoint2PointJoint(Vec3(0, 1, 0));
			}
			else
			{
				prevBody->addChild(monkey);
				jointc->newPoint2PointJoint2(Vec3(0, 1.0, 0), Vec3(0, -1.0, 0));
			}

			prevBody = monkey;
		}
	}

	// Trigger
	{
		SceneNode* node;
		ANKI_CHECK(getSceneGraph().newSceneNode("trigger", node));
		TriggerComponent* triggerc = node->newComponent<TriggerComponent>();
		triggerc->setSphereVolumeRadius(1.8f);
		node->setLocalTransform(Transform(Vec4(1.0f, 0.5f, 0.0f, 0.0f), Mat3x4::getIdentity(), 1.0f));
	}

	getInput().lockCursor(true);
	getInput().hideCursor(true);
	getInput().moveCursor(Vec2(0.0f));

	return Error::kNone;
}

Error MyApp::userMainLoop(Bool& quit, [[maybe_unused]] Second elapsedTime)
{
	// ANKI_CHECK(SampleApp::userMainLoop(quit));
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();

	if(getInput().getKey(KeyCode::kEscape))
	{
		quit = true;
	}

	if(getInput().getKey(KeyCode::kH) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "RtShadows") ? ""
																									 : "RtShadows");
	}

	if(getInput().getKey(KeyCode::kP) == 1)
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

	if(getInput().getKey(KeyCode::kL) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "Bloom") ? "" : "Bloom");
	}

	if(getInput().getKey(KeyCode::kJ) == 1)
	{
		ConfigSet::getSingleton().setRVrs(!ConfigSet::getSingleton().getRVrs());
	}

	if(getInput().getKey(KeyCode::kF1) == 1)
	{
		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			ConfigSet::getSingleton().setRDbg(false);
		}
		else if(mode == 1)
		{
			ConfigSet::getSingleton().setRDbg(true);
			getMainRenderer().getDbg().setDepthTestEnabled(true);
			getMainRenderer().getDbg().setDitheredDepthTestEnabled(false);
		}
		else
		{
			ConfigSet::getSingleton().setRDbg(true);
			getMainRenderer().getDbg().setDepthTestEnabled(false);
			getMainRenderer().getDbg().setDitheredDepthTestEnabled(true);
		}
	}

	// Move player
	{
		SceneNode& player = getSceneGraph().findSceneNode("player");
		PlayerControllerComponent& playerc = player.getFirstComponentOfType<PlayerControllerComponent>();

		if(getInput().getKey(KeyCode::kR))
		{
			player.getFirstComponentOfType<PlayerControllerComponent>().moveToPosition(Vec3(0.0f, 2.0f, 0.0f));
		}

		constexpr F32 ang = toRad(7.0f);

		F32 y = getInput().getMousePosition().y();
		F32 x = getInput().getMousePosition().x();
		if(y != 0.0 || x != 0.0)
		{
			// Set origin
			Vec4 origin = player.getWorldTransform().getOrigin();
			// origin.y() += 1.9f;

			// Set rotation
			Mat3x4 rot(Vec3(0.0f), Euler(ang * y * 11.25f, ang * x * -20.0f, 0.0f));

			rot = player.getLocalRotation().combineTransformations(rot);

			Vec3 newz = rot.getColumn(2).getNormalized();
			Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
			Vec3 newy = newz.cross(newx);
			rot.setColumns(newx, newy, newz, Vec3(0.0));
			rot.reorthogonalize();

			// Update move
			player.setLocalTransform(Transform(origin, rot, 1.0));
		}

		const F32 speed = 0.5;
		Vec4 moveVec(0.0);
		if(getInput().getKey(KeyCode::kW))
		{
			moveVec.z() += 1.0f;
		}

		if(getInput().getKey(KeyCode::kA))
		{
			moveVec.x() -= 1.0f;
		}

		if(getInput().getKey(KeyCode::kS))
		{
			moveVec.z() -= 1.0f;
		}

		if(getInput().getKey(KeyCode::kD))
		{
			moveVec.x() += 1.0f;
		}

		Vec4 dir = -player.getLocalRotation().getZAxis().xyz0();
		dir.y() = 0.0f;
		dir.normalize();

		playerc.setVelocity(moveVec.z() * speed, moveVec.x() * speed, 0.0, dir);
	}

	if(getInput().getMouseButton(MouseButton::kLeft) == 1)
	{
		ANKI_LOGI("Firing a monkey");

		static U32 instance = 0;

		Transform camTrf = getSceneGraph().getActiveCameraNode().getWorldTransform();

		SceneNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode(
			StringRaii(&getMemoryPool()).sprintf("FireMonkey%u", instance++).toCString(), monkey));
		ModelComponent* modelc = monkey->newComponent<ModelComponent>();
		modelc->loadModelResource("Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl");
		// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(camTrf);

		BodyComponent* bodyc = monkey->newComponent<BodyComponent>();
		bodyc->setMeshFromModelComponent();
		bodyc->teleportTo(camTrf);
		bodyc->setMass(1.0f);

		PhysicsBodyPtr pbody = bodyc->getPhysicsBody();
		pbody->applyForce(camTrf.getRotation().getZAxis().xyz() * -1500.0f, Vec3(0.0f, 0.0f, 0.0f));

		// Create the destruction event
		ANKI_CHECK(createDestructionEvent(monkey));
	}

	if(getInput().getMouseButton(MouseButton::kRight) == 1)
	{
		Transform camTrf = getSceneGraph().getActiveCameraNode().getWorldTransform();
		Vec3 from = camTrf.getOrigin().xyz();
		Vec3 to = from + -camTrf.getRotation().getZAxis() * 100.0f;

		RayCast ray(from, to, PhysicsMaterialBit::kAll & (~PhysicsMaterialBit::kParticle));
		ray.m_firstHit = true;

		PhysicsWorld::getSingleton().rayCast(ray);

		if(ray.m_hit)
		{
			// Create rotation
			const Vec3& zAxis = ray.m_hitNormal;
			Vec3 yAxis = Vec3(0, 1, 0.5);
			Vec3 xAxis = yAxis.cross(zAxis).getNormalized();
			yAxis = zAxis.cross(xAxis);

			Mat3x4 rot = Mat3x4::getIdentity();
			rot.setXAxis(xAxis);
			rot.setYAxis(yAxis);
			rot.setZAxis(zAxis);

			Transform trf(ray.m_hitPosition.xyz0(), rot, 1.0f);

			// Create an obj
			static U32 id = 0;
			SceneNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode(
				StringRaii(&getSceneGraph().getFrameMemoryPool()).sprintf("decal%u", id++).toCString(), monkey));
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
				StringRaii name(&getSceneGraph().getFrameMemoryPool());
				name.sprintf("fog%u", id++);

				SceneNode* fogNode;
				ANKI_CHECK(getSceneGraph().newSceneNode(name.toCString(), fogNode));
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
		SceneNode& node = getSceneGraph().findSceneNode("trigger");
		TriggerComponent& comp = node.getFirstComponentOfType<TriggerComponent>();

		for(U32 i = 0; i < comp.getBodyComponentsEnter().getSize(); ++i)
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
		delete app;
		ANKI_LOGI("Bye!!");
	}

	return 0;
}
