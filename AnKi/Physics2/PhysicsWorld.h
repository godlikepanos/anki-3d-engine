// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Physics2/PhysicsCollisionShape.h>
#include <AnKi/Util/BlockArray.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace anki {

/// @addtogroup physics
/// @{

/// The master container for all physics related stuff.
class Physics2World : public MakeSingleton<Physics2World>
{
	template<typename>
	friend class MakeSingleton;
	friend class Physics2CollisionShapePtrDeleter;

public:
	Error init(AllocAlignedCallback allocCb, void* allocCbData);

	Physics2CollisionShapePtr newSphereCollisionShape(F32 radius);
	Physics2CollisionShapePtr newBoxCollisionShape(Vec3 extend);

private:
	BlockArray<Physics2CollisionShape, SingletonMemoryPoolWrapper<Physics2MemoryPool>> m_collisionShapes;

	JPH::PhysicsSystem m_jphPhysicsSystem;

	Physics2World();

	~Physics2World();

	void destroyCollisionShape(Physics2CollisionShape* ptr);
};
/// @}

} // end namespace anki
