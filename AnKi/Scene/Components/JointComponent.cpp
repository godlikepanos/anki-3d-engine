// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

class JointComponent::JointNode : public IntrusiveListEnabled<JointNode>
{
public:
	PhysicsJointPtr m_joint;
	SceneNode* m_parentNode = nullptr; ///< Keep track the node that is connected with this node through a joint.
};

JointComponent::~JointComponent()
{
	while(!m_jointList.isEmpty())
	{
		JointNode* jnode = &m_jointList.getFront();
		m_jointList.popFront();

		deleteInstance(SceneMemoryPool::getSingleton(), jnode);
	}
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

template<typename TJoint, typename... TArgs>
void JointComponent::newJoint(const Vec3& relPosFactor, F32 breakingImpulse, TArgs&&... args)
{
	BodyComponent* bodyc = m_bodyc;

	if(bodyc)
	{
		Vec3 relPos = computeLocalPivotFromFactors(bodyc->getPhysicsBody(), relPosFactor);

		PhysicsJointPtr joint = PhysicsWorld::getSingleton().newInstance<TJoint>(bodyc->getPhysicsBody(), relPos,
																				 std::forward<TArgs>(args)...);
		joint->setBreakingImpulseThreshold(breakingImpulse);

		JointNode* newNode = newInstance<JointNode>(SceneMemoryPool::getSingleton());
		newNode->m_joint = std::move(joint);
		m_jointList.pushBack(newNode);
	}
	else
	{
		ANKI_SCENE_LOGW("Can't create new joint. The node is missing a BodyComponent");
	}
}

void JointComponent::newPoint2PointJoint(const Vec3& relPosFactor, F32 breakingImpulse)
{
	newJoint<PhysicsPoint2PointJoint>(relPosFactor, breakingImpulse);
}

void JointComponent::newPoint2PointJoint2(const Vec3& relPosFactorA, const Vec3& relPosFactorB, F32 breakingImpulse)
{
	BodyComponent* bodycA = m_bodyc;
	BodyComponent* bodycB = nullptr;
	if(m_node->getParent())
	{
		bodycB = m_node->getParent()->tryGetFirstComponentOfType<BodyComponent>();
	}

	if(bodycA && bodycB)
	{
		Vec3 relPosA = computeLocalPivotFromFactors(bodycA->getPhysicsBody(), relPosFactorA);
		Vec3 relPosB = computeLocalPivotFromFactors(bodycB->getPhysicsBody(), relPosFactorB);

		PhysicsJointPtr joint = PhysicsWorld::getSingleton().newInstance<PhysicsPoint2PointJoint>(
			bodycA->getPhysicsBody(), relPosA, bodycB->getPhysicsBody(), relPosB);
		joint->setBreakingImpulseThreshold(breakingImpulse);

		JointNode* newNode = newInstance<JointNode>(SceneMemoryPool::getSingleton());
		newNode->m_joint = std::move(joint);
		newNode->m_parentNode = m_node->getParent();
		m_jointList.pushBack(newNode);
	}
	else
	{
		ANKI_SCENE_LOGW("Can't create new joint. The node or its parent are missing a BodyComponent");
	}
}

void JointComponent::newHingeJoint(const Vec3& relPosFactor, const Vec3& axis, F32 breakingImpulse)
{
	newJoint<PhysicsHingeJoint>(relPosFactor, breakingImpulse, axis);
}

Error JointComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	// Iterate the joints and check if the connected scene node is not the parent of this node anymore.
	while(true)
	{
		Bool erasedOne = false;
		for(JointNode& joint : m_jointList)
		{
			if(joint.m_parentNode != info.m_node->getParent() || joint.m_joint->isBroken())
			{
				m_jointList.erase(&joint);
				deleteInstance(SceneMemoryPool::getSingleton(), &joint);
				erasedOne = true;
				updated = true;
				break;
			}
		}

		if(!erasedOne)
		{
			break;
		}
	}

	return Error::kNone;
}

void JointComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	if(other->getClassId() != BodyComponent::getStaticClassId())
	{
		return;
	}

	BodyComponent* bodyc = static_cast<BodyComponent*>(other);

	Bool jointListInvalid = false;
	if(added && m_bodyc == nullptr)
	{
		m_bodyc = bodyc;
		jointListInvalid = true;
	}
	else if(bodyc == m_bodyc)
	{
		m_bodyc = nullptr;
		jointListInvalid = true;
	}

	if(jointListInvalid)
	{
		onDestroy(*m_node);
	}
}

} // end namespace anki
