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

/// @memberof PhysicsTrigger
class PhysicsTriggerContact
{
public:
	Vec3 m_contactPoint; ///< In world space.
	Vec3 m_contactNormal; ///< In world space.
};

/// An interface to process contacts.
/// @memberof PhysicsTrigger
class PhysicsTriggerProcessContactInterface
{
public:
	virtual void processContact(
		PhysicsTriggerPtr& trigger, PhysicsFilteredObjectPtr& obj, ConstWeakArray<PhysicsTriggerContact> contacts) = 0;
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

	void setContactProcess(PhysicsTriggerProcessContactInterface* functor)
	{
		m_contactFunctor = functor;
	}

private:
	using Contact = PhysicsTriggerContact;

	PhysicsCollisionShapePtr m_shape;
	btPairCachingGhostObject* m_ghostShape = nullptr;

	PhysicsTriggerProcessContactInterface* m_contactFunctor = nullptr;

	PhysicsTrigger(PhysicsWorld* world, PhysicsCollisionShapePtr shape);

	~PhysicsTrigger();

	void processContacts();
};
/// @}

} // end namespace anki