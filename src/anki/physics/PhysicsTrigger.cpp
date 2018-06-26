// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsTrigger.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

PhysicsTrigger::PhysicsTrigger(PhysicsWorld* world, PhysicsCollisionShapePtr shape)
	: PhysicsObject(PhysicsObjectType::TRIGGER, world)
{
	m_shape = shape;

	m_ghostShape = getAllocator().newInstance<btPairCachingGhostObject>();
	m_ghostShape->setWorldTransform(btTransform::getIdentity());
	m_ghostShape->setCollisionShape(shape->getBtShape(true));

	m_ghostShape->setUserPointer(static_cast<PhysicsObject*>(this));

	auto lock = getWorld().lockWorld();
	getWorld().getBtWorld()->addCollisionObject(m_ghostShape);
}

PhysicsTrigger::~PhysicsTrigger()
{
	{
		auto lock = getWorld().lockWorld();
		getWorld().getBtWorld()->removeCollisionObject(m_ghostShape);
	}

	getAllocator().deleteInstance(m_ghostShape);
}

void PhysicsTrigger::processContacts()
{
	// TODO
#if 0
	for(U i = 0; i < m_ghostShape->getOverlappingPairCache()->getNumOverlappingPairs(); ++i)
	{
		btBroadphasePair* collisionPair = &m_ghostShape->getOverlappingPairCache()->getOverlappingPairArray()[i];

		btCollisionObject* obj0 = static_cast<btCollisionObject*>(collisionPair->m_pProxy0->m_clientObject);
        btCollisionObject* obj1 = static_cast<btCollisionObject*>(collisionPair->m_pProxy1->m_clientObject);
	}
#endif
}

} // end namespace anki