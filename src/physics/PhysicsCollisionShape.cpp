// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsCollisionShape.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
// PhysicsCollisionShape                                                       =
//==============================================================================

//==============================================================================
I32 PhysicsCollisionShape::m_gid = 0;

//==============================================================================
PhysicsCollisionShape::~PhysicsCollisionShape()
{
	if(m_shape)
	{
		NewtonDestroyCollision(m_shape);
	}
}

//==============================================================================
// PhysicsSphere                                                               =
//==============================================================================

//==============================================================================
Error PhysicsSphere::create(Initializer& init, F32 radius)
{
	Error err = ErrorCode::NONE;

	m_shape = NewtonCreateSphere(
		m_world->_getNewtonWorld(), radius, m_gid++, nullptr);
	if(!m_shape)
	{
		ANKI_LOGE("NewtonCreateSphere() failed");
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

//==============================================================================
// PhysicsBox                                                                  =
//==============================================================================

//==============================================================================
Error PhysicsBox::create(Initializer& init, const Vec3& extend)
{
	Error err = ErrorCode::NONE;

	m_shape = NewtonCreateBox(
		m_world->_getNewtonWorld(), extend.x(), extend.y(), extend.z(),
		m_gid++, nullptr);
	if(!m_shape)
	{
		ANKI_LOGE("NewtonCreateBox() failed");
		err = ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

} // end namespace anki

