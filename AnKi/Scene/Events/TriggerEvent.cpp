// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/TriggerEvent.h>
#include <AnKi/Scene/Components/ScriptComponent.h>

namespace anki {

void TriggerEvent::update([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	ANKI_ASSERT(getAssociatedSceneNodes().getSize() == 2);

	SceneNode& trigger = *getAssociatedSceneNodes()[0];
	SceneNode& node = *getAssociatedSceneNodes()[1];

	Array2d<SceneNode*, 2, 2> nodes = {{{{&trigger, &node}}, {{&node, &trigger}}}};

	for(U32 i = 0; i < 2; i++)
	{
		// Do the C++ first
		SceneNode& selfNode = *nodes[i][0];
		SceneNode& otherNode = *nodes[i][1];
		if(m_triggerEnter)
		{
			selfNode.onTriggerEnter(&otherNode);
		}
		else
		{
			selfNode.onTriggerExit(&otherNode);
		}

		// Do the scripts after
		selfNode.iterateComponentsOfType<ScriptComponent>([&](ScriptComponent& scriptc) {
			scriptc.callOnTrigger(m_triggerEnter, selfNode, otherNode);
			return FunctorContinue::kContinue;
		});
	}

	markForDeletion();
}

} // end namespace anki
