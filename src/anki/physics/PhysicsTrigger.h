// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/PhysicsObject.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// A trigger that uses a PhysicsShape and its purpose is to collect collision events.
class PhysicsTrigger : public PhysicsObject
{
	ANKI_PHYSICS_OBJECT

public:
	void setTransform(const Transform& trf)
	{
		m_ghostShape->setWorldTransform(toBt(trf));
	}

private:
	PhysicsCollisionShapePtr m_shape;
	btPairCachingGhostObject* m_ghostShape = nullptr;

	PhysicsTrigger(PhysicsWorld* world, PhysicsCollisionShapePtr shape);

	~PhysicsTrigger();

	void processContacts();
};
/// @}

} // end namespace anki