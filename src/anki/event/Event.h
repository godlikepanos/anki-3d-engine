// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/Math.h>
#include <anki/util/Enum.h>

namespace anki
{

// Forward
class EventManager;
class SceneNode;
class SceneGraph;

/// @addtogroup event
/// @{

/// The base class for all events
class Event
{
	friend class EventManager;

public:
	/// Event flags
	enum class Flag : U8
	{
		NONE = 0,
		REANIMATE = 1 << 0,
		MARKED_FOR_DELETION = 1 << 1
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flag, friend);

	/// Constructor
	Event(EventManager* manager);

	virtual ~Event();

	void init(F32 startTime, F32 duration, SceneNode* snode = nullptr, Flag flags = Flag::NONE);

	F32 getStartTime() const
	{
		return m_startTime;
	}

	F32 getDuration() const
	{
		return m_duration;
	}

	Bool isDead(F32 crntTime) const
	{
		return crntTime >= m_startTime + m_duration;
	}

	EventManager& getEventManager()
	{
		return *m_manager;
	}

	const EventManager& getEventManager() const
	{
		return *m_manager;
	}

	SceneNode* getSceneNode()
	{
		return m_node;
	}

	const SceneNode* getSceneNode() const
	{
		return m_node;
	}

	void setMarkedForDeletion();

	Bool getMarkedForDeletion() const
	{
		return (m_flags & Flag::MARKED_FOR_DELETION) != Flag::NONE;
	}

	void setReanimate(Bool reanimate)
	{
		m_flags = (reanimate) ? (m_flags | Flag::REANIMATE) : (m_flags & ~Flag::REANIMATE);
	}

	Bool getReanimate() const
	{
		return (m_flags & Flag::REANIMATE) != Flag::NONE;
	}

	SceneGraph& getSceneGraph();

	const SceneGraph& getSceneGraph() const;

	/// This method should be implemented by the derived classes
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	virtual ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime) = 0;

	/// This is called when the event is killed
	/// @param prevUpdateTime The time of the previous update (sec)
	/// @param crntTime The current time (sec)
	/// @param[out] Return false if you don't want to be killed
	virtual ANKI_USE_RESULT Error onKilled(F32 prevUpdateTime, F32 crntTime, Bool& kill)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		kill = true;
		return ErrorCode::NONE;
	}

protected:
	EventManager* m_manager = nullptr;

	F32 m_startTime; ///< The time the event will start. Eg 23:00. If it's < 0
	///< then start the event now.
	F32 m_duration; ///< The duration of the event

	SceneNode* m_node = nullptr;

	Flag m_flags = Flag::NONE;

	/// Return the u between current time and when the event started
	/// @return A number [0.0, 1.0]
	F32 getDelta(F32 crntTime) const;
};
/// @}

} // end namespace anki
