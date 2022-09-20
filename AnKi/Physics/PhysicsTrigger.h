// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Physics/PhysicsObject.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/ClassWrapper.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup physics
/// @{

/// An interface to process contacts.
/// @memberof PhysicsTrigger
class PhysicsTriggerProcessContactCallback
{
public:
	/// Will be called whenever a contact first touches a trigger.
	virtual void onTriggerEnter([[maybe_unused]] PhysicsTrigger& trigger, [[maybe_unused]] PhysicsFilteredObject& obj)
	{
	}

	/// Will be called whenever a contact touches a trigger.
	virtual void onTriggerInside([[maybe_unused]] PhysicsTrigger& trigger, [[maybe_unused]] PhysicsFilteredObject& obj)
	{
	}

	/// Will be called whenever a contact stops touching a trigger.
	virtual void onTriggerExit([[maybe_unused]] PhysicsTrigger& trigger, [[maybe_unused]] PhysicsFilteredObject& obj)
	{
	}
};

/// A trigger that uses a PhysicsShape and its purpose is to collect collision events.
class PhysicsTrigger : public PhysicsFilteredObject
{
	ANKI_PHYSICS_OBJECT(PhysicsObjectType::kTrigger)

public:
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

	void registerToWorld() override;

	void unregisterFromWorld() override;

	void processContacts();
};
/// @}

} // end namespace anki
