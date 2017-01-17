// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

CharacterControllerManager::CharacterControllerManager(PhysicsWorld* world)
	: CustomPlayerControllerManager(world->getNewtonWorld())
	, m_world(world)
{
}

void CharacterControllerManager::ApplyPlayerMove(CustomPlayerController* const controller, dFloat timestep)
{
	ANKI_ASSERT(controller);

	NewtonBody* body = controller->GetBody();
	PhysicsPlayerController* player = static_cast<PhysicsPlayerController*>(NewtonBodyGetUserData(body));
	ANKI_ASSERT(player);

	dVector gravity = toNewton(player->getWorld().getGravity());

	// Compute the angle the way newton wants it
	Vec4 forwardDir{player->m_forwardDir.x(), 0.0, player->m_forwardDir.z(), 0.0f};
	F32 cosTheta = clamp(Vec4(0.0, 0.0, -1.0, 0.0).dot(forwardDir), -1.0f, 1.0f);
	F32 sign = Vec4(0.0, 0.0, -1.0, 0.0).cross(forwardDir).y();
	sign = (!isZero(sign)) ? (absolute(sign) / sign) : 1.0f;
	F32 angle = acos(cosTheta) * sign;

	controller->SetPlayerVelocity(
		player->m_forwardSpeed, player->m_strafeSpeed, player->m_jumpSpeed, angle, gravity, timestep);
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	if(m_newtonPlayer)
	{
		getWorld().getCharacterControllerManager().DestroyController(m_newtonPlayer);
	}
}

Error PhysicsPlayerController::create(const PhysicsPlayerControllerInitInfo& init)
{
	dMatrix playerAxis;
	playerAxis[0] = dVector(0.0f, 1.0f, 0.0f, 0.0f); // the y axis is the character up vector
	playerAxis[1] = dVector(0.0f, 0.0f, -1.0f, 0.0f); // the x axis is the character front direction
	playerAxis[2] = playerAxis[0].CrossProduct(playerAxis[1]);
	playerAxis[3] = dVector(0.0f, 0.0f, 0.0f, 1.0f);

	m_newtonPlayer = getWorld().getCharacterControllerManager().CreatePlayer(
		init.m_mass, init.m_outerRadius, init.m_innerRadius, init.m_height, init.m_stepHeight, playerAxis);

	if(m_newtonPlayer == nullptr)
	{
		return ErrorCode::FUNCTION_FAILED;
	}

	// Set some data
	NewtonBody* body = m_newtonPlayer->GetBody();
	NewtonBodySetUserData(body, this);
	NewtonBodySetTransformCallback(body, onTransformUpdateCallback);

	dMatrix location(dGetIdentityMatrix());
	location.m_posit.m_x = init.m_position.x();
	location.m_posit.m_y = init.m_position.y();
	location.m_posit.m_z = init.m_position.z();
	NewtonBodySetMatrix(body, &location[0][0]);

	return ErrorCode::NONE;
}

void PhysicsPlayerController::moveToPosition(const Vec4& position)
{
	ANKI_ASSERT(!"TODO");
}

} // end namespace anki
