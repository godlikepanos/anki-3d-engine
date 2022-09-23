// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>

namespace anki {

PhysicsBody::PhysicsBody(PhysicsWorld* world, const PhysicsBodyInitInfo& init)
	: PhysicsFilteredObject(kClassType, world)
{
	ANKI_ASSERT(init.m_mass >= 0.0f);

	const Bool dynamic = init.m_mass > 0.0f;
	m_shape = init.m_shape;
	m_mass = init.m_mass;

	// Create motion state
	m_motionState.m_body = this;

	// Compute inertia
	btCollisionShape* shape = m_shape->getBtShape(dynamic);
	btVector3 localInertia(0, 0, 0);
	if(dynamic)
	{
		shape->calculateLocalInertia(init.m_mass, localInertia);
	}

	// Create body
	btRigidBody::btRigidBodyConstructionInfo cInfo(init.m_mass, &m_motionState, shape, localInertia);
	cInfo.m_friction = init.m_friction;
	m_body.init(cInfo);

	// User pointer
	m_body->setUserPointer(static_cast<PhysicsObject*>(this));

	// Other
	setMaterialGroup((dynamic) ? PhysicsMaterialBit::kDynamicGeometry : PhysicsMaterialBit::kStaticGeometry);

	PhysicsMaterialBit collidesWith = PhysicsMaterialBit::kAll;
	if(!dynamic)
	{
		collidesWith &= ~PhysicsMaterialBit::kStaticGeometry;
	}
	setMaterialMask(collidesWith);
	setTransform(init.m_transform);
}

PhysicsBody::~PhysicsBody()
{
	m_body.destroy();
}

void PhysicsBody::setMass(F32 mass)
{
	ANKI_ASSERT(m_mass > 0.0f && "Only relevant for dynamic bodies");
	ANKI_ASSERT(mass > 0.0f);
	btVector3 inertia;
	m_shape->getBtShape(true)->calculateLocalInertia(mass, inertia);
	m_body->setMassProps(mass, inertia);
	m_mass = mass;
}

void PhysicsBody::registerToWorld()
{
	getWorld().getBtWorld().addRigidBody(m_body.get());
}

void PhysicsBody::unregisterFromWorld()
{
	getWorld().getBtWorld().removeRigidBody(m_body.get());
}

} // end namespace anki
