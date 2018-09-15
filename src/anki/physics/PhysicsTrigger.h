// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// An interface to process contacts.
/// @memberof PhysicsTrigger
class PhysicsTriggerProcessContactCallback
{
public:
	virtual ~PhysicsTriggerProcessContactCallback()
	{
	}

	virtual void processContact(PhysicsTrigger& trigger, PhysicsFilteredObject& obj) = 0;
};

/// A trigger that uses a PhysicsShape and its purpose is to collect collision events.
class PhysicsTrigger : public PhysicsFilteredObject
{
	ANKI_PHYSICS_OBJECT

public:
	static const PhysicsObjectType CLASS_TYPE = PhysicsObjectType::TRIGGER;

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
	BtClassWrapper<btGhostObject> m_ghostShape;

	PhysicsTriggerProcessContactCallback* m_contactCallback = nullptr;

	PhysicsTrigger(PhysicsWorld* world, PhysicsCollisionShapePtr shape);

	~PhysicsTrigger();

	void processContacts();
};
/// @}

} // end namespace anki