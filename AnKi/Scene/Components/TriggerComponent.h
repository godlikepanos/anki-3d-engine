// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsBody.h>

namespace anki {

enum class TriggerComponentShapeType
{
	kSphere,
	kBox,
	kCount
};

// Trigger component
class TriggerComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(TriggerComponent)

public:
	TriggerComponent(SceneNode* node);

	~TriggerComponent();

	void setType(TriggerComponentShapeType type);

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

	PhysicsCollisionShapePtr m_shape;
	PhysicsBodyPtr m_trigger;

	SceneDynamicArray<SceneNode*> m_bodiesEnter;
	SceneDynamicArray<SceneNode*> m_bodiesExit;

	TriggerComponentShapeType m_type = TriggerComponentShapeType::kCount;

	Bool m_resetEnter = true;
	Bool m_resetExit = true;

	static MyPhysicsTriggerCallbacks m_callbacks;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
