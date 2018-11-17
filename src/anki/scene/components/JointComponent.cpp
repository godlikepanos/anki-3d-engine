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

class JointComponent::JointNode : public IntrusiveListEnabled<JointNode>
{
public:
	PhysicsJointPtr m_joint;
	SceneNode* m_parentNode = nullptr; ///< Keep track the node that is connected with this node through a joint.
};

JointComponent::~JointComponent()
{
	removeAllJoints();
}

void JointComponent::removeAllJoints()
{
	while(!m_jointList.isEmpty())
	{
		JointNode* node = &m_jointList.getFront();
		m_jointList.popFront();

		m_node->getAllocator().deleteInstance(node);
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
	BodyComponent* bodyc = m_node->tryGetComponent<BodyComponent>();

	if(bodyc)
	{
		Vec3 relPos = computeLocalPivotFromFactors(bodyc->getPhysicsBody(), relPosFactor);

		PhysicsJointPtr joint = m_node->getSceneGraph().getPhysicsWorld().newInstance<TJoint>(
			bodyc->getPhysicsBody(), relPos, std::forward<TArgs>(args)...);
		joint->setBreakingImpulseThreshold(breakingImpulse);

		JointNode* newNode = m_node->getAllocator().newInstance<JointNode>();
		newNode->m_joint = joint;
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
	BodyComponent* bodycA = m_node->tryGetComponent<BodyComponent>();
	BodyComponent* bodycB = nullptr;
	if(m_node->getParent())
	{
		bodycB = m_node->getParent()->tryGetComponent<BodyComponent>();
	}

	if(bodycA && bodycB)
	{
		Vec3 relPosA = computeLocalPivotFromFactors(bodycA->getPhysicsBody(), relPosFactorA);
		Vec3 relPosB = computeLocalPivotFromFactors(bodycB->getPhysicsBody(), relPosFactorB);

		PhysicsJointPtr joint = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsPoint2PointJoint>(
			bodycA->getPhysicsBody(), relPosA, bodycB->getPhysicsBody(), relPosB);
		joint->setBreakingImpulseThreshold(breakingImpulse);

		JointNode* newNode = m_node->getAllocator().newInstance<JointNode>();
		newNode->m_joint = joint;
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

Error JointComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	ANKI_ASSERT(&node == m_node);

	// Iterate the joints and check if the connected scene node is not the parent of this node anymore.
	while(true)
	{
		Bool erasedOne = false;
		for(auto otherNode : m_jointList)
		{
			if(otherNode.m_parentNode != node.getParent() || otherNode.m_joint->isBroken())
			{
				m_jointList.erase(&otherNode);
				node.getAllocator().deleteInstance(&otherNode);
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

	return Error::NONE;
}

} // end namespace anki