// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/physics/PhysicsTrigger.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Trigger component.
class TriggerComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::TRIGGER;

	TriggerComponent(SceneNode* node, PhysicsTriggerPtr trigger);

	~TriggerComponent();

	/// Process a contact.
	virtual void processContact(SceneNode& node);

	WeakArray<SceneNode*> getContactSceneNodes()
	{
		return WeakArray<SceneNode*>(m_contactNodes);
	}

private:
	class MyPhysicsTriggerProcessContactCallback;

	SceneNode* m_node;
	PhysicsTriggerPtr m_trigger;
	DynamicArray<SceneNode*> m_contactNodes;
	Timestamp m_contactNodesArrayTimestamp = 0;
	MyPhysicsTriggerProcessContactCallback* m_contactCb = nullptr;
};
/// @}

} // end namespace anki