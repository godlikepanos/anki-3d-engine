// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	fogComponent = node:getFogDensityComponent()

	dt = crntTime - prevTime
	density = density - 4.0 * dt
	radius = radius + 0.5 * dt

	pos = node:getMoveComponent():getLocalOrigin()
	pos:setY(pos:getY() - 1.1 * dt)
	node:getMoveComponent():setLocalOrigin(pos)

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
		cam.getFirstComponentOfType<MoveComponent>().setLocalTransform(
			Transform(Vec4(0.0, 0.0, 5.0, 0.0), Mat3x4::getIdentity(), 1.0));

		PlayerNode* player;
		ANKI_CHECK(getSceneGraph().newSceneNode("player", player));
		PlayerControllerComponent& pcomp = player->getFirstComponentOfType<PlayerControllerComponent>();
		pcomp.moveToPosition(Vec3(0.0f, 2.5f, 0.0f));
		pcomp.getPhysicsPlayerController()->setMaterialMask(PhysicsMaterialBit::kStaticGeometry);

		player->addChild(&cam);
	}

	// Create a body component with joint
	{
		ModelNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>("monkey_p2p", monkey));
		ANKI_CHECK(monkey->getFirstComponentOfType<ModelComponent>().loadModelResource(
			"Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl"));

		BodyNode* body;
		ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>("bmonkey_p2p", body));
		ANKI_CHECK(body->getFirstComponentOfType<BodyComponent>().loadMeshResource(
			"Assets/Suzanne_lod0_e3526e1428c0763c.ankimesh"));
		body->getFirstComponentOfType<BodyComponent>().setWorldTransform(
			Transform(Vec4(-0.0f, 4.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f));
		body->getFirstComponentOfType<BodyComponent>().setMass(2.0f);

		body->addChild(monkey);

		body->getFirstComponentOfType<JointComponent>().newHingeJoint(Vec3(0.2f, 1.0f, 0.0f), Vec3(1, 0, 0));
	}

	// Create a chain
	{
		const U LINKS = 5;

		BodyNode* prevBody = nullptr;
		for(U32 i = 0; i < LINKS; ++i)
		{
			ModelNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>(
				StringRaii(&getMemoryPool()).sprintf("monkey_chain%u", i).toCString(), monkey));
			ANKI_CHECK(monkey->getFirstComponentOfType<ModelComponent>().loadModelResource(
				"Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl"));

			Transform trf(Vec4(-4.3f, 12.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f);
			trf.getOrigin().y() -= F32(i) * 1.25f;
			// trf.getOrigin().x() -= i * 0.25f;

			// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

			BodyNode* body;
			ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>(
				StringRaii(&getMemoryPool()).sprintf("bmonkey_chain%u", i).toCString(), body));
			ANKI_CHECK(body->getFirstComponentOfType<BodyComponent>().loadMeshResource(
				"Assets/Suzanne_lod0_e3526e1428c0763c.ankimesh"));
			body->getFirstComponentOfType<BodyComponent>().setWorldTransform(trf);
			body->getFirstComponentOfType<BodyComponent>().setMass(1.0f);

			// Create joint
			JointComponent& jointc = body->getFirstComponentOfType<JointComponent>();
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

	// Trigger
	{
		TriggerNode* node;
		ANKI_CHECK(getSceneGraph().newSceneNode("trigger", node));
		node->getFirstComponentOfType<TriggerComponent>().setSphereVolumeRadius(1.8f);
		node->getFirstComponentOfType<TriggerComponent>().setWorldTransform(
			Transform(Vec4(1.0f, 0.5f, 0.0f, 0.0f), Mat3x4::getIdentity(), 1.0f));
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
		m_config.setRVrs(!m_config.getRVrs());
	}

	if(getInput().getKey(KeyCode::kF1) == 1)
	{
		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			getConfig().setRDbgEnabled(false);
		}
		else if(mode == 1)
		{
			getConfig().setRDbgEnabled(true);
			getMainRenderer().getDbg().setDepthTestEnabled(true);
			getMainRenderer().getDbg().setDitheredDepthTestEnabled(false);
		}
		else
		{
			getConfig().setRDbgEnabled(true);
			getMainRenderer().getDbg().setDepthTestEnabled(false);
			getMainRenderer().getDbg().setDitheredDepthTestEnabled(true);
		}
	}

	if(getInput().getKey(KeyCode::kR))
	{
		SceneNode& player = getSceneGraph().findSceneNode("player");
		player.getFirstComponentOfType<PlayerControllerComponent>().moveToPosition(Vec3(0.0f, 2.0f, 0.0f));
	}

	if(getInput().getMouseButton(MouseButton::kLeft) == 1)
	{
		ANKI_LOGI("Firing a monkey");

		static U32 instance = 0;

		Transform camTrf =
			getSceneGraph().getActiveCameraNode().getFirstComponentOfType<MoveComponent>().getWorldTransform();

		ModelNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>(
			StringRaii(&getMemoryPool()).sprintf("monkey%u", instance++).toCString(), monkey));
		ANKI_CHECK(monkey->getFirstComponentOfType<ModelComponent>().loadModelResource(
			"Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl"));
		// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(camTrf);

		BodyNode* body;
		ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>(
			StringRaii(&getMemoryPool()).sprintf("bmonkey%u", instance++).toCString(), body));
		ANKI_CHECK(body->getFirstComponentOfType<BodyComponent>().loadMeshResource(
			"Assets/Suzanne_lod0_e3526e1428c0763c.ankimesh"));
		body->getFirstComponentOfType<BodyComponent>().setWorldTransform(camTrf);
		body->getFirstComponentOfType<BodyComponent>().setMass(1.0f);

		PhysicsBodyPtr pbody = body->getFirstComponentOfType<BodyComponent>().getPhysicsBody();
		pbody->applyForce(camTrf.getRotation().getZAxis().xyz() * -1500.0f, Vec3(0.0f, 0.0f, 0.0f));

		body->addChild(monkey);

		// Create the destruction event
		ANKI_CHECK(createDestructionEvent(body));
	}

	if(getInput().getMouseButton(MouseButton::kRight) == 1)
	{
		Transform camTrf =
			getSceneGraph().getActiveCameraNode().getFirstComponentOfType<MoveComponent>().getWorldTransform();
		Vec3 from = camTrf.getOrigin().xyz();
		Vec3 to = from + -camTrf.getRotation().getZAxis() * 100.0f;

		RayCast ray(from, to, PhysicsMaterialBit::kAll & (~PhysicsMaterialBit::kParticle));
		ray.m_firstHit = true;

		getPhysicsWorld().rayCast(ray);

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
			ModelNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode(
				StringRaii(&getSceneGraph().getFrameMemoryPool()).sprintf("decal%u", id++).toCString(), monkey));
			ANKI_CHECK(monkey->getFirstComponentOfType<ModelComponent>().loadModelResource(
				"Assets/Suzanne_dynamic_36043dae41fe12d5.ankimdl"));
			monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

			ANKI_CHECK(createDestructionEvent(monkey));

#if 1
			// Create some particles
			ParticleEmitterNode* particles;
			ANKI_CHECK(getSceneGraph().newSceneNode(
				StringRaii(&getSceneGraph().getFrameMemoryPool()).sprintf("parts%u", id++).toCString(), particles));
			ANKI_CHECK(particles->getFirstComponentOfType<ParticleEmitterComponent>().loadParticleEmitterResource(
				"Assets/Smoke.ankipart"));
			particles->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);
			ANKI_CHECK(createDestructionEvent(particles));
#endif

			// Create some fog volumes
			for(U i = 0; i < 1; ++i)
			{
				static int id = 0;
				StringRaii name(&getSceneGraph().getFrameMemoryPool());
				name.sprintf("fog%u", id++);

				FogDensityNode* fogNode;
				ANKI_CHECK(getSceneGraph().newSceneNode(name.toCString(), fogNode));
				FogDensityComponent& fogComp = fogNode->getFirstComponentOfType<FogDensityComponent>();
				fogComp.setSphereVolumeRadius(2.1f);
				fogComp.setDensity(15.0f);

				fogNode->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

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
