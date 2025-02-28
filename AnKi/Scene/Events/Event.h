// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup scene
/// @{

/// The base class for all events
class Event : public IntrusiveListEnabled<Event>
{
	friend class EventManager;

public:
	/// @param startTime The time the event will start. If it's < 0 then start the event now.
	/// @param duration The duration of the event.
	Event(Second startTime, Second duration)
	{
		init(startTime, duration);
	}

	virtual ~Event() = default;

	Second getStartTime() const
	{
		return m_startTime;
	}

	Second getDuration() const
	{
		return m_duration;
	}

	Bool isDead(Second crntTime) const
	{
		return crntTime >= m_startTime + m_duration;
	}

	void markForDeletion()
	{
		m_markedForDeletion = true;
	}

	Bool isMarkedForDeletion() const
	{
		return m_markedForDeletion;
	}

	void setReanimate(Bool reanimate)
	{
		m_reanimate = reanimate;
	}

	Bool getReanimate() const
	{
		return m_reanimate;
	}

	WeakArray<SceneNode*> getAssociatedSceneNodes()
	{
		return (m_associatedNodes.getSize() == 0) ? WeakArray<SceneNode*>()
												  : WeakArray<SceneNode*>(&m_associatedNodes[0], m_associatedNodes.getSize());
	}

	ConstWeakArray<SceneNode*> getAssociatedSceneNodes() const
	{
		return (m_associatedNodes.getSize() == 0) ? ConstWeakArray<SceneNode*>()
												  : ConstWeakArray<SceneNode*>(&m_associatedNodes[0], m_associatedNodes.getSize());
	}

	void addAssociatedSceneNode(SceneNode* node)
	{
		ANKI_ASSERT(node);
		m_associatedNodes.emplaceBack(node);
	}

	/// This method should be implemented by the derived classes
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	virtual void update(Second prevUpdateTime, Second crntTime) = 0;

	/// This is called when the event is killed
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	virtual void onKilled([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
	{
	}

protected:
	Second m_startTime = 0.0;
	Second m_duration = 0.0;

	Bool m_markedForDeletion = false;
	Bool m_reanimate = false;

	SceneDynamicArray<SceneNode*> m_associatedNodes;

	/// Return the u between current time and when the event started
	/// @return A number [0.0, 1.0]
	Second getDelta(Second crntTime) const;

	void init(Second startTime, Second duration);
};
/// @}

} // end namespace anki
