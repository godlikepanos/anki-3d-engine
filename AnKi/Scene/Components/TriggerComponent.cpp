// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/TriggerComponent.h>
#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

// The callbacks execute before the TriggerComponent::update
class TriggerComponent::MyPhysicsTriggerCallbacks final : public PhysicsTriggerCallbacks
{
public:
	void onTriggerEnter(const PhysicsBody& trigger, const PhysicsObjectBase& obj) override
	{
		TriggerComponent& triggerc = *reinterpret_cast<TriggerComponent*>(trigger.getUserData());
		ANKI_ASSERT(triggerc.getType() == TriggerComponent::kClassType);

		if(triggerc.m_resetEnter)
		{
			triggerc.m_enteredNodes.destroy();
			triggerc.m_resetEnter = false;
		}

		if(obj.getType() == PhysicsObjectType::kBody)
		{
			const PhysicsBody& body = static_cast<const PhysicsBody&>(obj);
			BodyComponent& bodyc = *reinterpret_cast<BodyComponent*>(body.getUserData());
			ANKI_ASSERT(bodyc.getType() == BodyComponent::kClassType);
			triggerc.m_enteredNodes.emplaceBack(&bodyc.getSceneNode());
		}
		else if(obj.getType() == PhysicsObjectType::kPlayerController)
		{
			const PhysicsPlayerController& player = static_cast<const PhysicsPlayerController&>(obj);
			PlayerControllerComponent& comp = *reinterpret_cast<PlayerControllerComponent*>(player.getUserData());
			ANKI_ASSERT(comp.getType() == PlayerControllerComponent::kClassType);
			triggerc.m_enteredNodes.emplaceBack(&comp.getSceneNode());
		}
	}

	void onTriggerExit(const PhysicsBody& trigger, const PhysicsObjectBase& obj) override
	{
		TriggerComponent& triggerc = *reinterpret_cast<TriggerComponent*>(trigger.getUserData());
		ANKI_ASSERT(triggerc.getType() == TriggerComponent::kClassType);

		if(triggerc.m_resetExit)
		{
			triggerc.m_exitedNodes.destroy();
			triggerc.m_resetExit = false;
		}

		if(obj.getType() == PhysicsObjectType::kBody)
		{
			const PhysicsBody& body = static_cast<const PhysicsBody&>(obj);
			BodyComponent& bodyc = *reinterpret_cast<BodyComponent*>(body.getUserData());
			ANKI_ASSERT(bodyc.getType() == BodyComponent::kClassType);
			triggerc.m_exitedNodes.emplaceBack(&bodyc.getSceneNode());
		}
		else if(obj.getType() == PhysicsObjectType::kPlayerController)
		{
			const PhysicsPlayerController& player = static_cast<const PhysicsPlayerController&>(obj);
			PlayerControllerComponent& comp = *reinterpret_cast<PlayerControllerComponent*>(player.getUserData());
			ANKI_ASSERT(comp.getType() == PlayerControllerComponent::kClassType);
			triggerc.m_exitedNodes.emplaceBack(&comp.getSceneNode());
		}
	}
};

TriggerComponent::MyPhysicsTriggerCallbacks TriggerComponent::m_callbacks;

TriggerComponent::TriggerComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
{
}

TriggerComponent::~TriggerComponent()
{
}

TriggerComponent& TriggerComponent::setTriggerComponentType(TriggerComponentShapeType type)
{
	if(type != m_type)
	{
		// Force to recreate
		m_trigger.reset(nullptr);
		m_shape.reset(nullptr);
		m_type = type;
	}

	return *this;
}

void TriggerComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	ANKI_ASSERT(info.m_node);

	if(m_type == TriggerComponentShapeType::kCount)
	{
		return;
	}

	if(m_trigger && info.m_node->movedThisFrame() && info.m_node->getWorldTransform().getScale() != m_trigger->getTransform().getScale())
	{
		// Body will need to be re-created on scale change
		m_trigger.reset(nullptr);
	}

	if(!m_trigger)
	{
		// Create it

		updated = true;

		if(m_type == TriggerComponentShapeType::kSphere)
		{
			m_shape = PhysicsWorld::getSingleton().newSphereCollisionShape(1.0f);
		}
		else
		{
			m_shape = PhysicsWorld::getSingleton().newBoxCollisionShape(Vec3(1.0f));
		}

		PhysicsBodyInitInfo init;
		init.m_isTrigger = true;
		init.m_shape = m_shape.get();
		init.m_transform = info.m_node->getWorldTransform();
		init.m_layer = PhysicsLayer::kTrigger;

		m_trigger = PhysicsWorld::getSingleton().newPhysicsBody(init);
		m_trigger->setUserData(this);
		m_trigger->setPhysicsTriggerCallbacks(&m_callbacks);
	}
	else if(info.m_node->movedThisFrame())
	{
		updated = true;
		m_trigger->setPositionAndRotation(info.m_node->getWorldTransform().getOrigin().xyz,
										  info.m_node->getWorldTransform().getRotation().getRotationPart());
	}

	if(!m_resetEnter || !m_resetExit)
	{
		updated = true;
	}

	if(m_resetEnter)
	{
		// None entered, cleanup
		m_enteredNodes.destroy();
	}

	if(m_resetExit)
	{
		// None exited, cleanup
		m_exitedNodes.destroy();
	}

	// Call the callbacks
	for(SceneNode* node : m_enteredNodes)
	{
		info.m_node->onTriggerEnter(node);
	}

	for(SceneNode* node : m_exitedNodes)
	{
		info.m_node->onTriggerExit(node);
	}

	// Prepare them for the next frame
	m_resetEnter = true;
	m_resetExit = true;
}

Error TriggerComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_type, 1);
	return Error::kNone;
}

} // end namespace anki
