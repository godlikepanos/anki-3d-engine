// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsCollisionShape.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
I32 PhysicsCollisionShape::id = 0;

//==============================================================================
PhysicsCollisionShape::PhysicsCollisionShape()
{}

//==============================================================================
PhysicsCollisionShape::~PhysicsCollisionShape()
{
	if(m_shape)
	{
		NewtonDestroyCollision(m_shape);
	}
}

//==============================================================================
Error PhysicsCollisionShape::createSphere(Initializer& init, F32 radius)
{
	ANKI_ASSERT(init.m_world);
	Error err = ErrorCode::NONE;

	m_shape = NewtonCreateSphere(
		init.m_world->_getNewtonWorld(), radius, id++, nullptr);
	if(!m_shape)
	{
		ANKI_LOGE("NewtonCreateSphere() failed");
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
Error PhysicsCollisionShape::createBox(Initializer& init, const Vec3& extend)
{
	ANKI_ASSERT(init.m_world);
	Error err = ErrorCode::NONE;

	m_shape = NewtonCreateBox(
		init.m_world->_getNewtonWorld(), extend.x(), extend.y(), extend.z(),
		id++, nullptr);
	if(!m_shape)
	{
		ANKI_LOGE("NewtonCreateBox() failed");
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

} // end namespace anki

