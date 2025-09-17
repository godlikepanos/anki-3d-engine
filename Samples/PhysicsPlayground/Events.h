// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/AnKi.h>

using namespace anki;

inline void createDestructionEvent(SceneNode* node, Second timeToKill)
{
	constexpr CString script = R"(
function update(event, prevTime, crntTime)
	-- Do nothing
end

function onKilled(event, prevTime, crntTime)
	logi(string.format("Will kill %s", event:getAssociatedSceneNodes():getAt(0):getName()))
	event:getAssociatedSceneNodes():getAt(0):markForDeletion()
end
	)";
	ScriptEvent* event = SceneGraph::getSingleton().getEventManager().newEvent<ScriptEvent>(-1.0, timeToKill, script);
	event->addAssociatedSceneNode(node);
}
