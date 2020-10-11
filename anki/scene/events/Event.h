// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/util/List.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// The base class for all events
class Event : public IntrusiveListEnabled<Event>
{
	friend class EventManager;

public:
	/// Constructor
	Event(EventManager* manager);

	virtual ~Event();

	SceneAllocator<U8> getAllocator() const;

	EventManager& getEventManager()
	{
		return *m_manager;
	}

	const EventManager& getEventManager() const
	{
		return *m_manager;
	}

	SceneGraph& getSceneGraph();

	const SceneGraph& getSceneGraph() const;

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

	/// @note It's thread safe.
	void setMarkedForDeletion();

	Bool getMarkedForDeletion() const
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
		return (m_associatedNodes.getSize() == 0)
				   ? WeakArray<SceneNode*>()
				   : WeakArray<SceneNode*>(&m_associatedNodes[0], m_associatedNodes.getSize());
	}

	void addAssociatedSceneNode(SceneNode* node)
	{
		ANKI_ASSERT(node);
		m_associatedNodes.emplaceBack(getAllocator(), node);
	}

	/// This method should be implemented by the derived classes
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	virtual ANKI_USE_RESULT Error update(Second prevUpdateTime, Second crntTime) = 0;

	/// This is called when the event is killed
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	virtual ANKI_USE_RESULT Error onKilled(Second prevUpdateTime, Second crntTime)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		return Error::NONE;
	}

protected:
	EventManager* m_manager = nullptr;

	Second m_startTime = 0.0;
	Second m_duration = 0.0;

	Bool m_markedForDeletion = false;
	Bool m_reanimate = false;

	DynamicArray<SceneNode*> m_associatedNodes;

	/// @param startTime The time the event will start. If it's < 0 then start the event now.
	/// @param duration The duration of the event.
	void init(Second startTime, Second duration);

	/// Return the u between current time and when the event started
	/// @return A number [0.0, 1.0]
	Second getDelta(Second crntTime) const;
};
/// @}

} // end namespace anki
