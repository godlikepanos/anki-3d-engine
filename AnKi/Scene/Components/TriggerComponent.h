// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsTrigger.h>

namespace anki {

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

	void setSphereVolumeRadius(F32 radius);

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
	SceneDynamicArray<BodyComponent*> m_bodiesEnter;
	SceneDynamicArray<BodyComponent*> m_bodiesInside;
	SceneDynamicArray<BodyComponent*> m_bodiesExit;
	MyPhysicsTriggerProcessContactCallback* m_callbacks = nullptr;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
