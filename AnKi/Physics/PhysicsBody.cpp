// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

PhysicsBody::MyGroupFilter PhysicsBody::m_groupFilter;

Bool PhysicsBody::MyGroupFilter::CanCollide(const JPH::CollisionGroup& inGroup1, const JPH::CollisionGroup& inGroup2) const
{
	const U64 address1 = (U64(inGroup1.GetGroupID()) << 32_U64) | U64(inGroup1.GetSubGroupID());
	const U64 address2 = (U64(inGroup2.GetGroupID()) << 32_U64) | U64(inGroup2.GetSubGroupID());
	ANKI_ASSERT(address1 && address2);

	const PhysicsBody& body1 = *numberToPtr<const PhysicsBody*>(address1);
	const PhysicsBody& body2 = *numberToPtr<const PhysicsBody*>(address2);

	if(body1.m_collisionFilterCallback == body2.m_collisionFilterCallback && body1.m_collisionFilterCallback)
	{
		// Both have the same filter callback and it's not nullptr, use it
		return body1.m_collisionFilterCallback->collidesWith(body1, body2);
	}
	else
	{
		return true;
	}
}

PhysicsBody::PhysicsBody()
	: PhysicsObjectBase(PhysicsObjectType::kBody)
{
}

PhysicsBody::~PhysicsBody()
{
}

void PhysicsBody::init(const PhysicsBodyInitInfo& init)
{
	if(init.m_layer == PhysicsLayer::kStatic || init.m_layer == PhysicsLayer::kTrigger)
	{
		ANKI_ASSERT(init.m_mass == 0.0f);
	}
	else
	{
		ANKI_ASSERT(init.m_mass > 0.0f);
	}

	PhysicsWorld& world = PhysicsWorld::getSingleton();

	const Vec3 pos = init.m_transform.getOrigin().xyz();
	const Quat rot = Quat(init.m_transform.getRotation());

	// Create a scale shape
	const Bool hasScale = (init.m_transform.getScale().xyz() - 1.0).getLengthSquared() > kEpsilonf * 10.0;
	PhysicsCollisionShapePtr scaledShape;
	if(hasScale)
	{
		scaledShape = world.newScaleCollisionObject(init.m_transform.getScale().xyz(), init.m_shape);
	}

	// Create JPH body
	JPH::EMotionType motionType;
	if(init.m_isTrigger)
	{
		motionType = JPH::EMotionType::Kinematic;
	}
	else if(init.m_mass == 0.0f)
	{
		motionType = JPH::EMotionType::Static;
	}
	else
	{
		motionType = JPH::EMotionType::Dynamic;
	}

	JPH::BodyCreationSettings settings((scaledShape) ? &scaledShape->m_scaled : &init.m_shape->m_shapeBase, toJPH(pos), toJPH(rot), motionType,
									   JPH::ObjectLayer(init.m_layer));

	if(init.m_mass > 0.0f)
	{
		ANKI_ASSERT(!init.m_isTrigger && "Triggers can't have mass");
		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		settings.mMassPropertiesOverride.mMass = init.m_mass;
	}

	settings.mFriction = init.m_friction;
	settings.mUserData = ptrToNumber(static_cast<PhysicsObjectBase*>(this));

	settings.mIsSensor = init.m_isTrigger;

	// Call the thread-safe version because many threads may try to create bodies
	JPH::Body* jphBody = world.m_jphPhysicsSystem->GetBodyInterface().CreateBody(settings);
	world.m_jphPhysicsSystem->GetBodyInterface().AddBody(jphBody->GetID(), JPH::EActivation::Activate);

	// Misc
	m_jphBody = jphBody;
	m_primaryShape.reset(init.m_shape);
	m_scaledShape = scaledShape;
	m_worldTrf = init.m_transform;
	m_isTrigger = init.m_isTrigger;
	m_mass = init.m_mass;
	setUserData(init.m_userData);
}

void PhysicsBody::setPositionAndRotation(Vec3 position, const Mat3& rotation)
{
	const JPH::RVec3 pos = toJPH(position);
	const JPH::Quat rot = toJPH(Quat(rotation));

	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().SetPositionAndRotation(m_jphBody->GetID(), pos, rot,
																									 JPH::EActivation::Activate);
	m_worldTrf.setOrigin(position.xyz0());
	m_worldTrf.setRotation(rotation);
	++m_worldTrfVersion;
}

void PhysicsBody::applyForce(const Vec3& force, const Vec3& relPos)
{
	const Vec3 worldForcePos = m_worldTrf.transform(relPos);

	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().AddForce(m_jphBody->GetID(), toJPH(force), toJPH(worldForcePos));
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

void PhysicsBody::setLinearVelocity(Vec3 v)
{
	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().SetLinearVelocity(m_jphBody->GetID(), toJPH(v));
}

void PhysicsBody::setAngularVelocity(Vec3 v)
{
	PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().SetAngularVelocity(m_jphBody->GetID(), toJPH(v));
}

void PhysicsBody::clearForcesAndTorque()
{
	m_jphBody->ResetMotion();
}

void PhysicsBody::postPhysicsUpdate()
{
	if(m_activated)
	{
		const Transform newTrf =
			toAnKi(PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetWorldTransform(m_jphBody->GetID()));
		if(newTrf != m_worldTrf)
		{
			m_worldTrf = Transform(newTrf.getOrigin(), newTrf.getRotation(), m_worldTrf.getScale());
			++m_worldTrfVersion;
		}
	}
}

void PhysicsBody::setCollisionFilterCallback(PhysicsCollisionFilterCallback* callback)
{
	m_collisionFilterCallback = callback;
	JPH::CollisionGroup collisionGroup;

	if(m_collisionFilterCallback)
	{
		const U64 callbackAddress = ptrToNumber(callback);
		collisionGroup = JPH::CollisionGroup(&m_groupFilter, U32(callbackAddress >> 32_U64), U32(callbackAddress));
	}

	m_jphBody->SetCollisionGroup(collisionGroup);
}

} // namespace anki
