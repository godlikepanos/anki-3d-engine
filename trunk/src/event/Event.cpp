#include "anki/event/Event.h"
#include "anki/event/EventManager.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Event::Event(F32 startTime_, F32 duration_, EventManager* manager_, U8 flags)
	:	Flags<U8>(flags), 
		startTime(startTime_),
		duration(duration_),
		manager(manager_)
{
	ANKI_ASSERT(manager);
}

//==============================================================================
SceneAllocator<U8> Event::getSceneAllocator() const
{
	return manager->getSceneAllocator();
}

//==============================================================================
SceneAllocator<U8> Event::getSceneFrameAllocator() const
{
	return manager->getSceneFrameAllocator();
}

//==============================================================================
Event::~Event()
{
	if(manager)
	{
		manager->unregisterEvent(this);
	}
}

//==============================================================================
F32 Event::getDelta(F32 crntTime) const
{
	F32 d = crntTime - startTime; // delta
	F32 dp = d / duration; // delta as persentage
	return dp;
}

} // end namespace anki
