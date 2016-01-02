// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki {

//==============================================================================
// PhysicsCollisionShape                                                       =
//==============================================================================

//==============================================================================
I32 PhysicsCollisionShape::m_gid = 1;

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

//==============================================================================
// PhysicsTriangleSoup                                                         =
//==============================================================================

//==============================================================================
Error PhysicsTriangleSoup::create(Initializer& init,
	const Vec3* positions, U32 positionsStride, 
	const U16* indices, U32 indicesCount)
{
	m_shape = NewtonCreateTreeCollision(m_world->_getNewtonWorld(), 0);
	if(!m_shape)
	{
		ANKI_LOGE("NewtonCreateBox() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	NewtonTreeCollisionBeginBuild(m_shape);

	// Iterate index array
	const U16* indicesEnd = indices + indicesCount;
	for(; indices != indicesEnd; indices += 3)
	{
		Array<Vec3, 3> facePos;

		for(U i = 0; i < 3; ++i)
		{
			U idx = indices[i];
			const U8* ptr = 
				reinterpret_cast<const U8*>(positions) + positionsStride * idx;

			facePos[i] = *reinterpret_cast<const Vec3*>(ptr);
		}

		NewtonTreeCollisionAddFace(m_shape, 3, &facePos[0][0], sizeof(Vec3), 0);
	}

	const I optimize = !ANKI_DEBUG;
	NewtonTreeCollisionEndBuild(m_shape, optimize);

	return ErrorCode::NONE;
}

} // end namespace anki

