// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/TriggerComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

/// The callbacks execute before the TriggerComponent::update
class TriggerComponent::MyPhysicsTriggerProcessContactCallback final : public PhysicsTriggerProcessContactCallback
{
public:
	TriggerComponent* m_comp = nullptr;
	Bool m_updated = false;
	Bool m_enterUpdated = false;
	Bool m_insideUpdated = false;
	Bool m_exitUpdated = false;

	void onTriggerEnter([[maybe_unused]] PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
		// Clean previous results
		if(!m_enterUpdated)
		{
			m_enterUpdated = true;
			m_comp->m_bodiesEnter.destroy();
		}

		m_updated = true;

		m_comp->m_bodiesEnter.emplaceBack(static_cast<BodyComponent*>(obj.getUserData()));
	}

	void onTriggerInside([[maybe_unused]] PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
		// Clean previous results
		if(!m_insideUpdated)
		{
			m_insideUpdated = true;
			m_comp->m_bodiesInside.destroy();
		}

		m_updated = true;

		m_comp->m_bodiesInside.emplaceBack(static_cast<BodyComponent*>(obj.getUserData()));
	}

	void onTriggerExit([[maybe_unused]] PhysicsTrigger& trigger, PhysicsFilteredObject& obj)
	{
		// Clean previous results
		if(!m_exitUpdated)
		{
			m_exitUpdated = true;
			m_comp->m_bodiesExit.destroy();
		}

		m_updated = true;

		m_comp->m_bodiesExit.emplaceBack(static_cast<BodyComponent*>(obj.getUserData()));
	}
};

TriggerComponent::TriggerComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
	ANKI_ASSERT(node);
	m_callbacks = newInstance<MyPhysicsTriggerProcessContactCallback>(SceneMemoryPool::getSingleton());
	m_callbacks->m_comp = this;
}

TriggerComponent::~TriggerComponent()
{
	deleteInstance(SceneMemoryPool::getSingleton(), m_callbacks);
}

void TriggerComponent::setSphereVolumeRadius(F32 radius)
{
	// Need to re-create it
	m_shape = PhysicsWorld::getSingleton().newInstance<PhysicsSphere>(radius);
	m_trigger = PhysicsWorld::getSingleton().newInstance<PhysicsTrigger>(m_shape);
	m_trigger->setUserData(this);
	m_trigger->setContactProcessCallback(m_callbacks);
	m_trigger->setTransform(m_node->getWorldTransform());
}

Error TriggerComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = false;

	if(m_trigger.isCreated()) [[likely]]
	{
		updated = m_callbacks->m_updated;
		m_callbacks->m_updated = false;
		m_callbacks->m_enterUpdated = false;
		m_callbacks->m_insideUpdated = false;
		m_callbacks->m_exitUpdated = false;

		if(info.m_node->movedThisFrame() && m_trigger.isCreated())
		{
			updated = true;
			m_trigger->setTransform(info.m_node->getWorldTransform());
		}
	}

	return Error::kNone;
}

} // end namespace anki
