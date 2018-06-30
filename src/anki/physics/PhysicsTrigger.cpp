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
	: PhysicsFilteredObject(CLASS_TYPE, world)
{
	m_shape = shape;

	m_ghostShape = getAllocator().newInstance<btPairCachingGhostObject>();
	m_ghostShape->setWorldTransform(btTransform::getIdentity());
	m_ghostShape->setCollisionShape(shape->getBtShape(true));

	m_ghostShape->setUserPointer(static_cast<PhysicsObject*>(this));

	setMaterialGroup(PhysicsMaterialBit::TRIGGER);
	setMaterialMask(PhysicsMaterialBit::ALL);

	auto lock = getWorld().lockBtWorld();
	getWorld().getBtWorld()->addCollisionObject(m_ghostShape);
}

PhysicsTrigger::~PhysicsTrigger()
{
	{
		auto lock = getWorld().lockBtWorld();
		getWorld().getBtWorld()->removeCollisionObject(m_ghostShape);
	}

	getAllocator().deleteInstance(m_ghostShape);
}

void PhysicsTrigger::processContacts()
{
	if(m_filter == nullptr)
	{
		return;
	}

	// Process contacts
	const U pairCount = m_ghostShape->getOverlappingPairCache()->getNumOverlappingPairs();
	for(U i = 0; i < pairCount; ++i)
	{
		btBroadphasePair& collisionPair = m_ghostShape->getOverlappingPairCache()->getOverlappingPairArray()[i];

		btCollisionObject* obj0 = static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject);
		btCollisionObject* obj1 = static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject);

		PhysicsObject* aobj0 = static_cast<PhysicsObject*>(obj0->getUserPointer());
		PhysicsObject* aobj1 = static_cast<PhysicsObject*>(obj1->getUserPointer());

		if(aobj0 == nullptr || aobj1 == nullptr)
		{
			continue;
		}

		PhysicsObject* otherObj;
		if(aobj0 == static_cast<PhysicsObject*>(this))
		{
			otherObj = aobj1;
		}
		else
		{
			ANKI_ASSERT(aobj1 == static_cast<PhysicsObject*>(this));
			otherObj = aobj0;
		}

		PhysicsObjectPtr ptr(otherObj);
		if(!m_filter->skipContact(ptr))
		{
			m_filter->processContact(ptr, ConstWeakArray<PhysicsTriggerContact>());
		}
	}
}

} // end namespace anki