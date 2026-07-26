// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

// This is a special event that is called when nodes exit or enter a TriggerComponent
class TriggerEvent final : public Event
{
	ANKI_REGISTER_EVENT_CLASS(TriggerEvent)

public:
	TriggerEvent(Second startTime, Second duration, WeakArray<SceneNode*> nodes)
		: Event(startTime, duration, nodes)
	{
	}

	~TriggerEvent()
	{
	}

	void update([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime) override
	{
		ANKI_ASSERT(getAssociatedSceneNodes().getSize() == 2);
		if(m_triggerEnter)
		{
			getAssociatedSceneNodes()[0]->onTriggerEnter(getAssociatedSceneNodes()[1]);
		}
		else
		{
			getAssociatedSceneNodes()[0]->onTriggerExit(getAssociatedSceneNodes()[1]);
		}

		markForDeletion();
	}

	void setTriggerEnter()
	{
		m_triggerEnter = true;
	}

	void setTriggerExit()
	{
		m_triggerEnter = false;
	}

private:
	Bool m_triggerEnter = false;
};

} // end namespace anki
