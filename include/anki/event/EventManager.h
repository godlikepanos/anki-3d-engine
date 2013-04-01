#ifndef ANKI_EVENT_MANAGER_H
#define ANKI_EVENT_MANAGER_H

#include "anki/event/Event.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/scene/Common.h"
#include "anki/math/Math.h"

namespace anki {

// Forward
class SceneGraph;
class Path;
class SceneNode;

class SceneAmbientColorEvent;
class LightEvent;
class LightEventData;
class MovableEvent;
class MovableEventData;
class FollowPathEvent;

/// This manager creates the events ands keeps tracks of them
class EventManager
{
public:
	typedef SceneVector<std::shared_ptr<Event>> EventsContainer;

	EventManager(SceneGraph* scene);
	~EventManager();

	/// @name Accessors
	/// @{
	SceneAllocator<U8> getSceneAllocator() const;
	SceneAllocator<U8> getSceneFrameAllocator() const;
	/// @}

	/// @name Creators
	/// @{

	std::shared_ptr<Event> newSceneAmbientColorEvent(
		F32 startTime, F32 duration, const Vec3& finalColor);

	std::shared_ptr<Event> newLightEvent(
		F32 startTime, F32 duration, const LightEventData& data);

	std::shared_ptr<Event> newMovableEvent(
		F32 startTime, F32 duration, const MovableEventData& data);

	std::shared_ptr<Event> newFollowPathEvent(
		F32 startTime, F32 duration,
		SceneNode* movableSceneNode, Path* path, F32 distPerTime);
	/// @}

	/// Update
	void updateAllEvents(F32 prevUpdateTime, F32 crntTime);

	/// Remove an event from the container
	void unregisterEvent(Event* event);

private:
	SceneGraph* scene = nullptr;
	EventsContainer events;
	F32 prevUpdateTime;
	F32 crntTime;

	std::shared_ptr<Event> registerEvent(Event* event);
};

} // end namespace anki

#endif
