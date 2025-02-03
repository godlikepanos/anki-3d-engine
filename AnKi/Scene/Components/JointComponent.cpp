// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	F32 m_breakingImpulse = 0.0f;

	class P2P
	{
	public:
		Vec3 m_relPosBody1 = Vec3(kMaxF32);
		Vec3 m_relPosBody2 = Vec3(kMaxF32);
	};

	class Hinge
	{
	public:
		Vec3 m_relPosBody1 = Vec3(kMaxF32);
		Vec3 m_relPosBody2 = Vec3(kMaxF32);
		Vec3 m_axis = Vec3(0.0f);
	};

	union
	{
		Hinge m_hinge = {};
		P2P m_p2p;
	};

	JointType m_type = JointType::kCount;

	JointNode() = default;
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

void JointComponent::newPoint2PointJoint(const Vec3& relPosFactor, F32 breakingImpulse)
{
	JointNode* newNode = newInstance<JointNode>(SceneMemoryPool::getSingleton());

	newNode->m_type = JointType::kPoint;
	newNode->m_p2p.m_relPosBody1 = relPosFactor;
	newNode->m_breakingImpulse = breakingImpulse;

	m_jointList.pushBack(newNode);
}

void JointComponent::newPoint2PointJoint2(const Vec3& relPosFactorA, const Vec3& relPosFactorB, F32 breakingImpulse)
{
	JointNode* newNode = newInstance<JointNode>(SceneMemoryPool::getSingleton());

	newNode->m_type = JointType::kPoint;
	newNode->m_p2p.m_relPosBody1 = relPosFactorA;
	newNode->m_p2p.m_relPosBody2 = relPosFactorB;
	newNode->m_breakingImpulse = breakingImpulse;

	m_jointList.pushBack(newNode);
}

void JointComponent::newHingeJoint(const Vec3& relPosFactor, const Vec3& axis, F32 breakingImpulse)
{
	JointNode* newNode = newInstance<JointNode>(SceneMemoryPool::getSingleton());

	newNode->m_type = JointType::kHinge;
	newNode->m_hinge.m_relPosBody1 = relPosFactor;
	newNode->m_hinge.m_axis = axis;
	newNode->m_breakingImpulse = breakingImpulse;

	m_jointList.pushBack(newNode);
}

Error JointComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	SceneNode* parent = m_node->getParent();
	BodyComponent* bodyc1 = (m_bodyc && m_bodyc->getPhysicsBody()) ? m_bodyc : nullptr;
	BodyComponent* bodyc2 = nullptr;
	if(parent)
	{
		bodyc2 = parent->tryGetFirstComponentOfType<BodyComponent>();
		bodyc2 = (bodyc2 && bodyc2->getPhysicsBody()) ? bodyc2 : nullptr;
	}

	auto resetJoints = [this]() -> Bool {
		U32 resetCount = 0;
		for(JointNode& node : m_jointList)
		{
			resetCount += !!(node.m_joint);
			node.m_joint.reset(nullptr);
		}

		return resetCount > 0;
	};

	if(!bodyc1)
	{
		// No body no joints
		if(resetJoints())
		{
			updated = true;
		}
		return Error::kNone;
	}

	const Bool parentChanged = parent && m_parentUuid != parent->getUuid();
	if(parentChanged)
	{
		// New parent, reset joints
		updated = true;
		m_parentUuid = parent->getUuid();
		resetJoints();
	}

	// Remove broken joints
	while(true)
	{
		Bool erasedOne = false;
		for(JointNode& node : m_jointList)
		{
			if(node.m_joint && node.m_joint->isBroken())
			{
				m_jointList.erase(&node);
				deleteInstance(SceneMemoryPool::getSingleton(), &node);
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

	// Create new joints
	for(JointNode& node : m_jointList)
	{
		if(node.m_joint)
		{
			continue;
		}

		updated = true;

		switch(node.m_type)
		{
		case JointType::kPoint:
		{
			const Vec3 relPos1 = computeLocalPivotFromFactors(bodyc1->getPhysicsBody(), node.m_p2p.m_relPosBody1);

			if(node.m_p2p.m_relPosBody2 != Vec3(kMaxF32) && bodyc2)
			{
				const Vec3 relPos2 = computeLocalPivotFromFactors(bodyc2->getPhysicsBody(), node.m_p2p.m_relPosBody2);

				node.m_joint = PhysicsWorld::getSingleton().newInstance<PhysicsPoint2PointJoint>(bodyc1->getPhysicsBody(), relPos1,
																								 bodyc2->getPhysicsBody(), relPos2);
			}
			else
			{
				node.m_joint = PhysicsWorld::getSingleton().newInstance<PhysicsPoint2PointJoint>(bodyc1->getPhysicsBody(), relPos1);
			}
			break;
		}
		case JointType::kHinge:
		{
			const Vec3 relPos1 = computeLocalPivotFromFactors(bodyc1->getPhysicsBody(), node.m_hinge.m_relPosBody1);

			if(node.m_hinge.m_relPosBody2 != Vec3(kMaxF32) && bodyc2)
			{
				ANKI_ASSERT(!"TODO");
			}
			else
			{
				node.m_joint = PhysicsWorld::getSingleton().newInstance<PhysicsHingeJoint>(bodyc1->getPhysicsBody(), relPos1, node.m_hinge.m_axis);
			}
			break;
		}
		default:
			ANKI_ASSERT(0);
		}

		node.m_joint->setBreakingImpulseThreshold(node.m_breakingImpulse);
	}

	return Error::kNone;
}

void JointComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	if(other->getType() != SceneComponentType::kBody)
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
