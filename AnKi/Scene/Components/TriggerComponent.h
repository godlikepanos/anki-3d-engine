// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics2/PhysicsBody.h>

namespace anki {

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

	WeakArray<SceneNode*> getSceneNodesEnter()
	{
		return WeakArray<SceneNode*>(m_bodiesEnter);
	}

	WeakArray<SceneNode*> getSceneNodesExit()
	{
		return WeakArray<SceneNode*>(m_bodiesExit);
	}

private:
	class MyPhysicsTriggerCallbacks;

	SceneNode* m_node;

	v2::PhysicsCollisionShapePtr m_shape;
	v2::PhysicsBodyPtr m_trigger;

	SceneDynamicArray<SceneNode*> m_bodiesEnter;
	SceneDynamicArray<SceneNode*> m_bodiesExit;

	Bool m_resetEnter = true;
	Bool m_resetExit = true;

	static MyPhysicsTriggerCallbacks m_callbacks;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
