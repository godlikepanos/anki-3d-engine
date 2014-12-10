// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsBody.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/physics/PhysicsCollisionShape.h"

namespace anki {

//==============================================================================
PhysicsBody::PhysicsBody()
{}

//==============================================================================
PhysicsBody::~PhysicsBody()
{
	if(m_body)
	{
		NewtonDestroyBody(m_body);
	}
}

//==============================================================================
Error PhysicsBody::create(const Initializer& init)
{
	ANKI_ASSERT(init.m_world);
	ANKI_ASSERT(init.m_shape);

	// Create
	Mat4 trf = Mat4(init.m_startTrf);
	if(init.m_kinematic)
	{
		// TODO
	}
	else
	{
		m_body = NewtonCreateDynamicBody(init.m_world->_getNewtonWorld(),
			init.m_shape->_getNewtonShape(), &trf(0, 0));
	}

	if(!m_body)
	{
		ANKI_LOGE("NewtonCreateXXBody() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// User data & callbacks
	NewtonBodySetUserData(m_body, this);
	NewtonBodySetTransformCallback(m_body, onTransform);

	// Set mass
	NewtonCollision* shape = NewtonBodyGetCollision(m_body);
	NewtonBodySetMassProperties(m_body, init.m_mass, shape);

	// Set gravity
	if(init.m_gravity)
	{
		Vec3 gravityForce(0.0, -9.8f * init.m_mass, 0.0);
		NewtonBodySetForce(m_body, &gravityForce[0]);
	}

	// Activate
	NewtonBodySetSimulationState(m_body, true);

	return ErrorCode::NONE;
}

//==============================================================================
void PhysicsBody::onTransform(
	const NewtonBody* const body, 
	const dFloat* const matrix, 
	int threadIndex)
{
	ANKI_ASSERT(body);
	ANKI_ASSERT(matrix);

	void* ud = NewtonBodyGetUserData(body);
	ANKI_ASSERT(ud);

	PhysicsBody* self = reinterpret_cast<PhysicsBody*>(ud);

	Mat4 trf;
	memcpy(&trf, matrix, sizeof(Mat4));
	self->m_trf = Transform(trf);
}

} // end namespace anki
