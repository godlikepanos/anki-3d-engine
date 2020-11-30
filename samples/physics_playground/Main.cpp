// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include "../common/Framework.h"

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

	return Error::NONE;
}

static Error createFogVolumeFadeEvent(SceneNode* node)
{
	CString script = R"(
density = 15
radius = 3.5

function update(event, prevTime, crntTime)
	node = event:getAssociatedSceneNodes():getAt(0)
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
		fogComponent:setSphere(radius)
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

	return Error::NONE;
}

class RayCast : public PhysicsWorldRayCastCallback
{
public:
	Vec3 m_hitPosition = Vec3(MAX_F32);
	Vec3 m_hitNormal;
	Bool m_hit = false;

	RayCast(Vec3 from, Vec3 to, PhysicsMaterialBit mtl)
		: PhysicsWorldRayCastCallback(from, to, mtl)
	{
	}

	void processResult(PhysicsFilteredObject& obj, const Vec3& worldNormal, const Vec3& worldPosition)
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
	ANKI_CHECK(getResourceManager().loadResource("assets/scene.lua", script));
	ANKI_CHECK(getScriptManager().evalString(script->getSource()));

	// Create the player
	if(1)
	{
		SceneNode& cam = getSceneGraph().getActiveCameraNode();
		cam.getFirstComponentOfType<MoveComponent>().setLocalTransform(
			Transform(Vec4(0.0, 0.0, 5.0, 0.0), Mat3x4::getIdentity(), 1.0));

		PlayerNode* player;
		ANKI_CHECK(getSceneGraph().newSceneNode("player", player, Vec4(0.0f, 2.5f, 0.0f, 0.0f)));
		PlayerControllerComponent& pcomp = player->getFirstComponentOfType<PlayerControllerComponent>();
		pcomp.getPhysicsPlayerController()->setMaterialMask(PhysicsMaterialBit::STATIC_GEOMETRY);

		player->addChild(&cam);
	}

	// Create a body component with joint
	{
		ModelNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>("monkey_p2p", monkey, "assets/Suzanne_dynamic.ankimdl"));

		BodyNode* body;
		ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>("bmonkey_p2p", body, "assets/Suzanne.ankicl"));
		body->getFirstComponentOfType<BodyComponent>().setTransform(
			Transform(Vec4(-0.0f, 4.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f));

		body->addChild(monkey);

		body->getFirstComponentOfType<JointComponent>().newHingeJoint(Vec3(0.2f, 1.0f, 0.0f), Vec3(1, 0, 0));
	}

	// Create a chain
	{
		const U LINKS = 5;

		BodyNode* prevBody = nullptr;
		for(U i = 0; i < LINKS; ++i)
		{
			ModelNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>(
				StringAuto(getAllocator()).sprintf("monkey_chain%u", i).toCString(), monkey,
				"assets/Suzanne_dynamic.ankimdl"));

			Transform trf(Vec4(-4.3f, 12.0f, -3.0f, 0.0f), Mat3x4::getIdentity(), 1.0f);
			trf.getOrigin().y() -= F32(i) * 1.25f;
			// trf.getOrigin().x() -= i * 0.25f;

			// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

			BodyNode* body;
			ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>(
				StringAuto(getAllocator()).sprintf("bmonkey_chain%u", i).toCString(), body, "assets/Suzanne.ankicl"));
			body->getFirstComponentOfType<BodyComponent>().setTransform(trf);

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
		ANKI_CHECK(getSceneGraph().newSceneNode("trigger", node, 1.8f));

		node->getFirstComponentOfType<MoveComponent>().setLocalOrigin(Vec4(1.0f, 0.5f, 0.0f, 0.0f));
	}

	return Error::NONE;
}

Error MyApp::userMainLoop(Bool& quit, Second elapsedTime)
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
		player.getFirstComponentOfType<PlayerControllerComponent>().moveToPosition(Vec4(0.0f, 2.0f, 0.0f, 0.0f));
	}

	if(getInput().getMouseButton(MouseButton::LEFT) == 1)
	{
		ANKI_LOGI("Firing a monkey");

		static U instance = 0;

		Transform camTrf =
			getSceneGraph().getActiveCameraNode().getFirstComponentOfType<MoveComponent>().getWorldTransform();

		ModelNode* monkey;
		ANKI_CHECK(getSceneGraph().newSceneNode<ModelNode>(
			StringAuto(getAllocator()).sprintf("monkey%u", instance++).toCString(), monkey,
			"assets/Suzanne_dynamic.ankimdl"));
		// monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(camTrf);

		BodyNode* body;
		ANKI_CHECK(getSceneGraph().newSceneNode<BodyNode>(
			StringAuto(getAllocator()).sprintf("bmonkey%u", instance++).toCString(), body, "assets/Suzanne.ankicl"));
		body->getFirstComponentOfType<BodyComponent>().setTransform(camTrf);

		PhysicsBodyPtr pbody = body->getFirstComponentOfType<BodyComponent>().getPhysicsBody();
		pbody->applyForce(camTrf.getRotation().getZAxis().xyz() * -1500.0f, Vec3(0.0f, 0.0f, 0.0f));

		body->addChild(monkey);

		// Create the destruction event
		createDestructionEvent(body);
	}

	if(getInput().getMouseButton(MouseButton::RIGHT) == 1)
	{
		Transform camTrf =
			getSceneGraph().getActiveCameraNode().getFirstComponentOfType<MoveComponent>().getWorldTransform();
		Vec3 from = camTrf.getOrigin().xyz();
		Vec3 to = from + -camTrf.getRotation().getZAxis() * 100.0f;

		RayCast ray(from, to, PhysicsMaterialBit::ALL & (~PhysicsMaterialBit::PARTICLE));
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
			static U id = 0;
			ModelNode* monkey;
			ANKI_CHECK(getSceneGraph().newSceneNode(
				StringAuto(getSceneGraph().getFrameAllocator()).sprintf("decal%u", id++).toCString(), monkey,
				"assets/Suzanne_dynamic.ankimdl"));
			monkey->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

			createDestructionEvent(monkey);

#if 1
			// Create some particles
			ParticleEmitterNode* particles;
			ANKI_CHECK(getSceneGraph().newSceneNode(
				StringAuto(getSceneGraph().getFrameAllocator()).sprintf("parts%u", id++).toCString(), particles,
				"assets/smoke.ankipart"));
			particles->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);
			createDestructionEvent(particles);
#endif

			// Create some fog volumes
			for(U i = 0; i < 1; ++i)
			{
				static int id = 0;
				StringAuto name(getSceneGraph().getFrameAllocator());
				name.sprintf("fog%u", id++);

				FogDensityNode* fogNode;
				ANKI_CHECK(getSceneGraph().newSceneNode(name.toCString(), fogNode));
				FogDensityComponent& fogComp = fogNode->getFirstComponentOfType<FogDensityComponent>();
				fogComp.setSphere(2.1f);
				fogComp.setDensity(15.0f);

				fogNode->getFirstComponentOfType<MoveComponent>().setLocalTransform(trf);

				createDestructionEvent(fogNode);
				createFogVolumeFadeEvent(fogNode);
			}
		}
	}

	if(0)
	{
		SceneNode& node = getSceneGraph().findSceneNode("trigger");
		TriggerComponent& comp = node.getFirstComponentOfType<TriggerComponent>();

		for(U32 i = 0; i < comp.getContactSceneNodes().getSize(); ++i)
		{
			ANKI_LOGI("Touching %s", comp.getContactSceneNodes()[i]->getName().cstr());
		}
	}

	return Error::NONE;
}

int main(int argc, char* argv[])
{
	Error err = Error::NONE;

	MyApp* app = new MyApp;
	err = app->init(argc, argv, "physics_playground");
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
