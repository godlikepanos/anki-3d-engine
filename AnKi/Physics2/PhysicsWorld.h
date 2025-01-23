// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Physics2/PhysicsCollisionShape.h>
#include <AnKi/Physics2/PhysicsBody.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {
namespace v2 {

/// @addtogroup physics
/// @{

/// The master container for all physics related stuff.
class PhysicsWorld : public MakeSingleton<PhysicsWorld>
{
	template<typename>
	friend class MakeSingleton;
	friend class PhysicsCollisionShapePtrDeleter;
	friend class PhysicsBodyPtrDeleter;
	friend class PhysicsBody;

public:
	Error init(AllocAlignedCallback allocCb, void* allocCbData);

	PhysicsCollisionShapePtr newSphereCollisionShape(F32 radius);
	PhysicsCollisionShapePtr newBoxCollisionShape(Vec3 extend);

	PhysicsBodyPtr newPhysicsBody(const PhysicsBodyInitInfo& init);

	void update(Second dt);

private:
	class MyBodyActivationListener final : public JPH::BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const JPH::BodyID& inBodyID, U64 inBodyUserData) override;
		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, U64 inBodyUserData) override;
	};

	PhysicsBlockArray<PhysicsCollisionShape> m_collisionShapes;
	PhysicsBlockArray<PhysicsBody> m_bodies;

	ClassWrapper<JPH::PhysicsSystem> m_jphPhysicsSystem;
	ClassWrapper<JPH::JobSystemThreadPool> m_jobSystem;
	ClassWrapper<JPH::TempAllocatorImpl> m_tempAllocator;

	// Add bodies and remove bodies in batches for perf reasons
	PhysicsDynamicArray<PhysicsBody*> m_bodiesToAdd;
	PhysicsDynamicArray<PhysicsBody*> m_bodiesToRemove;

	Mutex m_mtx;

	PhysicsWorld();

	~PhysicsWorld();

	void addBodies();
	void removeBodies();

	template<typename TJPHCollisionShape, typename... TArgs>
	PhysicsCollisionShapePtr newCollisionShape(PhysicsCollisionShape::ShapeType type, TArgs&&... args);
};
/// @}

} // namespace v2
} // end namespace anki
