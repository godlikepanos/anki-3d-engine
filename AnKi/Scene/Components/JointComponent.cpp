// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

JointComponent::JointComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	node->setIgnoreParentTransform(true);
}

JointComponent::~JointComponent()
{
}

Error JointComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	SceneNode& node = *info.m_node;

	SceneNode* parent = node.getParent();
	SceneNode* child = node.hasChildren() ? &node.getChild(0) : nullptr;

	BodyComponent* bodyc1 = (parent) ? parent->tryGetFirstComponentOfType<BodyComponent>() : nullptr;
	BodyComponent* bodyc2 = (child) ? child->tryGetFirstComponentOfType<BodyComponent>() : nullptr;

	PhysicsBody* body1 = (bodyc1) ? bodyc1->getPhysicsBody().tryGet() : nullptr;
	PhysicsBody* body2 = (bodyc2) ? bodyc2->getPhysicsBody().tryGet() : nullptr;

	if(!body1 || !body2 || m_type == JointType::kCount)
	{
		m_joint.reset(nullptr);
		return Error::kNone;
	}

	const Bool parentChanged = parent && m_parentNodeUuid != parent->getUuid();
	const Bool childChanged = child && m_childNodeUuid != child->getUuid();
	if(parentChanged || childChanged || node.movedThisFrame())
	{
		// Need to re-create the joint
		updated = true;
		m_parentNodeUuid = parent->getUuid();
		m_childNodeUuid = child->getUuid();
		m_joint.reset(nullptr);
	}

	// Create new joint
	if(!m_joint)
	{
		updated = true;

		switch(m_type)
		{
		case JointType::kPoint:
			m_joint = PhysicsWorld::getSingleton().newPointJoint(body1, body2, node.getWorldTransform().getOrigin().xyz());
			break;
		case JointType::kHinge:
			m_joint = PhysicsWorld::getSingleton().newHingeJoint(body1, body2, node.getWorldTransform());
			break;
		default:
			ANKI_ASSERT(0);
		}
	}

	return Error::kNone;
}

} // end namespace anki
