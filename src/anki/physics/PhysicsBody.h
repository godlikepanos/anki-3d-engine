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
		getBtBody()->setWorldTransform(toBt(trf));
	}

	void applyForce(const Vec3& force, const Vec3& relPos)
	{
		getBtBody()->applyForce(toBt(force), toBt(relPos));
	}

	void setMass(F32 mass);

	F32 getMass() const
	{
		return m_mass;
	}

anki_internal:
	const btRigidBody* getBtBody() const
	{
		return reinterpret_cast<const btRigidBody*>(&m_bodyMem[0]);
	}

	btRigidBody* getBtBody()
	{
		return reinterpret_cast<btRigidBody*>(&m_bodyMem[0]);
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
	alignas(alignof(btRigidBody)) Array<U8, sizeof(btRigidBody)> m_bodyMem;

	Transform m_trf = Transform::getIdentity();
	MotionState m_motionState;

	PhysicsCollisionShapePtr m_shape;

	F32 m_mass = 1.0f;

	PhysicsBody(PhysicsWorld* world, const PhysicsBodyInitInfo& init);

	~PhysicsBody();
};
/// @}

} // end namespace anki
