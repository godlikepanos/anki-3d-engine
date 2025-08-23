// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/AnKi.h>

using namespace anki;

class FpsCharacter : public SceneNode
{
public:
	F32 m_mouseLookPower = toRad(7.0f);
	F32 m_walkingSpeed = 8.5f;
	F32 m_jumpSpeed = 8.0f;
	Bool m_crouching = false;

	FpsCharacter()
		: SceneNode("FpsCharacter")
	{
		PlayerControllerComponent* playerc = newComponent<PlayerControllerComponent>();

		SceneNode* cam = SceneGraph::getSingleton().newSceneNode<SceneNode>("FpsCharacterCam");
		cam->setLocalTransform(Transform(Vec3(0.0f, 2.0f, 0.0f), Mat3::getIdentity(), Vec3(1.0f)));
		addChild(cam);

		SceneNode* shotgun = SceneGraph::getSingleton().newSceneNode<SceneNode>("Shotgun");
		shotgun->setLocalTransform(Transform(Vec3(0.065f, -0.13f, -0.4f), Mat3(Euler(0.0f, kPi, 0.0f)), Vec3(1.0f)));
		shotgun->newComponent<MeshComponent>()->setMeshFilename("sleevegloveLOW.001_893660395596b206.ankimesh");
		shotgun->newComponent<MaterialComponent>()->setMaterialFilename("arms_3a4232ebbd425e7a.ankimtl").setSubmeshIndex(0);
		shotgun->newComponent<MaterialComponent>()->setMaterialFilename("boomstick_89a614a521ace7fd.ankimtl").setSubmeshIndex(1);
		cam->addChild(shotgun);
	}

	void frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime) override
	{
		PlayerControllerComponent& playerc = getFirstComponentOfType<PlayerControllerComponent>();

		// Rotate
		F32 y = Input::getSingleton().getMousePosition().y();
		F32 x = Input::getSingleton().getMousePosition().x();
		if(y != 0.0 || x != 0.0)
		{
			// Set rotation
			Mat3 rot(Euler(m_mouseLookPower * y, m_mouseLookPower * x, 0.0f));

			rot = getLocalRotation() * rot;

			const Vec3 newz = rot.getColumn(2).normalize();
			const Vec3 newx = Vec3(0.0, 1.0, 0.0).cross(newz);
			const Vec3 newy = newz.cross(newx);
			rot.setColumns(newx, newy, newz);
			rot = rot.reorthogonalize();

			// Update move
			setLocalRotation(rot);
		}

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
			jumpSpeed += m_jumpSpeed;
		}

		Bool crouchChanged = false;
		if(Input::getSingleton().getKey(KeyCode::kC))
		{
			m_crouching = !m_crouching;
			crouchChanged = true;
		}

		if(moveVec != 0.0f || jumpSpeed != 0.0f || crouchChanged)
		{
			Vec3 dir;
			if(moveVec != 0.0f)
			{
				dir = -(getLocalRotation() * moveVec);
				dir.y() = 0.0f;
				dir = dir.normalize();
			}

			F32 speed = m_walkingSpeed;
			if(Input::getSingleton().getKey(KeyCode::kLeftShift))
			{
				speed *= 2.0f;
			}
			playerc.setVelocity(speed, jumpSpeed, dir, m_crouching);
		}
	}
};
