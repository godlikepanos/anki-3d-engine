// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Physics2/PhysicsCollisionShape.h>
#include <AnKi/Physics2/PhysicsBody.h>
#include <AnKi/Physics2/PhysicsJoint.h>
#include <AnKi/Physics2/PhysicsPlayerController.h>
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
	friend class PhysicsPlayerController;
	friend class PhysicsPlayerControllerPtrDeleter;
	friend class PhysicsJointPtrDeleter;

public:
	Error init(AllocAlignedCallback allocCb, void* allocCbData);

	PhysicsCollisionShapePtr newSphereCollisionShape(F32 radius);
	PhysicsCollisionShapePtr newBoxCollisionShape(Vec3 extend);
	PhysicsCollisionShapePtr newCapsuleCollisionShape(F32 height, F32 radius); ///< Capsule axis is in Y.

	PhysicsBodyPtr newPhysicsBody(const PhysicsBodyInitInfo& init);

	PhysicsJointPtr newPointJoint(PhysicsBody* body1, PhysicsBody* body2, Bool pointsInWorldSpace, const Vec3& body1Point, const Vec3& body2Point);

	PhysicsPlayerControllerPtr newPlayerController(const PhysicsPlayerControllerInitInfo& init);

	void update(Second dt);

private:
	class MyBodyActivationListener final : public JPH::BodyActivationListener
	{
	public:
		virtual void OnBodyActivated(const JPH::BodyID& inBodyID, U64 inBodyUserData) override;
		virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, U64 inBodyUserData) override;
	};

	template<typename T>
	class ObjArray
	{
	public:
		PhysicsBlockArray<T> m_array;
		Mutex m_mtx;
	};

	ClassWrapper<JPH::PhysicsSystem> m_jphPhysicsSystem;
	ClassWrapper<JPH::JobSystemThreadPool> m_jobSystem;
	ClassWrapper<JPH::TempAllocatorImpl> m_tempAllocator;

	ObjArray<PhysicsCollisionShape> m_collisionShapes;
	ObjArray<PhysicsBody> m_bodies;
	ObjArray<PhysicsJoint> m_joints;
	ObjArray<PhysicsPlayerController> m_characters;

	Bool m_optimizeBroadphase = true;

	PhysicsWorld();

	~PhysicsWorld();

	template<typename TJPHCollisionShape, typename... TArgs>
	PhysicsCollisionShapePtr newCollisionShape(PhysicsCollisionShape::ShapeType type, TArgs&&... args);

	template<typename TJPHJoint, typename... TArgs>
	PhysicsJointPtr newJoint(PhysicsJoint::Type type, PhysicsBody* body1, PhysicsBody* body2, TArgs&&... args);
};
/// @}

} // namespace v2
} // end namespace anki
