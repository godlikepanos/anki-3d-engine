// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/TriggerComponent.h>

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
	: SceneComponent(CLASS_TYPE, node)
	, m_trigger(trigger)
{
	m_contactCb = getAllocator().newInstance<MyPhysicsTriggerProcessContactCallback>();
	m_contactCb->m_comp = this;
	m_trigger->setContactProcessCallback(m_contactCb);
}

TriggerComponent::~TriggerComponent()
{
	getAllocator().deleteInstance(m_contactCb);
	m_contactNodes.destroy(getAllocator());
}

Error TriggerComponent::update(Second, Second, Bool& updated)
{
	updated = false;
	return Error::NONE;
}

void TriggerComponent::processContact(SceneNode& node)
{
	// Clear the list if it's a new frame
	if(m_contactNodesArrayTimestamp != getGlobalTimestamp())
	{
		m_contactNodesArrayTimestamp = getGlobalTimestamp();
		m_contactNodes.destroy(getAllocator());
	}

	m_contactNodes.emplaceBack(getAllocator(), &node);
}

} // end namespace anki