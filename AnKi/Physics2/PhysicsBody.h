// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics2/Common.h>
#include <AnKi/Util/ClassWrapper.h>
#include <Jolt/Physics/Body/Body.h>

namespace anki {

/// @addtogroup physics
/// @{

/// Init info for PhysicsBody.
class PhysicsBodyInitInfo
{
public:
	Physics2CollisionShape* m_shape = nullptr;
	F32 m_mass = 0.0f;
	Transform m_transform = Transform::getIdentity();
	F32 m_friction = 0.5f;
};

/// Rigid body.
class PhysicsBody
{
public:
	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		// TODO
	}

	void applyForce(const Vec3& force, const Vec3& relPos)
	{
		// TODO
	}

	void setMass(F32 mass);

	F32 getMass() const
	{
		return m_mass;
	}

	void activate(Bool activate)
	{
		// TODO
	}

	void clearForces()
	{
		// TODO
	}

	void setLinearVelocity(const Vec3& velocity)
	{
		// TODO
	}

	void setAngularVelocity(const Vec3& velocity)
	{
		// TODO
	}

	void setGravity(const Vec3& gravity)
	{
		// TODO
	}

	void setAngularFactor(const Vec3& factor)
	{
		// TODO
	}

private:
	/// Store the data of the btRigidBody in place to avoid additional allocations.
	ClassWrapper<btRigidBody> m_body;

	Transform m_trf = Transform::getIdentity();
	MotionState m_motionState;

	PhysicsCollisionShapePtr m_shape;

	F32 m_mass = 1.0f;

	PhysicsBody(const PhysicsBodyInitInfo& init);

	~PhysicsBody();

	void registerToWorld() override;

	void unregisterFromWorld() override;
};
/// @}

} // end namespace anki
