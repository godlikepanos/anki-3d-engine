// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsCollisionShape.h>

namespace anki
{

class PhysicsBody::MotionState : public btMotionState
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
		m_body->m_updated = true;
	}
};

PhysicsBody::PhysicsBody(PhysicsWorld* world, const PhysicsBodyInitInfo& init)
	: PhysicsObject(PhysicsObjectType::BODY, world)
{
	if(init.m_static)
	{
		ANKI_ASSERT(init.m_mass > 0.0f);
	}

	// Create motion state
	m_motionState = getAllocator().newInstance<MotionState>();
	m_motionState->m_body = this;

	// Compute inertia
	btCollisionShape* shape = init.m_shape->getBtShape(!init.m_static);
	btVector3 localInertia(0, 0, 0);
	if(!init.m_static)
	{
		shape->calculateLocalInertia(init.m_mass, localInertia);
	}

	// Create body
	btRigidBody::btRigidBodyConstructionInfo cInfo(init.m_mass, m_motionState, shape, localInertia);
	m_body = getAllocator().newInstance<btRigidBody>(cInfo);

	getWorld().getBtWorld()->addRigidBody(m_body);
}

PhysicsBody::~PhysicsBody()
{
	if(m_body)
	{
		getWorld().getBtWorld()->removeRigidBody(m_body);
		getAllocator().deleteInstance(m_body);
	}

	if(m_motionState)
	{
		getAllocator().deleteInstance(m_motionState);
	}
}

} // end namespace anki
