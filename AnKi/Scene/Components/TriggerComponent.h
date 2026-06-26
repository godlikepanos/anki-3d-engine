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

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TriggerComponentShapeType);

static inline Array kTriggerComponentShapeTypeNames = {"Sphere", "Box"};

// Trigger component
class TriggerComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(TriggerComponent)

public:
	TriggerComponent(const SceneComponentInitInfo& init);

	~TriggerComponent();

	TriggerComponent& setTriggerComponentType(TriggerComponentShapeType type);

	TriggerComponentShapeType getTriggerComponentType() const
	{
		return m_type;
	}

	ANKI_INTERNAL WeakArray<SceneNode*> getSceneNodesEnter()
	{
		return WeakArray<SceneNode*>(m_enteredNodes);
	}

	ANKI_INTERNAL WeakArray<SceneNode*> getSceneNodesExit()
	{
		return WeakArray<SceneNode*>(m_exitedNodes);
	}

private:
	class MyPhysicsTriggerCallbacks;

	PhysicsCollisionShapePtr m_shape;
	PhysicsBodyPtr m_trigger;

	SceneDynamicArray<SceneNode*> m_enteredNodes;
	SceneDynamicArray<SceneNode*> m_exitedNodes;

	TriggerComponentShapeType m_type = TriggerComponentShapeType::kSphere;

	Bool m_resetEnter = true;
	Bool m_resetExit = true;

	static MyPhysicsTriggerCallbacks m_callbacks;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
