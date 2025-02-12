// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/TriggerComponent.h>
#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

/// The callbacks execute before the TriggerComponent::update
class TriggerComponent::MyPhysicsTriggerCallbacks final : public PhysicsTriggerCallbacks
{
public:
	void onTriggerEnter(const PhysicsBody& trigger, const PhysicsObjectBase& obj) override
	{
		TriggerComponent& triggerc = *reinterpret_cast<TriggerComponent*>(trigger.getUserData());
		ANKI_ASSERT(triggerc.getType() == TriggerComponent::kClassType);

		if(triggerc.m_resetEnter)
		{
			triggerc.m_bodiesEnter.destroy();
			triggerc.m_resetEnter = false;
		}

		if(obj.getType() == PhysicsObjectType::kBody)
		{
			const PhysicsBody& body = static_cast<const PhysicsBody&>(obj);
			BodyComponent& bodyc = *reinterpret_cast<BodyComponent*>(body.getUserData());
			ANKI_ASSERT(bodyc.getType() == BodyComponent::kClassType);
			triggerc.m_bodiesEnter.emplaceBack(&bodyc.getSceneNode());
		}
	}

	void onTriggerExit(const PhysicsBody& trigger, const PhysicsObjectBase& obj) override
	{
		TriggerComponent& triggerc = *reinterpret_cast<TriggerComponent*>(trigger.getUserData());
		ANKI_ASSERT(triggerc.getType() == TriggerComponent::kClassType);

		if(triggerc.m_resetExit)
		{
			triggerc.m_bodiesExit.destroy();
			triggerc.m_resetExit = false;
		}

		if(obj.getType() == PhysicsObjectType::kBody)
		{
			const PhysicsBody& body = static_cast<const PhysicsBody&>(obj);
			BodyComponent& bodyc = *reinterpret_cast<BodyComponent*>(body.getUserData());
			ANKI_ASSERT(bodyc.getType() == BodyComponent::kClassType);
			triggerc.m_bodiesExit.emplaceBack(&bodyc.getSceneNode());
		}
	}
};

TriggerComponent::MyPhysicsTriggerCallbacks TriggerComponent::m_callbacks;

TriggerComponent::TriggerComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
	, m_node(node)
{
}

TriggerComponent::~TriggerComponent()
{
}

void TriggerComponent::setSphereVolumeRadius(F32 radius)
{
	// Need to re-create it
	m_shape = PhysicsWorld::getSingleton().newSphereCollisionShape(radius);

	PhysicsBodyInitInfo init;
	init.m_isTrigger = true;
	init.m_shape = m_shape.get();
	init.m_transform = m_node->getWorldTransform();
	init.m_layer = PhysicsLayer::kTrigger;

	m_trigger = PhysicsWorld::getSingleton().newPhysicsBody(init);
	m_trigger->setUserData(this);
	m_trigger->setPhysicsTriggerCallbacks(&m_callbacks);
	m_trigger->setTransform(m_node->getWorldTransform());
}

Error TriggerComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(m_trigger) [[likely]]
	{
		if(!m_resetEnter || !m_resetExit)
		{
			updated = true;
		}

		if(m_resetEnter)
		{
			// None entered, cleanup
			m_bodiesEnter.destroy();
		}

		if(m_resetExit)
		{
			// None exited, cleanup
			m_bodiesExit.destroy();
		}

		// Prepare them for the next frame
		m_resetEnter = true;
		m_resetExit = true;

		if(info.m_node->movedThisFrame())
		{
			updated = true;
			m_trigger->setTransform(info.m_node->getWorldTransform());
		}
	}

	return Error::kNone;
}

} // end namespace anki
