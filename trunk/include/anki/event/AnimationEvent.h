// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/Event.h"
#include "anki/resource/Resource.h"

namespace anki {

/// @addtogroup Events
/// @{

/// Event controled by animation resource
class AnimationEvent: public Event
{
public:
	AnimationEvent(EventManager* manager, const AnimationResourcePointer& anim, 
		SceneNode* movableSceneNode);

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime) override;

private:
	AnimationResourcePointer anim;
};
/// @}

} // end namespace anki
