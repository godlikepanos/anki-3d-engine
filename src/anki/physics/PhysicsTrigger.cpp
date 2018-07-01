// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsTrigger.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/util/Rtti.h>

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
	if(m_contactFunctor == nullptr)
	{
		return;
	}

	const U pairCount = m_ghostShape->getOverlappingPairCache()->getNumOverlappingPairs();
	if(pairCount < 0)
	{
		return;
	}

	PhysicsTriggerPtr thisPtr(this);

	// Process contacts
	for(U i = 0; i < pairCount; ++i)
	{
		btBroadphasePair& collisionPair = m_ghostShape->getOverlappingPairCache()->getOverlappingPairArray()[i];

		btCollisionObject* obj0 = static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject);
		btCollisionObject* obj1 = static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject);
		ANKI_ASSERT(obj0 && obj1);

		PhysicsObject* aobj0 = static_cast<PhysicsObject*>(obj0->getUserPointer());
		PhysicsObject* aobj1 = static_cast<PhysicsObject*>(obj1->getUserPointer());
		ANKI_ASSERT(aobj0 && aobj1);

		PhysicsFilteredObject* fobj0 = dcast<PhysicsFilteredObject*>(aobj0);
		PhysicsFilteredObject* fobj1 = dcast<PhysicsFilteredObject*>(aobj1);

		PhysicsFilteredObject* otherObj;
		if(fobj0 == static_cast<PhysicsFilteredObject*>(this))
		{
			otherObj = fobj1;
		}
		else
		{
			ANKI_ASSERT(fobj1 == static_cast<PhysicsFilteredObject*>(this));
			otherObj = fobj0;
		}

		PhysicsFilteredObjectPtr ptr(otherObj);
		m_contactFunctor->processContact(thisPtr, ptr, ConstWeakArray<PhysicsTriggerContact>());
	}
}

} // end namespace anki