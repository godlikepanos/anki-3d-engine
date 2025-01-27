// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics2/PhysicsBody.h>
#include <AnKi/Physics2/PhysicsWorld.h>

namespace anki {
namespace v2 {

void PhysicsBody::setTransform(const Transform& trf)
{
	ANKI_ASSERT(trf.getScale() == m_worldTrf.getScale() && "Can't handle dynamic scaling for now");

	const JPH::RVec3 pos = toJPH(trf.getOrigin().xyz());
	const JPH::Quat rot = toJPH(Quat(trf.getRotation()));

	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().SetPositionAndRotation(m_jphBody->GetID(), pos, rot,
																									 JPH::EActivation::Activate);

	m_worldTrf = trf;
	++m_worldTrfVersion;
}

void PhysicsBody::applyForce(const Vec3& force, const Vec3& relPos)
{
	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().AddForce(m_jphBody->GetID(), toJPH(force), toJPH(relPos));
}

void PhysicsBody::applyForce(const Vec3& force)
{
	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().AddForce(m_jphBody->GetID(), toJPH(force));
}

void PhysicsBody::activate(Bool activate)
{
	if(activate)
	{
		PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().ActivateBody(m_jphBody->GetID());
	}
	else
	{
		PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().DeactivateBody(m_jphBody->GetID());
	}
}

void PhysicsBody::setGravityFactor(F32 factor)
{
	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().SetGravityFactor(m_jphBody->GetID(), factor);
}

void PhysicsBody::postPhysicsUpdate()
{
	if(m_activated)
	{
		const Transform newTrf =
			toAnKi(PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetWorldTransform(m_jphBody->GetID()));
		if(newTrf != m_worldTrf)
		{
			m_worldTrf = newTrf;
			++m_worldTrfVersion;
		}
	}
}

} // namespace v2
} // namespace anki
