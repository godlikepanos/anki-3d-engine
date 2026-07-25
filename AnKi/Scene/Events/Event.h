// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// See ANKI_REGISTER_EVENT_CLASS
class EventRegistryRecord : public GlobalRegistryRecord
{
public:
	using GetAllocationInfoCallback = void (*)(U32& size, U32& alignment);
	using ConstructCallback = void (*)(void* eventMemory, Second startTime, Second duration, WeakArray<SceneNode*> nodes);

	GetAllocationInfoCallback m_getAllocationInfoCallback = nullptr;
	ConstructCallback m_constructCallback = nullptr;

	EventRegistryRecord(const Char* className, GetAllocationInfoCallback getAllocInfoCallback, ConstructCallback constructCallback)
		: GlobalRegistryRecord(className, GlobalRegistryRecordType::kEvent)
		, m_getAllocationInfoCallback(getAllocInfoCallback)
		, m_constructCallback(constructCallback)
	{
	}
};

// It's required on all Event derived classes. It registers some global info required by serialization
#define ANKI_REGISTER_EVENT_CLASS(className) \
	static void getAllocationInfoCallback(U32& size, U32& alignment) \
	{ \
		size = U32(sizeof(className)); \
		alignment = U32(alignof(className)); \
	} \
	static void constructCallback(void* eventMemory, Second startTime, Second duration, WeakArray<SceneNode*> nodes) \
	{ \
		::new(eventMemory) className(startTime, duration, nodes); \
	} \
	inline static EventRegistryRecord m_registryRecord{ANKI_STRINGIZE(className), getAllocationInfoCallback, constructCallback}; \
	const EventRegistryRecord* getEventRegistryRecord() const override \
	{ \
		static_assert(std::is_final_v<className>, "ANKI_REGISTER_EVENT_CLASS only in final classes"); \
		return &m_registryRecord; \
	}

// The base class for all events
class Event
{
	friend class EventManager;

public:
	// startTime: The time the event will start. If it's < 0 then start the event now.
	// duration: The duration of the event.
	Event(Second startTime, Second duration, WeakArray<SceneNode*> nodes);

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
		return WeakArray(m_associatedNodes);
	}

	ConstWeakArray<SceneNode*> getAssociatedSceneNodes() const
	{
		return m_associatedNodes;
	}

	// This method should be implemented by the derived classes
	// prevUpdateTime: The time of the previous update (sec)
	// crntTime: The current time (sec)
	virtual void update(Second prevUpdateTime, Second crntTime) = 0;

	// This is called when the event is killed
	// prevUpdateTime: The time of the previous update (sec)
	// crntTime: The current time (sec)
	virtual void onKilled([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
	{
	}

	virtual const EventRegistryRecord* getEventRegistryRecord() const = 0;

protected:
	Second m_startTime = 0.0;
	Second m_duration = 0.0;

	U32 m_blockArrayIndex = kMaxU32;
	Bool m_markedForDeletion = false;
	Bool m_reanimate = false;

	SceneDynamicArray<SceneNode*> m_associatedNodes;

	// Return the u between current time and when the event started. A number [0.0, 1.0]
	Second getDelta(Second crntTime) const
	{
		const Second d = crntTime - m_startTime; // delta
		const Second dp = d / m_duration; // delta as persentage
		return dp;
	}
};

} // end namespace anki
