// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>
#include <anki/util/WeakArray.h>
#include <anki/util/ClassWrapper.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// An interface to process contacts.
/// @memberof PhysicsTrigger
class PhysicsTriggerProcessContactCallback
{
public:
	/// Will be called whenever a contact first touches a trigger.
	virtual void onTriggerEnter(PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
	}

	/// Will be called whenever a contact touches a trigger.
	virtual void onTriggerInside(PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
	}

	/// Will be called whenever a contact stops touching a trigger.
	virtual void onTriggerExit(PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
	}
};

/// A trigger that uses a PhysicsShape and its purpose is to collect collision events.
class PhysicsTrigger : public PhysicsFilteredObject
{
	ANKI_PHYSICS_OBJECT

public:
	static const PhysicsObjectType CLASS_TYPE = PhysicsObjectType::TRIGGER;

	Transform getTransform() const
	{
		return toAnki(m_ghostShape->getWorldTransform());
	}

	void setTransform(const Transform& trf)
	{
		m_ghostShape->setWorldTransform(toBt(trf));
	}

	void setContactProcessCallback(PhysicsTriggerProcessContactCallback* cb)
	{
		m_contactCallback = cb;
	}

private:
	PhysicsCollisionShapePtr m_shape;
	ClassWrapper<btGhostObject> m_ghostShape;

	DynamicArray<PhysicsTriggerFilteredPair*> m_pairs;

	PhysicsTriggerProcessContactCallback* m_contactCallback = nullptr;

	U64 m_processContactsFrame = 0;

	PhysicsTrigger(PhysicsWorld* world, PhysicsCollisionShapePtr shape);

	~PhysicsTrigger();

	void processContacts();
};
/// @}

} // end namespace anki
