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

	m_ghostShape.init();
	m_ghostShape->setWorldTransform(btTransform::getIdentity());
	m_ghostShape->setCollisionShape(shape->getBtShape(true));

	m_ghostShape->setUserPointer(static_cast<PhysicsObject*>(this));

	setMaterialGroup(PhysicsMaterialBit::TRIGGER);
	setMaterialMask(PhysicsMaterialBit::ALL);

	auto lock = getWorld().lockBtWorld();
	getWorld().getBtWorld()->addCollisionObject(m_ghostShape.get());
}

PhysicsTrigger::~PhysicsTrigger()
{
	{
		auto lock = getWorld().lockBtWorld();
		getWorld().getBtWorld()->removeCollisionObject(m_ghostShape.get());
	}

	m_ghostShape.destroy();
}

void PhysicsTrigger::processContacts()
{
	if(m_contactCallback == nullptr)
	{
		return;
	}

	if(m_ghostShape->getOverlappingPairs().size() < 0)
	{
		return;
	}

	// Process contacts
	for(U i = 0; i < U(m_ghostShape->getOverlappingPairs().size()); ++i)
	{
		btCollisionObject* obj = m_ghostShape->getOverlappingPairs()[i];

		ANKI_ASSERT(obj);

		PhysicsObject* aobj = static_cast<PhysicsObject*>(obj->getUserPointer());
		ANKI_ASSERT(aobj);

		PhysicsFilteredObject* fobj = dcast<PhysicsFilteredObject*>(aobj);
		m_contactCallback->processContact(*this, *fobj);
	}
}

} // end namespace anki