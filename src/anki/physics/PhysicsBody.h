// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// Init info for PhysicsBody.
class PhysicsBodyInitInfo
{
public:
	PhysicsCollisionShapePtr m_shape;
	F32 m_mass = 0.0f;
	Transform m_transform = Transform::getIdentity();
	F32 m_friction = 0.5f;
};

/// Rigid body.
class PhysicsBody : public PhysicsFilteredObject
{
	ANKI_PHYSICS_OBJECT

public:
	static const PhysicsObjectType CLASS_TYPE = PhysicsObjectType::BODY;

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_trf = trf;
		m_body->setWorldTransform(toBt(trf));
	}

	void applyForce(const Vec3& force, const Vec3& relPos)
	{
		m_body->applyForce(toBt(force), toBt(relPos));
	}

	void setMass(F32 mass);

	F32 getMass() const
	{
		return m_mass;
	}

	void activate(Bool activate)
	{
		m_body->forceActivationState((activate) ? ACTIVE_TAG : DISABLE_SIMULATION);
		if(activate)
		{
			m_body->activate(true);
		}
	}

	void clearForces()
	{
		m_body->clearForces();
	}

	void setLinearVelocity(const Vec3& velocity)
	{
		m_body->setLinearVelocity(toBt(velocity));
	}

	void setAngularVelocity(const Vec3& velocity)
	{
		m_body->setAngularVelocity(toBt(velocity));
	}

	void setGravity(const Vec3& gravity)
	{
		m_body->setGravity(toBt(gravity));
	}

	void setAngularFactor(const Vec3& factor)
	{
		m_body->setAngularFactor(toBt(factor));
	}

anki_internal:
	const btRigidBody* getBtBody() const
	{
		return m_body.get();
	}

	btRigidBody* getBtBody()
	{
		return m_body.get();
	}

private:
	class MotionState : public btMotionState
	{
	public:
		PhysicsBody* m_body = nullptr;

		void getWorldTransform(btTransform& worldTrans) const override
		{
			worldTrans = toBt(m_body->m_trf);
		}

		void setWorldTransform(const btTransform& worldTrans) override
		{
			m_body->m_trf = toAnki(worldTrans);
		}
	};

	/// Store the data of the btRigidBody in place to avoid additional allocations.
	BtClassWrapper<btRigidBody> m_body;

	Transform m_trf = Transform::getIdentity();
	MotionState m_motionState;

	PhysicsCollisionShapePtr m_shape;

	F32 m_mass = 1.0f;

	PhysicsBody(PhysicsWorld* world, const PhysicsBodyInitInfo& init);

	~PhysicsBody();
};
/// @}

} // end namespace anki
