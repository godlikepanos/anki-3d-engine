// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

I32 PhysicsCollisionShape::m_gid = 1;

PhysicsCollisionShape::~PhysicsCollisionShape()
{
	if(m_shape)
	{
		NewtonDestroyCollision(m_shape);
	}
}

Error PhysicsSphere::create(PhysicsCollisionShapeInitInfo& init, F32 radius)
{
	Error err = Error::NONE;

	m_shape = NewtonCreateSphere(m_world->getNewtonWorld(), radius, m_gid++, nullptr);
	if(!m_shape)
	{
		ANKI_PHYS_LOGE("NewtonCreateSphere() failed");
		err = Error::FUNCTION_FAILED;
	}

	return err;
}

Error PhysicsBox::create(PhysicsCollisionShapeInitInfo& init, const Vec3& extend)
{
	Error err = Error::NONE;

	m_shape = NewtonCreateBox(m_world->getNewtonWorld(), extend.x(), extend.y(), extend.z(), m_gid++, nullptr);
	if(!m_shape)
	{
		ANKI_PHYS_LOGE("NewtonCreateBox() failed");
		err = Error::FUNCTION_FAILED;
	}

	return err;
}

Error PhysicsTriangleSoup::create(PhysicsCollisionShapeInitInfo& init,
	const Vec3* positions,
	U32 positionsStride,
	const U32* indices,
	U32 indicesCount)
{
	m_shape = NewtonCreateTreeCollision(m_world->getNewtonWorld(), 0);
	if(!m_shape)
	{
		ANKI_PHYS_LOGE("NewtonCreateBox() failed");
		return Error::FUNCTION_FAILED;
	}

	NewtonTreeCollisionBeginBuild(m_shape);

	// Iterate index array
	const U32* indicesEnd = indices + indicesCount;
	for(; indices != indicesEnd; indices += 3)
	{
		Array<Vec3, 3> facePos;

		for(U i = 0; i < 3; ++i)
		{
			U idx = indices[i];
			const U8* ptr = reinterpret_cast<const U8*>(positions) + positionsStride * idx;

			facePos[i] = *reinterpret_cast<const Vec3*>(ptr);
		}

		NewtonTreeCollisionAddFace(m_shape, 3, &facePos[0][0], sizeof(Vec3), 0);
	}

	const I optimize = 1;
	NewtonTreeCollisionEndBuild(m_shape, optimize);

	return Error::NONE;
}

} // end namespace anki
