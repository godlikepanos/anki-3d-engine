// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/TriggerComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(TriggerComponent)

/// The callbacks execute before the TriggerComponent::update
class TriggerComponent::MyPhysicsTriggerProcessContactCallback final : public PhysicsTriggerProcessContactCallback
{
public:
	TriggerComponent* m_comp = nullptr;
	Bool m_updated = false;
	Bool m_enterUpdated = false;
	Bool m_insideUpdated = false;
	Bool m_exitUpdated = false;

	void onTriggerEnter(PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
		// Clean previous results
		if(!m_enterUpdated)
		{
			m_enterUpdated = true;
			m_comp->m_bodiesEnter.destroy(m_comp->m_node->getAllocator());
		}

		m_updated = true;

		m_comp->m_bodiesEnter.emplaceBack(m_comp->m_node->getAllocator(),
										  static_cast<BodyComponent*>(obj.getUserData()));
	}

	void onTriggerInside(PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
		// Clean previous results
		if(!m_insideUpdated)
		{
			m_insideUpdated = true;
			m_comp->m_bodiesInside.destroy(m_comp->m_node->getAllocator());
		}

		m_updated = true;

		m_comp->m_bodiesInside.emplaceBack(m_comp->m_node->getAllocator(),
										   static_cast<BodyComponent*>(obj.getUserData()));
	}

	void onTriggerExit(PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
		// Clean previous results
		if(!m_exitUpdated)
		{
			m_exitUpdated = true;
			m_comp->m_bodiesExit.destroy(m_comp->m_node->getAllocator());
		}

		m_updated = true;

		m_comp->m_bodiesExit.emplaceBack(m_comp->m_node->getAllocator(),
										 static_cast<BodyComponent*>(obj.getUserData()));
	}
};

TriggerComponent::TriggerComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
	ANKI_ASSERT(node);
	m_callbacks = m_node->getAllocator().newInstance<MyPhysicsTriggerProcessContactCallback>();
	m_callbacks->m_comp = this;
}

TriggerComponent::~TriggerComponent()
{
	m_node->getAllocator().deleteInstance(m_callbacks);
	m_bodiesEnter.destroy(m_node->getAllocator());
	m_bodiesInside.destroy(m_node->getAllocator());
	m_bodiesExit.destroy(m_node->getAllocator());
}

void TriggerComponent::setSphereVolumeRadius(F32 radius)
{
	m_shape = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsSphere>(radius);
	m_trigger = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsTrigger>(m_shape);
	m_trigger->setUserData(this);
	m_trigger->setContactProcessCallback(m_callbacks);
}

Error TriggerComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	updated = m_callbacks->m_updated;
	m_callbacks->m_updated = false;
	m_callbacks->m_enterUpdated = false;
	m_callbacks->m_insideUpdated = false;
	m_callbacks->m_exitUpdated = false;
	return Error::NONE;
}

} // end namespace anki
