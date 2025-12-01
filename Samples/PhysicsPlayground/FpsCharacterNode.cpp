// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Samples/PhysicsPlayground/FpsCharacterNode.h>
#include <Samples/PhysicsPlayground/GrenadeNode.h>
#include <Samples/PhysicsPlayground/Events.h>

static void createFogVolumeFadeEvent(SceneNode* node)
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
}

FpsCharacter::FpsCharacter(CString name)
	: SceneNode(name)
{
	newComponent<PlayerControllerComponent>();

	SceneNode* cam = SceneGraph::getSingleton().newSceneNode<SceneNode>("FpsCharacterCam");
	cam->setLocalTransform(Transform(Vec3(0.0f, 2.0f, 0.0f), Mat3::getIdentity(), Vec3(1.0f)));
	CameraComponent* camc = cam->newComponent<CameraComponent>();
	camc->setPerspective(0.1f, 1000.0f, Renderer::getSingleton().getAspectRatio() * toRad<F32>(g_cvarGameFov), toRad<F32>(g_cvarGameFov));
	addChild(cam);
	m_cameraNode = cam;

	SceneNode* shotgun = SceneGraph::getSingleton().newSceneNode<SceneNode>("Shotgun");
	shotgun->setLocalTransform(Transform(m_shotgunRestingPosition, Mat3(m_shotgunRestingRotation), Vec3(1.0f, 1.0f, 0.45f)));
	shotgun->newComponent<MeshComponent>()->setMeshFilename("Assets/sleevegloveLOW.001_893660395596b206.ankimesh");
	shotgun->newComponent<MaterialComponent>()->setMaterialFilename("Assets/arms_3a4232ebbd425e7a.ankimtl").setSubmeshIndex(0);
	shotgun->newComponent<MaterialComponent>()->setMaterialFilename("Assets/boomstick_89a614a521ace7fd.ankimtl").setSubmeshIndex(1);
	cam->addChild(shotgun);
	m_shotgunNode = shotgun;
}

void FpsCharacter::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	// Change FOV
	CameraComponent& camc = m_cameraNode->getFirstComponentOfType<CameraComponent>();
	if(toRad<F32>(g_cvarGameFov) != camc.getFovY())
	{
		camc.setFovX(Renderer::getSingleton().getAspectRatio() * toRad<F32>(g_cvarGameFov));
		camc.setFovY(toRad<F32>(g_cvarGameFov));
	}

	// Mouselook
	const Input& inp = Input::getSingleton();
	const Vec2 mousePos = inp.getMousePositionNdc();
	if(mousePos != 0.0f)
	{
		Mat3 camRot = m_cameraNode->getLocalRotation();

		Mat3 newRot(Euler(g_cvarGameMouseLookPower * mousePos.y(), g_cvarGameMouseLookPower * -mousePos.x(), 0.0f));
		newRot = camRot * newRot;

		const Vec3 newz = newRot.getColumn(2).normalize();
		const Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
		const Vec3 newy = newz.cross(newx);
		newRot.setColumns(newx, newy, newz);
		newRot = newRot.reorthogonalize();

		m_cameraNode->setLocalRotation(newRot);
	}

	// Movement
	{
		Vec3 moveVec(0.0);
		if(inp.getKey(KeyCode::kW) > 0)
		{
			moveVec.z() += 1.0f;
		}

		if(inp.getKey(KeyCode::kA) > 0)
		{
			moveVec.x() += 1.0f;
		}

		if(inp.getKey(KeyCode::kS) > 0)
		{
			moveVec.z() -= 1.0f;
		}

		if(inp.getKey(KeyCode::kD) > 0)
		{
			moveVec.x() -= 1.0f;
		}

		F32 jumpSpeed = 0.0f;
		if(inp.getKey(KeyCode::kSpace) > 0)
		{
			jumpSpeed += m_jumpSpeed;
		}

		Bool crouchChanged = false;
		if(inp.getKey(KeyCode::kC) > 0)
		{
			m_crouching = !m_crouching;
			crouchChanged = true;
		}

		if(moveVec != 0.0f || jumpSpeed != 0.0f || crouchChanged)
		{
			Vec3 dir;
			if(moveVec != 0.0f)
			{
				dir = -(m_cameraNode->getLocalRotation() * moveVec);
				dir.y() = 0.0f;
				dir = dir.normalize();
			}

			F32 speed = m_walkingSpeed;
			if(inp.getKey(KeyCode::kLeftShift) > 0)
			{
				speed *= 2.0f;
			}

			PlayerControllerComponent& playerc = getFirstComponentOfType<PlayerControllerComponent>();
			playerc.setVelocity(speed, jumpSpeed, dir, m_crouching);
		}
	}

	// Animate gun back to resting position
	if(m_shotgunNode->getLocalOrigin() != m_shotgunRestingPosition)
	{
		const Vec3 gunPos = m_shotgunNode->getLocalOrigin();

		Vec3 travelDir = m_shotgunRestingPosition - gunPos;
		const F32 remainlingDist = travelDir.length();
		travelDir = travelDir.normalize();

		const F32 speed = getRandomRange(0.2f, 0.4f);
		const F32 dist = speed * F32(crntTime - prevUpdateTime);

		if(dist >= remainlingDist)
		{
			m_shotgunNode->setLocalOrigin(m_shotgunRestingPosition);
			m_shotgunNode->setLocalRotation(Mat3(m_shotgunRestingRotation));
		}
		else
		{
			m_shotgunNode->setLocalOrigin(travelDir * dist + gunPos);

			const F32 distFactor = dist / remainlingDist;
			const Quat crntAngle(m_shotgunNode->getLocalRotation());
			const Quat restingAngle(m_shotgunRestingRotation);
			const Quat newAngle = crntAngle.slerp(restingAngle, distFactor);
			m_shotgunNode->setLocalRotation(Mat3(newAngle));
		}
	}

	// Shooting
	if(inp.getMouseButton(MouseButton::kLeft) == 1)
	{
		fireShotgun();

		const Vec3 newPosition(0.0f, getRandomRange(-0.05f, -0.03f), 0.15f);
		m_shotgunNode->setLocalOrigin(m_shotgunRestingPosition + newPosition);
		const Euler newRotation(getRandomRange(0.0_degrees, 10.0_degrees), getRandomRange(-10.0_degrees, 1.0_degrees), 0.0f);
		m_shotgunNode->setLocalRotation(Mat3(newRotation) * Mat3(m_shotgunRestingRotation));
	}

	if(inp.getMouseButton(MouseButton::kRight) == 1)
	{
		fireGrenade();
	}
}

void FpsCharacter::fireShotgun()
{
	for(U32 i = 0; i < 8; ++i)
	{
		F32 spreadAngle = getRandomRange(-m_shotgunSpreadAngle / 2.0f, m_shotgunSpreadAngle / 2.0f);
		Mat3 randDirection(Axisang(spreadAngle, Vec3(1.0f, 0.0f, 0.0f)));

		spreadAngle = getRandomRange(-m_shotgunSpreadAngle / 2.0f, m_shotgunSpreadAngle / 2.0f);
		randDirection = randDirection * Mat3(Axisang(spreadAngle, Vec3(0.0f, 1.0f, 0.0f)));
		randDirection = m_cameraNode->getLocalRotation().getRotationPart() * randDirection;

		const Vec3 from = m_cameraNode->getWorldTransform().getOrigin().xyz();
		const Vec3 to = from + -randDirection.getZAxis() * m_shotgunMaxLength;

		RayHitResult result;
		const Bool hit = PhysicsWorld::getSingleton().castRayClosestHit(from, to, PhysicsLayerBit::kStatic | PhysicsLayerBit::kMoving, result);

		if(hit && result.m_object->getType() == PhysicsObjectType::kBody)
		{
			PhysicsBody& body = static_cast<PhysicsBody&>(*result.m_object);
			const Bool isStatic = body.getMass() == 0.0f;

			if(isStatic)
			{
				// Create rotation
				const Vec3& zAxis = result.m_normal;
				Vec3 yAxis = Vec3(0, 1, 0.5);
				Vec3 xAxis = yAxis.cross(zAxis).normalize();
				yAxis = zAxis.cross(xAxis);

				Mat3 rot;
				rot.setXAxis(xAxis);
				rot.setYAxis(yAxis);
				rot.setZAxis(zAxis);

				const Transform trf(result.m_hitPosition, rot, Vec3(1.0f, 1.0f, 1.0f));

				// Create an obj
				SceneNode* bulletDecal = SceneGraph::getSingleton().newSceneNode<SceneNode>("");
				bulletDecal->setLocalTransform(trf);
				bulletDecal->setLocalScale(Vec3(0.1f, 0.1f, 0.3f));
				DecalComponent* decalc = bulletDecal->newComponent<DecalComponent>();
				decalc->loadDiffuseImageResource("Assets/bullet_hole_decal.ankitex", 1.0f);

				createDestructionEvent(bulletDecal, 10.0_sec);
			}
			else
			{
				const Vec3 force = -result.m_normal * m_shotgunForce;
				body.applyForce(force);
			}

			if(i == 0)
			{
				SceneNode* fogNode = SceneGraph::getSingleton().newSceneNode<SceneNode>("");
				FogDensityComponent* fogComp = fogNode->newComponent<FogDensityComponent>();
				fogNode->setLocalScale(Vec3(2.1f));
				fogComp->setDensity(15.0f);

				fogNode->setLocalOrigin(result.m_hitPosition);

				createDestructionEvent(fogNode, 10.0_sec);
				createFogVolumeFadeEvent(fogNode);
			}
		}
	}
}

void FpsCharacter::fireGrenade()
{
	Transform camTrf = m_cameraNode->getWorldTransform();
	const Vec3 newPos = camTrf.getOrigin().xyz() + camTrf.getRotation().getZAxis() * -3.0f;
	camTrf.setOrigin(newPos.xyz0());

	SceneNode* grenade = SceneGraph::getSingleton().newSceneNode<GrenadeNode>("");
	grenade->setLocalOrigin(camTrf.getOrigin().xyz());
	grenade->setLocalRotation(camTrf.getRotation().getRotationPart());
	BodyComponent& bodyc = grenade->getFirstComponentOfType<BodyComponent>();
	bodyc.applyForce(camTrf.getRotation().getZAxis().xyz() * -1200.0f, Vec3(0.0f, 0.0f, 0.0f));
}
