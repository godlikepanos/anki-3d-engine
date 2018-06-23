// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/JointComponent.h>
#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

JointComponent::~JointComponent()
{
	m_jointList.destroy(getAllocator());
}

Vec3 JointComponent::computeLocalPivotFromFactors(const PhysicsBodyPtr& body, const Vec3& factors)
{
	btTransform identityTrf;
	identityTrf.setIdentity();
	btVector3 aabbmin, aabbmax, center;
	body->getBtBody()->getCollisionShape()->getAabb(identityTrf, aabbmin, aabbmax);

	center = (aabbmin + aabbmax) * 0.5f;

	Vec3 out;
	for(U i = 0; i < 3; ++i)
	{
		const F32 factor = factors[i];
		ANKI_ASSERT(factor >= -1.0f || factor <= 1.0f);
		const F32 dist = aabbmax[i] - center[i];
		out[i] = dist * factor + center[i];
	}

	return out;
}

void JointComponent::newPoint2PointJoint(const Vec3& relPosFactor, F32 brakingImpulse)
{
	BodyComponent* bodyc = m_node->tryGetComponent<BodyComponent>();

	if(bodyc)
	{
		Vec3 relPos = computeLocalPivotFromFactors(bodyc->getPhysicsBody(), relPosFactor);

		PhysicsJointPtr joint =
			getSceneGraph().getPhysicsWorld().newInstance<PhysicsPoint2PointJoint>(bodyc->getPhysicsBody(), relPos);
		joint->setBreakingImpulseThreshold(brakingImpulse);

		m_jointList.pushBack(getAllocator(), joint);
	}
	else
	{
		ANKI_SCENE_LOGW("Can't create new joint. The node is missing a body component");
	}
}

void JointComponent::newHingeJoint(const Vec3& relPosFactor, const Vec3& axis, F32 brakingImpulse)
{
	BodyComponent* bodyc = m_node->tryGetComponent<BodyComponent>();

	if(bodyc)
	{
		Vec3 relPos = computeLocalPivotFromFactors(bodyc->getPhysicsBody(), relPosFactor);

		PhysicsJointPtr joint =
			getSceneGraph().getPhysicsWorld().newInstance<PhysicsHingeJoint>(bodyc->getPhysicsBody(), relPos, axis);
		joint->setBreakingImpulseThreshold(brakingImpulse);

		m_jointList.pushBack(getAllocator(), joint);
	}
	else
	{
		ANKI_SCENE_LOGW("Can't create new joint. The node is missing a body component");
	}
}

Error JointComponent::update(Second prevTime, Second crntTime, Bool& updated)
{
	// TODO
	return Error::NONE;
}

} // end namespace anki