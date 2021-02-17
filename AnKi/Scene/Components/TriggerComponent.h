// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsTrigger.h>

namespace anki
{

// Forward
class BodyComponent;

/// @addtogroup scene
/// @{

/// Trigger component.
class TriggerComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(TriggerComponent)

public:
	TriggerComponent(SceneNode* node);

	~TriggerComponent();

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

	void setSphereVolumeRadius(F32 radius);

	Transform getWorldTransform() const
	{
		return (m_trigger.isCreated()) ? m_trigger->getTransform() : Transform::getIdentity();
	}

	void setWorldTransform(const Transform& trf)
	{
		if(m_trigger.isCreated())
		{
			m_trigger->setTransform(trf);
		}
	}

	WeakArray<BodyComponent*> getBodyComponentsEnter()
	{
		return WeakArray<BodyComponent*>(m_bodiesEnter);
	}

	WeakArray<BodyComponent*> getBodyComponentsInside()
	{
		return WeakArray<BodyComponent*>(m_bodiesInside);
	}

	WeakArray<BodyComponent*> getBodyComponentsExit()
	{
		return WeakArray<BodyComponent*>(m_bodiesExit);
	}

private:
	class MyPhysicsTriggerProcessContactCallback;

	SceneNode* m_node;
	PhysicsCollisionShapePtr m_shape;
	PhysicsTriggerPtr m_trigger;
	DynamicArray<BodyComponent*> m_bodiesEnter;
	DynamicArray<BodyComponent*> m_bodiesInside;
	DynamicArray<BodyComponent*> m_bodiesExit;
	MyPhysicsTriggerProcessContactCallback* m_callbacks = nullptr;
};
/// @}

} // end namespace anki
