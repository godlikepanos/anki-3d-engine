// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(JointsCreated, StatCategory::kScene, "Joints created", StatFlag::kNone)

JointComponent::JointComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
	, m_node(init.m_node)
{
	m_node->setIgnoreParentTransform(true);
}

JointComponent::~JointComponent()
{
}

Bool JointComponent::isValid() const
{
	SceneNode* node1 = m_node->getParent();
	SceneNode* node2 = m_node;

	BodyComponent* bodyc1 = (node1) ? node1->tryGetFirstComponentOfType<BodyComponent>() : nullptr;
	BodyComponent* bodyc2 = (node2) ? node2->tryGetFirstComponentOfType<BodyComponent>() : nullptr;

	PhysicsBody* body1 = (bodyc1) ? bodyc1->getPhysicsBody().tryGet() : nullptr;
	PhysicsBody* body2 = (bodyc2) ? bodyc2->getPhysicsBody().tryGet() : nullptr;

	return body1 && body2 && m_type < JointComponentyType::kCount;
}

void JointComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isValid())
	{
		m_joint.reset(nullptr);
		return;
	}

	SceneNode* node1 = m_node->getParent();
	SceneNode* node2 = m_node;

	BodyComponent* bodyc1 = node1->tryGetFirstComponentOfType<BodyComponent>();
	BodyComponent* bodyc2 = node2->tryGetFirstComponentOfType<BodyComponent>();

	PhysicsBody* body1 = bodyc1->getPhysicsBody().tryGet();
	PhysicsBody* body2 = bodyc2->getPhysicsBody().tryGet();

	const Bool node1Changed = m_parentNodeUuid != node1->getUuid();

	// Move the pivot now because it depends on the parent which is a deferred operation
	if(m_movePivot1To2)
	{
		// World transform of pivot2 (relative to node2's body)
		Transform trf2 = node2->getLocalTransform();
		trf2.setScale(Vec3(1.0f));
		const Transform worldPivot2 = trf2.combineTransformations(m_pivot2);

		// Express that world transform relative to node1's body to get the new pivot1
		Transform trf1 = node1->getLocalTransform();
		trf1.setScale(Vec3(1.0f));
		m_pivot1 = trf1.invert().combineTransformations(worldPivot2);

		m_pivotsDirty = true;
		m_movePivot1To2 = false;
	}
	else if(m_movePivot2To1)
	{
		// World transform of pivot1 (relative to node1's body)
		Transform trf1 = node1->getLocalTransform();
		trf1.setScale(Vec3(1.0f));
		const Transform worldPivot1 = trf1.combineTransformations(m_pivot1);

		// Express that world transform relative to node2's body to get the new pivot2
		Transform trf2 = node2->getLocalTransform();
		trf2.setScale(Vec3(1.0f));
		m_pivot2 = trf2.invert().combineTransformations(worldPivot1);

		m_pivotsDirty = true;
		m_movePivot2To1 = false;
	}

	if(node1Changed || m_pivotsDirty)
	{
		// Need to re-create the joint
		updated = true;
		m_pivotsDirty = false;
		m_parentNodeUuid = node1->getUuid();
		m_joint.reset(nullptr);
	}

	// Create new joint
	if(!m_joint)
	{
		updated = true;

		Transform trf1 = node1->getLocalTransform();
		trf1 = trf1.combineTransformations(m_pivot1);
		trf1.setScale(Vec3(1.0f));

		Transform trf2 = node2->getLocalTransform();
		trf2 = trf2.combineTransformations(m_pivot2);
		trf2.setScale(Vec3(1.0f));

		switch(m_type)
		{
		case JointComponentyType::kPoint:
			m_joint = PhysicsWorld::getSingleton().newPointJoint(body1, body2, trf1.getOrigin().xyz, trf2.getOrigin().xyz);
			break;
		case JointComponentyType::kHinge:
			m_joint = PhysicsWorld::getSingleton().newHingeJoint(body1, body2, trf1, trf2);
			break;
		default:
			ANKI_ASSERT(0);
		}

		g_svarJointsCreated.increment(1);
	}
}

Error JointComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_type, 1);

	Vec3 pivot1Origin = m_pivot1.getOrigin().xyz;
	Vec3 pivot2Origin = m_pivot2.getOrigin().xyz;
	Mat3 pivot1Rotation = m_pivot1.getRotation().getRotationPart();
	Mat3 pivot2Rotation = m_pivot2.getRotation().getRotationPart();

	ANKI_SERIALIZE(pivot1Origin, 1);
	ANKI_SERIALIZE(pivot2Origin, 1);
	ANKI_SERIALIZE(pivot1Rotation, 1);
	ANKI_SERIALIZE(pivot2Rotation, 1);

	if(serializer.isInReadMode())
	{
		m_pivot1.setOrigin(pivot1Origin);
		m_pivot1.setRotation(pivot1Rotation);
		m_pivot2.setOrigin(pivot2Origin);
		m_pivot2.setRotation(pivot2Rotation);
	}

	return Error::kNone;
}

} // end namespace anki
