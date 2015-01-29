// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsBody.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/physics/PhysicsCollisionShape.h"

namespace anki {

//==============================================================================
PhysicsBody::PhysicsBody(PhysicsWorld* world)
:	PhysicsObject(Type::BODY, world)
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
	ANKI_ASSERT(init.m_shape);

	//I collisionType = NewtonCollisionGetType(init.m_shape->_getNewtonShape());

	// Create
	Mat4 trf = Mat4(init.m_startTrf);
	trf.transpose();
	if(init.m_kinematic)
	{
		// TODO
	}
	else
	{
		m_body = NewtonCreateDynamicBody(m_world->_getNewtonWorld(),
			init.m_shape->_getNewtonShape(), &trf(0, 0));
	}

	if(!m_body)
	{
		ANKI_LOGE("NewtonCreateXXBody() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// User data & callbacks
	NewtonBodySetUserData(m_body, this);
	NewtonBodySetTransformCallback(m_body, onTransformCallback);

	// Set mass
	NewtonCollision* shape = NewtonBodyGetCollision(m_body);
	NewtonBodySetMassProperties(m_body, init.m_mass, shape);

	// Set gravity
	if(init.m_gravity)
	{
		NewtonBodySetForceAndTorqueCallback(m_body, applyGravityForce);
	}

	// Activate
	NewtonBodySetSimulationState(m_body, true);

	return ErrorCode::NONE;
}

//==============================================================================
void PhysicsBody::setTransform(const Transform& trf)
{
	Mat4 mat(trf);
	mat.transpose();
	NewtonBodySetMatrix(m_body, &mat(0, 0));
}

//==============================================================================
void PhysicsBody::onTransformCallback(
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
	trf.transpose();
	trf(3, 3) = 0.0;
	self->m_trf = Transform(trf);
	self->m_updated = true;
}

//==============================================================================
void PhysicsBody::applyGravityForce(
	const NewtonBody* body, 
	dFloat timestep, 
	int threadIndex)
{
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;

	NewtonBodyGetMassMatrix(body, &mass, &Ixx, &Iyy, &Izz);

	const F32 GRAVITY = -9.8;
	Vec4 force(0.0, mass * GRAVITY, 0.0, 0.0);
	NewtonBodySetForce(body, &force[0]);
}

} // end namespace anki
