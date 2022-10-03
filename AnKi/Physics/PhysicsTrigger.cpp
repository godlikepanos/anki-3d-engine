// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsTrigger.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Util/Rtti.h>

namespace anki {

PhysicsTrigger::PhysicsTrigger(PhysicsWorld* world, PhysicsCollisionShapePtr shape)
	: PhysicsFilteredObject(kClassType, world)
{
	m_shape = shape;

	m_ghostShape.init();
	m_ghostShape->setWorldTransform(btTransform::getIdentity());
	m_ghostShape->setCollisionShape(shape->getBtShape(true));

	// If you don't have that bodies will bounce on the trigger
	m_ghostShape->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	m_ghostShape->setUserPointer(static_cast<PhysicsObject*>(this));

	setMaterialGroup(PhysicsMaterialBit::kTrigger);
	setMaterialMask(PhysicsMaterialBit::kAll ^ PhysicsMaterialBit::kStaticGeometry);
}

PhysicsTrigger::~PhysicsTrigger()
{
	for(PhysicsTriggerFilteredPair* pair : m_pairs)
	{
		ANKI_ASSERT(pair);

		pair->m_trigger = nullptr;

		if(pair->shouldDelete())
		{
			deleteInstance(getMemoryPool(), pair);
		}
	}

	m_pairs.destroy(getMemoryPool());

	m_ghostShape.destroy();
}

void PhysicsTrigger::registerToWorld()
{
	getWorld().getBtWorld().addCollisionObject(m_ghostShape.get());
}

void PhysicsTrigger::unregisterFromWorld()
{
	getWorld().getBtWorld().removeCollisionObject(m_ghostShape.get());
}

void PhysicsTrigger::processContacts()
{
	++m_processContactsFrame;

	if(m_contactCallback == nullptr)
	{
		m_pairs.destroy(getMemoryPool());
		return;
	}

	// Gather the new pairs
	DynamicArrayRaii<PhysicsTriggerFilteredPair*> newPairs(&getWorld().getTempMemoryPool());
	newPairs.resizeStorage(m_ghostShape->getOverlappingPairs().size());
	for(U32 i = 0; i < U32(m_ghostShape->getOverlappingPairs().size()); ++i)
	{
		btCollisionObject* bobj = m_ghostShape->getOverlappingPairs()[i];
		ANKI_ASSERT(bobj);
		PhysicsObject* aobj = static_cast<PhysicsObject*>(bobj->getUserPointer());
		ANKI_ASSERT(aobj);
		PhysicsFilteredObject* obj = dcast<PhysicsFilteredObject*>(aobj);

		Bool isNew;
		PhysicsTriggerFilteredPair* pair = getWorld().getOrCreatePhysicsTriggerFilteredPair(this, obj, isNew);
		if(pair)
		{
			ANKI_ASSERT(pair->isAlive());
			newPairs.emplaceBack(pair);

			if(isNew)
			{
				m_contactCallback->onTriggerEnter(*this, *obj);
			}
			else
			{
				m_contactCallback->onTriggerInside(*this, *obj);
			}

			pair->m_frame = m_processContactsFrame;
		}
	}

	// Remove stale pairs
	for(U32 i = 0; i < m_pairs.getSize(); ++i)
	{
		PhysicsTriggerFilteredPair* pair = m_pairs[i];
		ANKI_ASSERT(pair->m_trigger == this);

		if(pair->m_filteredObject == nullptr)
		{
			// Filtered object died while inside the tigger, destroy the pair
			deleteInstance(getMemoryPool(), pair);
		}
		else if(pair->m_frame == m_processContactsFrame)
		{
			// Was updated this frame so don't touch it
		}
		else
		{
			// Was updated in some previous frame, notify and brake the link to the pair
			ANKI_ASSERT(pair->isAlive());
			ANKI_ASSERT(pair->m_frame < m_processContactsFrame);
			m_contactCallback->onTriggerExit(*this, *pair->m_filteredObject);
			pair->m_trigger = nullptr;
		}
	}

	// Store the new contacts
	m_pairs.resize(getMemoryPool(), newPairs.getSize());
	if(m_pairs.getSize())
	{
		memcpy(&m_pairs[0], &newPairs[0], m_pairs.getSizeInBytes());
	}
}

} // end namespace anki
