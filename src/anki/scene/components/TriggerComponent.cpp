// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/TriggerComponent.h>
#include <anki/scene/SceneNode.h>

namespace anki
{

class TriggerComponent::MyPhysicsTriggerProcessContactCallback : public PhysicsTriggerProcessContactCallback
{
public:
	TriggerComponent* m_comp = nullptr;

	void processContact(PhysicsTrigger& trigger, PhysicsFilteredObject& obj) final
	{
		void* ptr = obj.getUserData();
		ANKI_ASSERT(ptr);

		SceneNode* node = static_cast<SceneNode*>(ptr);
		m_comp->processContact(*node);
	}
};

TriggerComponent::TriggerComponent(SceneNode* node, PhysicsTriggerPtr trigger)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
	, m_trigger(trigger)
{
	ANKI_ASSERT(node);
	m_contactCb = m_node->getAllocator().newInstance<MyPhysicsTriggerProcessContactCallback>();
	m_contactCb->m_comp = this;
	m_trigger->setContactProcessCallback(m_contactCb);
}

TriggerComponent::~TriggerComponent()
{
	m_node->getAllocator().deleteInstance(m_contactCb);
	m_contactNodes.destroy(m_node->getAllocator());
}

void TriggerComponent::processContact(SceneNode& node)
{
	ANKI_ASSERT(&node != m_node);

	// Clear the list if it's a new frame
	if(m_contactNodesArrayTimestamp != node.getGlobalTimestamp())
	{
		m_contactNodesArrayTimestamp = node.getGlobalTimestamp();
		m_contactNodes.destroy(node.getAllocator());
	}

	m_contactNodes.emplaceBack(node.getAllocator(), &node);
}

} // end namespace anki